#pragma once


namespace thnet {
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
#ifdef _MSC_VER
typedef unsigned __int64 U64;
#else
typedef unsigned long long U64;
#endif
enum Kind { Ping=1, Pong, JoinQueue, CreateRoom, JoinRoom, Waiting,
    MatchStart, StartAck, Frame, GameEvent, Ack, Leave, Error, Ready, Go, GoAck };
enum EventType { SpawnExAttack=1, SpawnBoss, PlayerHit, DeathbombUsed, RoundOver,
    TeamState, BossDamage, SupportRequest, SupportCommit, ScoreReport, PauseState };
const U32 RulesVersion=0x20261112;
#pragma pack(push, 1)
struct Header { char magic[4]; U8 version, kind; U16 size; U64 token; };
struct JoinData { U32 nonce; U8 character, difficulty; U16 room; U8 mode,boss; U32 rules; };
struct MatchData { U32 match, seed; U8 role, character0, character1, difficulty; U32 nonce; U8 mode,boss; U16 reserved; U32 rules; };
struct FrameSample { U32 tick; float x, y; U16 inputMask; };
struct PacketFrameState { FrameSample states[3]; };
struct PacketGameEvent { U32 eventId, tick; U8 type; U32 rngSeed; float params[4]; };
#pragma pack(pop)
typedef char HeaderSize[sizeof(Header)==16 ? 1:-1];
typedef char MatchSize[sizeof(MatchData)==24 ? 1:-1];
typedef char FrameSize[sizeof(PacketFrameState)==42 ? 1:-1];
typedef char EventSize[sizeof(PacketGameEvent)==29 ? 1:-1];
inline bool Newer(U32 a, U32 b) { return (int)(a-b)>0; }
}
