
import struct

MAGIC = b'TH9N'
VERSION = 2
RULES = 0x20261112
PING, PONG, QUEUE, CREATE, JOIN, WAIT, START, START_ACK, FRAME, EVENT, ACK, LEAVE, ERROR = range(1, 14)
READY, GO, GO_ACK = 14, 15, 16

HEADER = struct.Struct('<4sBBHQ')
JOIN_DATA = struct.Struct('<IBBHBBI')  
START_DATA = struct.Struct('<IIBBBBIBBHI')  
SAMPLE = struct.Struct('<IffH')
FRAME_SIZE = SAMPLE.size * 3
EVENT_DATA = struct.Struct('<IIBIffff')
SIZES = {PING: 4, QUEUE: JOIN_DATA.size, CREATE: JOIN_DATA.size, JOIN: JOIN_DATA.size, START_ACK: 4,
         FRAME: FRAME_SIZE, EVENT: EVENT_DATA.size, ACK: 4, LEAVE: 0, READY: 4, GO_ACK: 4}

def packet(kind, token=0, payload=b''):
    return HEADER.pack(MAGIC, VERSION, kind, len(payload), token) + payload

def unpack(data):
    if len(data) < HEADER.size:
        raise ValueError('short header')
    magic, version, kind, size, token = HEADER.unpack_from(data)
    if magic != MAGIC or version != VERSION or len(data) != HEADER.size + size:
        raise ValueError('invalid header')
    return kind, token, data[HEADER.size:]
