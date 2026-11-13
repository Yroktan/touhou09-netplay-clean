
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET NativeSocket;
#define CLOSESOCK closesocket
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
typedef int NativeSocket;
#define INVALID_SOCKET (-1)
#define CLOSESOCK close
#endif
#include <string.h>
#include <math.h>
#include "NetManager.h"
#include "NetConfig.h"

namespace thnet {
NetManager g_NetManager;
static U64 Microseconds() {
#ifdef _WIN32
    LARGE_INTEGER counter, frequency;
    QueryPerformanceCounter(&counter); QueryPerformanceFrequency(&frequency);
    return U64(counter.QuadPart/frequency.QuadPart)*1000000 +
        U64(counter.QuadPart%frequency.QuadPart)*1000000/U64(frequency.QuadPart);
#else
    timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return U64(t.tv_sec)*1000000 + t.tv_nsec/1000;
#endif
}
U32 NetManager::ClockMs() { return U32(Microseconds()/1000); }
NetManager::NetManager() { memset(this, 0, sizeof(*this)); socket_=U64(INVALID_SOCKET); }
NetManager::~NetManager() { Close(); }
bool NetManager::Open(const char *host, U16 port) {
    Close();
    RelayEndpoint endpoint;char configError[80];
    if(!LoadRelayEndpoint(host,port,endpoint,configError,sizeof(configError))) {
        Fail(configError);return false;
    }
#ifdef _WIN32
    WSADATA data;
    if (WSAStartup(MAKEWORD(2,2), &data)!=0) { Fail("WSAStartup failed"); return false; }
    winsock_=true;
#endif
    addrinfo hints;memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;hints.ai_socktype=SOCK_DGRAM;hints.ai_protocol=IPPROTO_UDP;
    addrinfo *addresses=0;
    if(getaddrinfo(endpoint.host,0,&hints,&addresses)!=0 || !addresses) {
        Close();Fail("Relay hostname resolution failed");return false;
    }
    sockaddr_in server=*(sockaddr_in*)addresses->ai_addr;
    freeaddrinfo(addresses);server.sin_port=htons(endpoint.port);
    NativeSocket s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s==INVALID_SOCKET) { Close(); Fail("UDP socket failed"); return false; }
    socket_=U64(s);
#ifdef _WIN32
    u_long nonblocking=1;
    if (ioctlsocket(s, FIONBIO, &nonblocking)!=0) { Close(); Fail("FIONBIO failed"); return false; }
#else
    if (fcntl(s, F_SETFL, O_NONBLOCK)<0) { Close(); Fail("O_NONBLOCK failed"); return false; }
#endif
    if (connect(s,(sockaddr*)&server,sizeof(server))!=0) { Close(); Fail("UDP connect failed"); return false; }
    state_=Lobby;
    nextIncomingId_=1;
    lastPing_=ClockMs()-1000; lastReceive_=ClockMs();
    Poll();
    return true;
}
void NetManager::Close() {
    if (socket_!=U64(INVALID_SOCKET)) {
        if (token_) Send(Leave,0,0);
        CLOSESOCK(NativeSocket(socket_));
    }
#ifdef _WIN32
    if (winsock_) WSACleanup();
#endif
    memset(this,0,sizeof(*this)); socket_=U64(INVALID_SOCKET);
}
void NetManager::Fail(const char *reason) {
    state_=Disconnected;
    strncpy(error_,reason,sizeof(error_)-1); error_[sizeof(error_)-1]=0;
}
bool NetManager::Send(U8 kind, const void *data, U16 size) {
    if (socket_==U64(INVALID_SOCKET) || size>256) return false;
    unsigned char bytes[272];
    Header h; memcpy(h.magic,"TH9N",4); h.version=2; h.kind=kind; h.size=size; h.token=token_;
    memcpy(bytes,&h,sizeof(h)); if(size) memcpy(bytes+sizeof(h),data,size);
    return send(NativeSocket(socket_),(const char*)bytes,sizeof(h)+size,0)==int(sizeof(h)+size);
}
bool NetManager::Online() const { return lastPong_ && U32(ClockMs()-lastPong_)<3000; }
void NetManager::Join(Kind kind, U8 character, U8 difficulty, U16 room, U8 mode, U8 boss) {
    if (state_!=Lobby || (kind!=JoinQueue && kind!=CreateRoom && kind!=JoinRoom) || character>15 || difficulty>3) return;
    if(kind==JoinRoom && (room<1000 || room>9999)) return;
    join_.nonce=ClockMs() ^ U32(Microseconds()); if(!join_.nonce) join_.nonce=1;
    join_.character=character; join_.difficulty=difficulty; join_.room=room;
    join_.mode=mode; join_.boss=boss; join_.rules=RulesVersion;
    joinKind_=U8(kind); state_=Searching; joinAt_=ClockMs();
    Send(joinKind_,&join_,sizeof(join_));
}
void NetManager::MarkReady() {
    if(state_!=Matched) return;
    if(!ready_) { ready_=true; readyAt_=ClockMs()-500; lastPing_=ClockMs(); Send(Ping,&lastPing_,4); }
}
bool NetManager::CanSimulate() const {
    return state_==Matched && haveStart_ && clockSynced_ && (int)(ClockMs()+serverOffset_-startAt_)>=0;
}
U32 NetManager::StartTick() const {
    return CanSimulate() ? U32(ClockMs()+serverOffset_-startAt_)*60/1000 : 0;
}
void NetManager::Poll() {
    if (socket_==U64(INVALID_SOCKET) || state_==Disconnected) return;
    U64 begin=Microseconds();
    unsigned char bytes[512];

    for(unsigned i=0;i<64 && Microseconds()-begin<500;++i) {
        int n=recv(NativeSocket(socket_),(char*)bytes,sizeof(bytes),0);
        if(n<=0) break;
        Receive(bytes,unsigned(n));
    }
    U32 now=ClockMs();
    if(U32(now-lastPing_)>=1000) { lastPing_=now; Send(Ping,&now,4); }
    if(ready_ && !haveStart_ && U32(now-readyAt_)>=250) { readyAt_=now; Send(Ready,&match_.match,4); }
    if(state_==Searching && U32(now-joinAt_)>=500) { joinAt_=now; Send(joinKind_,&join_,sizeof(join_)); }
    if((state_==Searching || state_==Matched) && U32(now-lastReceive_)>=10000) Fail("Server connection timed out");
    for(unsigned i=0;i<64;++i) if(pending_[i].used) {
        Pending &p=pending_[i];
        if(U32(now-p.first)>=10000) { Fail("Reliable event delivery timed out"); break; }
        if(U32(now-p.sent)>=100) { Send(GameEvent,&p.event,sizeof(p.event)); p.sent=now; }
    }
    U32 elapsed=U32(Microseconds()-begin); if(elapsed>maxPollUs_) maxPollUs_=elapsed;
}
void NetManager::Receive(const unsigned char *bytes, unsigned size) {
    if(size<sizeof(Header)) return;
    Header h; memcpy(&h,bytes,sizeof(h));
    if(memcmp(h.magic,"TH9N",4) || h.version!=2 || size!=sizeof(h)+h.size) return;
    const unsigned char *body=bytes+sizeof(h);
    U32 now=ClockMs();
    if(h.kind==Pong && h.size==8 && h.token==token_) {
        U32 echoed; memcpy(&echoed,body,4);
        if(echoed!=lastPing_) return;
        U32 server; memcpy(&server,body+4,4);
        pingMs_=now-echoed; serverOffset_=server-(echoed+pingMs_/2); clockSynced_=true;
        lastPong_=lastReceive_=now; return;
    }
    if(h.kind==Waiting && h.size==6 && state_==Searching) {
        U32 nonce; memcpy(&nonce,body+2,4); if(nonce!=join_.nonce) return;
        token_=h.token; memcpy(&room_,body,2); lastReceive_=now; return;
    }
    if(h.kind==MatchStart && h.size==sizeof(MatchData)) {
        MatchData m; memcpy(&m,body,sizeof(m));
        if(m.nonce!=join_.nonce || m.role>1 || m.character0>15 || m.character1>15 || m.difficulty>3 ||
            m.mode>1 || m.boss>15 || m.rules!=RulesVersion) return;
        if(state_==Matched && (m.match!=match_.match || h.token!=token_)) return;
        if(state_!=Searching && state_!=Matched) return;
        token_=h.token; match_=m; state_=Matched; lastReceive_=now;
        Send(StartAck,&m.match,4); return;
    }
    if(h.kind==Error && state_==Searching && !h.token) { Fail("Room unavailable or server full"); return; }
    if(!token_ || h.token!=token_) return;
    if(h.kind==Go && h.size==8 && state_==Matched) {
        U32 match; memcpy(&match,body,4); if(match!=match_.match) return;
        memcpy(&startAt_,body+4,4); haveStart_=true;
        Send(GoAck,&match_.match,4); lastReceive_=now; return;
    }
    if(h.kind==Leave && h.size==0) { Fail("Other player disconnected"); return; }
    if(state_!=Matched) return;
    if(h.kind==Frame && h.size==sizeof(PacketFrameState)) {
        PacketFrameState frame; memcpy(&frame,body,sizeof(frame));
        for(unsigned i=0;i<3;++i) {
            const FrameSample &s=frame.states[i];

            if(!(s.x>=-144 && s.x<=144 && s.y>=0 && s.y<=448)) continue;
            if(haveRemote_ && !Newer(s.tick,latestTick_) && U32(latestTick_-s.tick)>=256) continue;
            SampleSlot &slot=samples_[s.tick%256]; slot.used=true; slot.sample=s;
            if(!haveRemote_ || Newer(s.tick,latestTick_)) latestTick_=s.tick;
            haveRemote_=true;
        }
        lastReceive_=now;
    } else if(h.kind==Ack && h.size==4) {
        U32 id; memcpy(&id,body,4);
        for(unsigned i=0;i<64;++i) if(pending_[i].used && pending_[i].event.eventId==id) pending_[i].used=false;
        lastReceive_=now;
    } else if(h.kind==GameEvent && h.size==sizeof(PacketGameEvent)) {
        PacketGameEvent event; memcpy(&event,body,sizeof(event));
        if(!event.eventId || event.type<SpawnExAttack || event.type>PauseState) return;
        for(unsigned p=0;p<4;++p) if(!(event.params[p]>=-10000000 && event.params[p]<=10000000)) return;
        int ahead=int(event.eventId-nextIncomingId_);
        if(ahead<0) { Send(Ack,&event.eventId,4); return; }
        if(ahead>=64) return;
        incoming_[event.eventId%64]=event;
        Send(Ack,&event.eventId,4); lastReceive_=now;
    }
}
void NetManager::SendFrame(const FrameSample &s) {
    if(state_!=Matched) return;
    outgoing_.states[2]=outgoing_.states[1]; outgoing_.states[1]=outgoing_.states[0]; outgoing_.states[0]=s;
    if(historyCount_<3) { ++historyCount_; for(unsigned i=historyCount_;i<3;++i) outgoing_.states[i]=s; }
    Send(Frame,&outgoing_,sizeof(outgoing_));
}
bool NetManager::SendEvent(PacketGameEvent event) {
    if(state_!=Matched) return false;
    for(unsigned i=0;i<64;++i) if(!pending_[i].used) {
        event.eventId=++eventId_; if(!event.eventId) event.eventId=++eventId_;
        Pending &p=pending_[i]; p.used=true; p.event=event; p.first=p.sent=ClockMs();
        Send(GameEvent,&event,sizeof(event)); return true;
    }
    Fail("Reliable event queue overflow"); return false;
}
bool NetManager::PopEvent(PacketGameEvent &event) {
    PacketGameEvent &slot=incoming_[nextIncomingId_%64];
    if(slot.eventId!=nextIncomingId_) return false;
    event=slot; slot.eventId=0;
    if(!++nextIncomingId_) ++nextIncomingId_;
    return true;
}
bool NetManager::RemotePosition(U32 tick, float &x, float &y, U16 &input) {
    if(!haveRemote_) return false;
    U32 target=tick>=3 ? tick-3 : 0;
    const FrameSample *best=0;
    for(unsigned i=0;i<256;++i) if(samples_[i].used) {
        const FrameSample &s=samples_[i].sample;
        if(!Newer(s.tick,target) && (!best || Newer(s.tick,best->tick))) best=&s;
    }
    if(!best) return false;
    U32 age=target-best->tick; if(age>6) age=6;
    float vx=0,vy=0;
    const SampleSlot &previous=samples_[(best->tick-1)%256];
    if(previous.used && previous.sample.tick==best->tick-1) {
        vx=best->x-previous.sample.x; vy=best->y-previous.sample.y;
        if(vx>12 || vx< -12 || vy>12 || vy< -12) vx=vy=0;
    }
    float tx=best->x+vx*age, ty=best->y+vy*age;
    if(tx< -136) tx=-136; if(tx>136) tx=136;
    if(ty<16) ty=16; if(ty>432) ty=432;
    if(!haveVisual_) { visualX_=tx; visualY_=ty; haveVisual_=true; visualTick_=tick; }
    else if(visualTick_!=tick) { visualX_+=(tx-visualX_)/3; visualY_+=(ty-visualY_)/3; visualTick_=tick; }
    x=visualX_; y=visualY_; input=best->inputMask; return true;
}
}
