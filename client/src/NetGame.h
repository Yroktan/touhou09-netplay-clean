#pragma once
#include "NetManager.h"
class MainMenu;
class Player;
class SpellCard;
struct ExAttack;
struct Enemy;
struct D3DXVECTOR3;
namespace thnet {
bool Active();
bool WaitForPeer();
bool NetworkPaused();
bool HandlePause(const PacketGameEvent &event);
bool RouteRoundWin(int side);
void RoundReset();
void RoundWon(int side);
bool PostMatchUpdate();
void InitEnemyRandom(Enemy *enemy,int sub);
void InitAttackRandom(ExAttack *attack,int type);
class PatternRandom {
    U16 seed_;
    U32 generated_;
    int side_,index_;
    bool extra_,engaged_;
public:
    explicit PatternRandom(Enemy *enemy);
    explicit PatternRandom(ExAttack *attack);
    ~PatternRandom();
};
class WaveRandom {
    U16 seed_;
    U32 generated_;
    bool engaged_;
public:
    WaveRandom(int side,unsigned sequence,unsigned frame);
    ~WaveRandom();
};
class ActorScope {
public:
    ActorScope();
    ~ActorScope();
};
int TitleUpdate(MainMenu *menu);
int LobbyUpdate(MainMenu *menu);
void OpenLobby(MainMenu *menu);
void MarkGame(bool network);
bool ConsumeTitleReturn();
void DrawMenu(MainMenu *menu);
void BeginFrame();
void BeginGameUpdate();
void EndSession();
bool RemotePlayer(int side);
void BeforePlayer(Player *player);
void AfterPlayer(Player *player);
bool BeforeDeath(Player *player);
class DeathScope {
    Player *player_;
    int health_, state_;
public:
    DeathScope(Player *player);
    ~DeathScope();
};
bool RouteExAttack(int type, D3DXVECTOR3 *pos, int side);
bool RouteSpell(SpellCard *card, int type, int kind, int unknown);
}
