#pragma once
#include "NetProtocol.h"

namespace thnet {
class NetManager {
public:
    enum State { Closed, Lobby, Searching, Matched, Disconnected };
    NetManager();
    ~NetManager();
    bool Open(const char *host=0, U16 port=0);
    void Close();
    void Poll();
    void Join(Kind kind, U8 character, U8 difficulty, U16 room=0, U8 mode=0, U8 boss=0);
    void MarkReady();
    bool CanSimulate() const;
    U32 StartTick() const;
    void SendFrame(const FrameSample &sample);
    bool SendEvent(PacketGameEvent event);
    bool PopEvent(PacketGameEvent &event);
    bool RemotePosition(U32 localTick, float &x, float &y, U16 &input);
    bool Online() const;
    State GetState() const { return state_; }
    const MatchData &Match() const { return match_; }
    U16 Room() const { return room_; }
    U32 PingMs() const { return pingMs_; }
    const char *ErrorText() const { return error_; }
    U32 MaxPollUs() const { return maxPollUs_; }
    static U32 ClockMs();
private:
    struct Pending { bool used; PacketGameEvent event; U32 sent, first; };
    struct SampleSlot { bool used; FrameSample sample; };
    U64 socket_, token_;
    State state_;
    bool winsock_, haveRemote_, haveVisual_;
    MatchData match_;
    JoinData join_;
    U8 joinKind_;
    U16 room_;
    U32 lastPing_, lastPong_, lastReceive_, joinAt_, eventId_, latestTick_;
    U32 pingMs_, maxPollUs_;
    U32 serverOffset_, startAt_, readyAt_;
    bool ready_, haveStart_, clockSynced_;
    PacketFrameState outgoing_;
    unsigned historyCount_;
    SampleSlot samples_[256];
    Pending pending_[64];
    PacketGameEvent incoming_[64];
    U32 nextIncomingId_, visualTick_;
    float visualX_, visualY_;
    char error_[80];
    bool Send(U8 kind, const void *data, U16 size);
    void Receive(const unsigned char *data, unsigned size);
    void Fail(const char *reason);
};
extern NetManager g_NetManager;
}
