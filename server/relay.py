
import asyncio
import collections
import secrets
import struct
import time
from dataclasses import dataclass
from protocol import *
from config import bind_endpoint

@dataclass
class Session:
    addr: tuple
    token: int
    nonce: int
    character: int
    difficulty: int
    seen: float
    mode: int = 0
    boss: int = 0
    role: int = 0
    room: int = 0
    peer: int = 0
    match: int = 0
    start: bytes = b''
    acknowledged: bool = False
    retry_at: float = 0
    started_at: float = 0
    budget: float = 240
    budget_at: float = 0
    ready: bool = False
    go: bytes = b''
    go_ack: bool = False
    go_retry: float = 0

class Relay(asyncio.DatagramProtocol):
    def __init__(self, clock=time.monotonic):
        self.clock = clock
        self.sessions = {}
        self.addresses = {}
        self.rooms = {}
        self.queues = collections.defaultdict(collections.deque)
        self.transport = None

    def connection_made(self, transport):
        self.transport = transport

    def send(self, addr, kind, token=0, payload=b''):
        self.transport.sendto(packet(kind, token, payload), addr)

    def remove(self, token, notify=True):
        s = self.sessions.pop(token, None)
        if not s:
            return
        self.addresses.pop(s.addr, None)
        if self.rooms.get(s.room) == token:
            self.rooms.pop(s.room)
        q = self.queues[(s.mode, s.boss, s.difficulty)]
        try:
            q.remove(token)
        except ValueError:
            pass
        peer = self.sessions.get(s.peer)
        if peer and notify:
            self.send(peer.addr, LEAVE, peer.token)
            self.remove(peer.token, False)

    def match(self, a, b):
        match = secrets.randbits(32) or 1
        seed = secrets.randbits(32)
        for role, s, other in ((0, a, b), (1, b, a)):
            s.peer, s.match = other.token, match
            s.role, s.mode, s.boss, s.difficulty = role, a.mode, a.boss, a.difficulty
            s.started_at = self.clock()
            s.start = START_DATA.pack(match, seed, role, a.character, b.character, a.difficulty, s.nonce, a.mode, a.boss, 0, RULES)
            self.send(s.addr, START, s.token, s.start)
            s.retry_at = self.clock() + .25
        if a.room:
            self.rooms.pop(a.room, None)

    def datagram_received(self, data, addr):
        try:
            kind, token, payload = unpack(data)
        except ValueError:
            return
        if kind not in SIZES or len(payload) != SIZES[kind]:
            return
        now = self.clock()
        if kind == PING:
            
            self.send(addr, PONG, token, payload + struct.pack('<I', int(now * 1000) & 0xffffffff))
            s = self.sessions.get(token)
            if s and s.addr == addr:
                s.seen = now
            return
        if kind in (QUEUE, CREATE, JOIN):
            nonce, character, difficulty, room, mode, boss, rules = JOIN_DATA.unpack(payload)
            if character > 15 or difficulty > 3 or mode > 1 or boss > 15 or rules != RULES or (kind == JOIN and not 1000 <= room <= 9999):
                return
            old = self.sessions.get(self.addresses.get(addr))
            if old:
                if old.nonce == nonce:
                    old.seen = now
                    if old.start:
                        self.send(addr, START, old.token, old.start)
                    else:
                        self.send(addr, WAIT, old.token, struct.pack('<HI', old.room, nonce))
                    return
                self.remove(old.token)
            if len(self.sessions) >= 2048:
                self.send(addr, ERROR, payload=b'busy')
                return
            host = None
            if kind == JOIN:
                host = self.sessions.get(self.rooms.get(room))
                if not host or host.peer:
                    self.send(addr, ERROR, payload=b'room unavailable')
                    return
            token = secrets.randbits(64) or 1
            while token in self.sessions:
                token = secrets.randbits(64) or 1
            s = Session(addr, token, nonce, character, difficulty, now, mode=mode, boss=boss, budget_at=now)
            self.sessions[token] = s
            self.addresses[addr] = token
            if kind == CREATE:
                available = [r for r in range(1000, 10000) if r not in self.rooms]
                if not available:
                    self.remove(token)
                    self.send(addr, ERROR, payload=b'rooms full')
                    return
                s.room = secrets.choice(available)
                self.rooms[s.room] = token
            self.send(addr, WAIT, token, struct.pack('<HI', s.room, nonce))
            if host:
                self.match(host, s)
            elif kind == QUEUE:
                q = self.queues[(mode, boss, difficulty)]
                while q and q[0] not in self.sessions:
                    q.popleft()
                if q:
                    self.match(self.sessions[q.popleft()], s)
                else:
                    q.append(token)
            return
        s = self.sessions.get(token)
        if not s or s.addr != addr:
            return
        s.budget = min(240, s.budget + (now - s.budget_at) * 180)
        s.budget_at = now
        if s.budget < 1:
            return
        s.budget -= 1
        s.seen = now
        if kind == LEAVE:
            self.remove(token)
        elif kind == START_ACK:
            if struct.unpack('<I', payload)[0] == s.match:
                s.acknowledged = True
        elif kind == READY:
            peer = self.sessions.get(s.peer)
            if peer and s.acknowledged and struct.unpack('<I', payload)[0] == s.match:
                s.ready = True
                if peer.ready and not s.go:
                    go = struct.pack('<II', s.match, (int(now*1000)+1500) & 0xffffffff)
                    for player in (s, peer):
                        player.go = go
                        player.go_retry = now + .25
                        self.send(player.addr, GO, player.token, go)
                elif s.go:
                    self.send(s.addr, GO, s.token, s.go)
        elif kind == GO_ACK:
            if s.go and struct.unpack('<I', payload)[0] == s.match:
                s.go_ack = True
        elif kind in (FRAME, EVENT, ACK):
            peer = self.sessions.get(s.peer)
            if not peer or not s.acknowledged or not peer.acknowledged:
                return
            if kind == EVENT and EVENT_DATA.unpack(payload)[2] not in range(1, 12):
                return
            
            self.send(peer.addr, kind, peer.token, payload)

    def sweep(self):
        now = self.clock()
        for token, s in list(self.sessions.items()):
            if token not in self.sessions:
                continue
            if now - s.seen >= 10 or (s.start and not s.acknowledged and now - s.started_at >= 10):
                self.remove(token)
                continue
            elif s.start and not s.acknowledged and now >= s.retry_at:
                self.send(s.addr, START, s.token, s.start)
                s.retry_at = now + .25
            if s.go and not s.go_ack and now >= s.go_retry:
                self.send(s.addr, GO, s.token, s.go)
                s.go_retry = now + .25

async def main(host, port):
    loop = asyncio.get_running_loop()
    transport, relay = await loop.create_datagram_endpoint(Relay, local_addr=(host, port))
    print('TH09 relay listening on %s:%d' % (host, port), flush=True)
    try:
        while True:
            await asyncio.sleep(.25)
            relay.sweep()
    finally:
        transport.close()

if __name__ == '__main__':
    import argparse
    import sys
    if sys.platform=='win32':
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    parser = argparse.ArgumentParser()
    parser.add_argument('--host')
    parser.add_argument('--port', type=int)
    args = parser.parse_args()
    asyncio.run(main(*bind_endpoint(args.host,args.port)))
