#ifdef TH09_NATIVE_NETPLAY
#include "mainmenu.h"
#include "ascii.h"
#include "gamemanager.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "exattack.h"
#include "spellcard.h"
#include "gui.h"
#include "scoredat.h"
#include "NetGame.h"
#include "CoopGame.h"
#include "UiText.h"
#include <math.h>

namespace thnet {
static bool active=false, applying=false, forwardingTitle=false, started=false, testStarted=false;
static bool pauseFlags[2]={false,false};
static int actorDepth=0, titleCursor=0, lobbyCursor=0, character=0, difficulty=1, mode=0;
static U32 tick=0, roundStart=0, sentAttacks=0, receivedAttacks=0;
static U16 roomCode=0, previousRemoteInput=0;
static unsigned digitsDown=0;
static unsigned resultFrames=0;
static bool testing=false, roomWritten=false;
static bool customGame=false, customNetwork=false;
static FILE *trace=0;
static char disconnectNotice[96]={0};
static Rng enemyRandom[2][128],attackRandom[256];
static U32 Mix(U32 x) {
    x^=x>>16;x*=0x7feb352dU;x^=x>>15;x*=0x846ca68bU;return x^(x>>16);
}
void InitEnemyRandom(Enemy *enemy,int sub) {
    if(!Active() || enemy->index<0 || enemy->index>=128) return;
    U32 seed=Mix(g_NetManager.Match().seed ^ U32(g_Rng.GetSeed()) ^ U32(sub*397) ^ U32(enemy->manager->sideIdx*65537));
    Rng &rng=enemyRandom[enemy->manager->sideIdx][enemy->index];
    rng.SetSeed(WORD(seed^(seed>>16)));rng.ResetGenerated();
}
void InitAttackRandom(ExAttack *attack,int type) {
    if(!Active()) return;
    int index=int(attack-g_GameManager.exAttackManager->attacks);if(index<0 || index>=256)return;
    U32 seed=Mix(g_NetManager.Match().seed ^ U32(g_Rng.GetSeed()) ^ U32(type*65537));
    attackRandom[index].SetSeed(WORD(seed^(seed>>16)));attackRandom[index].ResetGenerated();
}
PatternRandom::PatternRandom(Enemy *enemy):side_(enemy->manager->sideIdx),index_(enemy->index),extra_(false),engaged_(Active()) {
    if(!engaged_ || side_<0 || side_>1 || index_<0 || index_>=128) { engaged_=false;return; }
    seed_=g_Rng.seed;generated_=g_Rng.generated;g_Rng=enemyRandom[side_][index_];
}
PatternRandom::PatternRandom(ExAttack *attack):side_(0),index_(0),extra_(true),engaged_(Active()) {
    if(!engaged_)return;
    index_=int(attack-g_GameManager.exAttackManager->attacks);
    if(index_<0 || index_>=256) { engaged_=false;return; }
    seed_=g_Rng.seed;generated_=g_Rng.generated;g_Rng=attackRandom[index_];
}
PatternRandom::~PatternRandom() {
    if(!engaged_)return;
    if(extra_) attackRandom[index_]=g_Rng;else enemyRandom[side_][index_]=g_Rng;
    g_Rng.seed=seed_;g_Rng.generated=generated_;
}
WaveRandom::WaveRandom(int side,unsigned sequence,unsigned frame):engaged_(Active()) {
    if(!engaged_)return;
    seed_=g_Rng.seed;generated_=g_Rng.generated;
    U32 seed=Mix(g_NetManager.Match().seed ^ U32(side*65537) ^ Mix(sequence) ^ Mix(frame) ^ U32(g_GameManager.unk_100*397));
    g_Rng.SetSeed(WORD(seed^(seed>>16)));g_Rng.ResetGenerated();
}
WaveRandom::~WaveRandom() { if(engaged_) {g_Rng.seed=seed_;g_Rng.generated=generated_;} }
ActorScope::ActorScope() { ++actorDepth; }
ActorScope::~ActorScope() { --actorDepth; }
bool Active() { return active && g_NetManager.GetState()==NetManager::Matched; }
bool RemotePlayer(int side) { return Active() && !coop::Active() && side!=g_NetManager.Match().role; }
bool NetworkPaused() { return pauseFlags[0] || pauseFlags[1]; }
void MarkGame(bool network) {customGame=true;customNetwork=network;}
bool ConsumeTitleReturn() {
    if(!customGame)return false;
    customGame=false;titleCursor=customNetwork?4:3;return true;
}
void EndSession() {
    if(trace) { fclose(trace);trace=0; }
    active=started=false;applying=false;tick=roundStart=0;
    g_GameManager.stateFlags&=~0x800;
    pauseFlags[0]=pauseFlags[1]=false;
    if(coop::Active()) coop::Stop();
    g_NetManager.Close();
}
static void Text(float x,float y,DWORD color,const char *format,int value=0) {
    D3DXVECTOR3 p(x,y,0);
    g_AsciiText.color=color;g_AsciiText.SetScale(1,1);g_AsciiText.inArena=0;
    ui::Format(x,y,color,format,value);
}
void OpenLobby(MainMenu *menu) {
    EndSession();g_NetManager.Open();lobbyCursor=0;roomCode=0;digitsDown=0;roomWritten=false;
    g_GameManager.unk_114=0;
    disconnectNotice[0]=0;
    menu->SetState(17);menu->helpSprite=0;
}
int TitleUpdate(MainMenu *menu) {
    if(forwardingTitle || menu->subState!=1) return -1;
    char value[32];
    if(!testStarted && GetEnvironmentVariableA("TH09_NET_AUTOTEST",value,sizeof(value))) {
        testStarted=true;testing=true;bool host=strcmp(value,"host")==0;
        OpenLobby(menu);
        if(GetEnvironmentVariableA("TH09_TEST_MODE",value,sizeof(value))) mode=(strcmp(value,"boss")==0 || strcmp(value,"story")==0) ? 1:0;
        character=host?0:1;
        if(GetEnvironmentVariableA("TH09_TEST_CHARACTER",value,sizeof(value))) character=atoi(value)%16;
        if(GetEnvironmentVariableA("TH09_TEST_DIFFICULTY",value,sizeof(value))) difficulty=atoi(value)%4;
        if(host) g_NetManager.Join(CreateRoom,U8(character),U8(difficulty),0,U8(mode),U8(0));
        else {
            GetEnvironmentVariableA("TH09_TEST_ROOM",value,sizeof(value));
            g_NetManager.Join(JoinRoom,U8(character),U8(difficulty),U16(atoi(value)),U8(mode),U8(0));
        }
        return 1;
    }
    int autoResult=coop::AutoStart(menu);if(autoResult>=0) return autoResult;
    int old=menu->cursor;menu->cursor=titleCursor;menu->MoveMenuCursor(10);
    titleCursor=menu->cursor;menu->cursor=old;menu->idleFrames=0;
    if(g_PlayerInputs[2].unk_6&10) titleCursor=9;
    if(!(g_PlayerInputs[2].unk_6&0x1001)) return 1;
    if(titleCursor==3) { coop::OpenSetup(menu);return 1; }
    if(titleCursor==4) { OpenLobby(menu);return 1; }
    menu->cursor=titleCursor>4 ? titleCursor-2:titleCursor;


    menu->inputLockout=10;
    WORD pressed=g_PlayerInputs[2].unk_6,repeated=g_PlayerInputs[2].unk_4;
    g_PlayerInputs[2].unk_6&=~0x30;g_PlayerInputs[2].unk_4&=~0x30;
    forwardingTitle=true;int result=menu->UpdateState1();forwardingTitle=false;
    g_PlayerInputs[2].unk_6=pressed;g_PlayerInputs[2].unk_4=repeated;return result;
}
int LobbyUpdate(MainMenu *menu) {
    if(g_PlayerInputs[2].unk_6&10) {
        if(g_NetManager.GetState()==NetManager::Searching) {
            g_NetManager.Close();g_NetManager.Open();return 1;
        }
        EndSession();menu->SetState(1);return 1;
    }
    if(testing && g_NetManager.Room() && !roomWritten) {
        FILE *file=fopen("net-room.txt","wb");
        if(file) { fprintf(file,"%04d",g_NetManager.Room());fclose(file);roomWritten=true; }
    }
    if(g_NetManager.GetState()==NetManager::Matched) {
        const MatchData &m=g_NetManager.Match();
        active=true;started=false;tick=roundStart=sentAttacks=receivedAttacks=0;
        MarkGame(true);
        resultFrames=0;
        previousRemoteInput=0;pauseFlags[0]=pauseFlags[1]=false;
        if(testing) trace=fopen("net-trace.jsonl","wb");
        if(m.mode==1) { coop::StartNetwork();return 0; }
        g_GameManager.stateFlags=0;g_GameManager.mode=2;g_GameManager.unk_114=0;
        g_GameManager.unk_fc=9;g_GameManager.unk_100=0;g_GameManager.difficulty=m.difficulty;
        g_GameManager.initialLives[0]=g_GameManager.initialLives[1]=10;
        g_GameManager.sides[0].character=m.character0;g_GameManager.sides[1].character=m.character1;
        g_GameManager.sides[0].unk_28=g_GameManager.sides[1].unk_28=0;
        g_GameManager.sides[0].unk_30=g_GameManager.sides[1].unk_30=0;
        g_Rng.SetSeed(WORD(m.seed^(m.seed>>16)));g_Rng.ResetGenerated();
        g_App.StopMusic();g_App.scene=2;return 0;
    }
    if(g_NetManager.GetState()!=NetManager::Lobby) return 1;
    int old=menu->cursor;menu->cursor=lobbyCursor;
    int moved=menu->MoveMenuCursor(6);lobbyCursor=menu->cursor;menu->cursor=old;

    int delta=(g_PlayerInputs[2].IsPressed(0x80)?1:0)-(g_PlayerInputs[2].IsPressed(0x40)?1:0);
    if(lobbyCursor==0) {mode=(mode+delta+2)%2;if(mode)character%=14;}
    if(lobbyCursor==1) {int count=mode?14:16;character=(character+delta+count)%count;}
    if(lobbyCursor==2) difficulty=(difficulty+delta+4)%4;

    unsigned down=0;
    if(lobbyCursor==5) for(int d=0;d<10;++d)
        if((GetAsyncKeyState('0'+d)&0x8000)||(GetAsyncKeyState(VK_NUMPAD0+d)&0x8000)) {
            down|=1u<<d;if(!(digitsDown&(1u<<d))) roomCode=U16((roomCode%1000)*10+d);
        }
    if(GetAsyncKeyState(VK_BACK)&0x8000) { down|=1024;if(!(digitsDown&1024)) roomCode/=10; }
    digitsDown=down;
    if(g_PlayerInputs[2].unk_6&0x1001) {
        if(lobbyCursor==0) {mode=1-mode;if(mode)character%=14;}
        else if(lobbyCursor==1) character=(character+1)%(mode?14:16);
        else if(lobbyCursor==2) difficulty=(difficulty+1)%4;

        else if(lobbyCursor==3) g_NetManager.Join(JoinQueue,U8(character),U8(difficulty),0,U8(mode),U8(0));
        else if(lobbyCursor==4) g_NetManager.Join(CreateRoom,U8(character),U8(difficulty),0,U8(mode),U8(0));
        else if(lobbyCursor==5) g_NetManager.Join(JoinRoom,U8(character),U8(difficulty),roomCode,U8(mode),U8(0));
    }
    return 1;
}
void DrawMenu(MainMenu *menu) {
    DWORD color=g_AsciiText.color;float sx=g_AsciiText.scaleX,sy=g_AsciiText.scaleY;int arena=g_AsciiText.inArena;
    if(menu->state==17 || menu->state==18) {
        g_SpriteEngine->FlushQuadBatch();
        D3DRECT panel={42,43,601,471};
        g_App.d3dDevice->Clear(1,&panel,D3DCLEAR_TARGET,0xff10202e,1,0);
    }
    if(menu->state==1) {
        static const char *entries[]={"Story Mode","Extra Mode","Match Mode","Local Story Co-op","NETPLAY","Replay","Result","Music Room","Option","Quit"};
        for(int i=0;i<10;++i) {
            DWORD tint=i==1 && !g_PlayRecord.openEntries3[12]?0xff506070:i==titleCursor?0xffffe0a0:0xffd0d8ff;
            float y=146.0f+i*25;
            ui::Title(385,y+4,tint,entries[i]);
        }
        if(disconnectNotice[0]) Text(50,440,0xffff8080,disconnectNotice);
    } else if(menu->state==17) {
        ui::Title(150,55,0xffffff80,"NETPLAY");
        Text(120,88,0xffffffff,g_NetManager.Online()?"Server: Online":"Server: Offline");
        if(g_NetManager.Online()) Text(380,88,0xffffffff,"Ping: %d ms",g_NetManager.PingMs());
        for(int i=0;i<6;++i) {
            D3DXVECTOR3 p(125,145.0f+i*31,0);g_AsciiText.color=i==lobbyCursor?0xffffff80:0xffc0d0ff;
            if(i==0) ui::Format(p.x,p.y,g_AsciiText.color,"Mode: %s",mode?"Story Co-op":"Duel 1 vs 1");
            else if(i==1) ui::Format(p.x,p.y,g_AsciiText.color,"Character: %s",g_ReplayMenuCharacterNames[character]);
            else if(i==2) ui::Format(p.x,p.y,g_AsciiText.color,"Difficulty: %s",g_ReplayMenuDifficultyNames[difficulty]);
            else if(i==3) ui::Format(p.x,p.y,g_AsciiText.color,"Quick Match");
            else if(i==4) ui::Format(p.x,p.y,g_AsciiText.color,"Create Room");
            else ui::Format(p.x,p.y,g_AsciiText.color,"Join Room: %04d",roomCode);
        }
        if(g_NetManager.GetState()==NetManager::Searching) {
            Text(125,385,0xffffffff,"Waiting for another player...");
            if(g_NetManager.Room()) Text(125,410,0xffffff80,"Room code: %04d",g_NetManager.Room());
        } else if(g_NetManager.GetState()==NetManager::Disconnected)
            Text(80,395,0xffff8080,g_NetManager.ErrorText());
        Text(70,450,0xffc0c0c0,"Arrows: choose | Z: confirm | X/Esc: back");
    } else if(menu->state==18) coop::DrawSetup();
    g_AsciiText.color=color;g_AsciiText.SetScale(sx,sy);g_AsciiText.inArena=arena;
}
void BeginFrame() {
    if(coop::Active() && g_App.scene==9)coop::OnEnding();
    bool storyTransition=coop::Active() && (g_App.scene==3 || g_App.scene==9 || g_App.scene==10 || g_App.scene==11 || g_App.scene==12 || g_App.scene==13);
    if(coop::Active() && g_App.scene!=2 && !storyTransition) coop::Stop();
    if(active && g_App.scene!=2 && !storyTransition) {
        bool test=testing;EndSession();if(test) g_GameWindow.isClosing=1;return;
    }
    g_NetManager.Poll();
}
bool WaitForPeer() {
    if(!active) return coop::WaitForInputs();
    if(g_NetManager.GetState()!=NetManager::Matched) {
        if(coop::Active()) return coop::WaitForInputs();
        if(g_GameManager.unk_108[0]>=2 || g_GameManager.unk_108[1]>=2)
            strcpy(disconnectNotice,"Match finished. NETPLAY: create or join a new room.");
        else strncpy(disconnectNotice,g_NetManager.ErrorText(),sizeof(disconnectNotice)-1);
        EndSession();g_App.scene=1;if(testing)g_GameWindow.isClosing=1;return true;
    }
    if(coop::Networked()) coop::Prepare();
    g_NetManager.MarkReady();
    if(!g_NetManager.CanSimulate()) {
        Text(135,220,0xffffffff,"Waiting for both players to load...");
        return true;
    }
    if(!started) { started=true;tick=g_NetManager.StartTick();roundStart=tick; }
    return coop::Networked() ? coop::WaitForInputs():false;
}
bool HandlePause(const PacketGameEvent &event) {
    if(event.type!=PauseState) return false;
    int side=int(event.params[0]);
    if(side==1-int(g_NetManager.Match().role) && (event.params[1]==0 || event.params[1]==1))
        pauseFlags[side]=event.params[1]!=0;
    if(NetworkPaused()) g_GameManager.stateFlags|=0x800;
    else if(!coop::Active()) g_GameManager.stateFlags&=~0x800;
    return true;
}
static void PollPause() {
    if(g_PlayerInputs[2].unk_6&8) {
        int side=g_NetManager.Match().role;pauseFlags[side]=!pauseFlags[side];
        PacketGameEvent event;memset(&event,0,sizeof(event));event.type=PauseState;event.tick=tick;
        event.params[0]=float(side);event.params[1]=pauseFlags[side]?1.0f:0.0f;g_NetManager.SendEvent(event);
        g_PlayerInputs[2].unk_6&=~8;
    }
    if(NetworkPaused()) g_GameManager.stateFlags|=0x800;
    else if(!coop::Active() || (g_GameManager.stateFlags&0x800)) g_GameManager.stateFlags&=~0x800;
}
static U32 EventAge(U32 eventTick) { U32 age=Newer(tick,eventTick)?tick-eventTick:0;return age>30?30:age; }
class SpawnSnapshot {
    bool enemies_[2][128],attacks_[256],projectiles_[2][0x248];
public:
    SpawnSnapshot() {
        for(int side=0;side<2;++side) {
            for(int i=0;i<128;++i) enemies_[side][i]=g_GameManager.sides[side].enemyManager->enemies[i].active!=0;
            g_GameManager.sides[side].bulletManager->SnapshotProjectiles(projectiles_[side]);
        }
        for(int i=0;i<256;++i)attacks_[i]=g_GameManager.exAttackManager->attacks[i].active!=0;
    }
    void Advance(U32 age) {
        ActorScope nested;
        for(U32 n=0;n<age;++n) {
            for(int side=0;side<2;++side) {
                EnemyManager *manager=g_GameManager.sides[side].enemyManager;
                for(int i=0;i<128;++i) if(!enemies_[side][i] && manager->enemies[i].active) {
                    Enemy *enemy=&manager->enemies[i];
                    if(manager->ecl.RunEnemy(enemy)==-1) { enemy->Despawn();enemy->active=0;continue; }
                    enemy->ClampPos();enemy->Move();enemy->ClampPos();
                    enemy->absPos=enemy->pos+enemy->posOrigin;
                    if(enemy->invulnTimer>0)enemy->invulnTimer--;
                    g_SpriteEngine->Step(&enemy->primarySprite);
                }
            }
            for(int i=0;i<256;++i) {
                ExAttack *attack=&g_GameManager.exAttackManager->attacks[i];
                if(attacks_[i] || !attack->active)continue;
                PatternRandom random(attack);
                if(attack->onUpdate(attack)) {
                    if(attack->onFree)attack->onFree(attack);
                    if(attack->sprites){g_MemoryTracker.Free(attack->sprites);attack->sprites=0;}
                    attack->active=0;continue;
                }
                for(int s=0;s<attack->spriteCount;++s)g_SpriteEngine->Step(&attack->sprites[s]);
                attack->timer++;
            }
            for(int side=0;side<2;++side)g_GameManager.sides[side].bulletManager->AdvanceNewProjectiles(projectiles_[side]);
        }
    }
};
bool RouteRoundWin(int side) {
    if(!Active() || coop::Active() || applying) return false;
    if(g_NetManager.Match().role!=0) return true;
    if(g_GameManager.gui->GetUnknown11ea8()) return true;
    PacketGameEvent event;memset(&event,0,sizeof(event));event.tick=tick;event.type=RoundOver;
    event.params[0]=float(side);event.params[1]=float(g_GameManager.unk_100);
    g_NetManager.SendEvent(event);return false;
}
void RoundReset() {
    if(!Active() || coop::Active()) return;
    roundStart=tick;previousRemoteInput=0;
    U32 seed=g_NetManager.Match().seed ^ (U32(g_GameManager.unk_100)*0x9e3779b9U);
    g_Rng.SetSeed(WORD(seed^(seed>>16)));g_Rng.ResetGenerated();
}
void RoundWon(int side) {
    if(!Active() || coop::Active() || !testing) return;
    FILE *file=fopen("net-results.jsonl","ab");
    if(file) {
        fprintf(file,"{\"role\":%u,\"match\":%u,\"seed\":%u,\"tick\":%u,\"round\":%d,\"winner\":%d,\"wins\":[%d,%d],\"maxPollUs\":%u}\n",
            unsigned(g_NetManager.Match().role),g_NetManager.Match().match,g_NetManager.Match().seed,tick,g_GameManager.unk_100,
            side,g_GameManager.unk_108[0],g_GameManager.unk_108[1],g_NetManager.MaxPollUs());fclose(file);
    }
}
bool PostMatchUpdate() {
    if(!Active() || coop::Active())return false;
    DWORD color=g_AsciiText.color;float sx=g_AsciiText.scaleX,sy=g_AsciiText.scaleY;int arena=g_AsciiText.inArena;
    Text(125,215,0xffffff80,"NETPLAY MATCH FINISHED");
    D3DXVECTOR3 pos(155,245,0);g_AsciiText.color=0xffffffff;
    ui::Format(pos.x,pos.y,g_AsciiText.color,"Player %d wins!  Score: %d - %d",
        g_GameManager.unk_108[0]>g_GameManager.unk_108[1]?1:2,g_GameManager.unk_108[0],g_GameManager.unk_108[1]);
    Text(120,290,0xffffffff,"Z/Enter or X/Esc: return to title");
    Text(120,320,0xffb0d0ff,"Open NETPLAY to start the next match.");
    g_AsciiText.color=color;g_AsciiText.SetScale(sx,sy);g_AsciiText.inArena=arena;
    ++resultFrames;
    if((resultFrames>20 && (g_PlayerInputs[2].unk_6&0x100b)) || (testing && resultFrames>=180))g_App.scene=1;
    return true;
}
void BeginGameUpdate() {
    if(!active) { coop::Update();return; }
    if(coop::Networked()) { coop::Update();return; }
    PollPause();
    if(!Active()) return;
    int local=g_NetManager.Match().role;
    g_PlayerInputs[local]=g_PlayerInputs[2];
    if(testing) {
        if(g_GameManager.gui->IsMsgRunning()) {
            g_PlayerInputs[2].unk_0|=0x100;
            g_PlayerInputs[2].unk_2c|=0x100;
            if(tick%30==0) g_PlayerInputs[2].unk_6|=1;
        }
        Player *p=g_GameManager.sides[local].player;
        if(p) {
            float target=sinf(tick*.017f+local*3.1415927f)*95;
            WORD input=1;
            if(p->pos.x<target-3) input|=0x80;if(p->pos.x>target+3) input|=0x40;
            if(p->pos.y<380) input|=0x20;if(p->pos.y>390) input|=0x10;
            if(p->unk_30388>=200) input|=2;
            WORD previous=g_PlayerInputs[local].unk_2c;
            g_PlayerInputs[local].unk_0=g_PlayerInputs[local].unk_2c=input;
            g_PlayerInputs[local].unk_6=g_PlayerInputs[local].unk_32=WORD(input&~previous);
        }
    }
    PlayerInput &remote=g_PlayerInputs[1-local];float x,y;U16 input=previousRemoteInput;
    g_NetManager.RemotePosition(tick,x,y,input);
    remote.unk_0=remote.unk_2c=input;remote.unk_6=remote.unk_32=WORD(input&~previousRemoteInput);
    remote.unk_4=remote.unk_30=0;previousRemoteInput=input;
    PacketGameEvent event;
    while(g_NetManager.PopEvent(event)) {
        if(HandlePause(event)) continue;
        if(Newer(roundStart,event.tick)) continue;
        Player *other=g_GameManager.sides[1-local].player;if(!other) continue;
        applying=true;
        if(event.type==PlayerHit && event.params[0]>=0 && event.params[0]<=10) {
            other->unk_a8=int(event.params[0]);other->SetPlayerState(4);other->stateTimer=int(EventAge(event.tick));
            if(!other->unk_a8 && local==0 && !g_GameManager.gui->GetUnknown11ea8()) {
                applying=false;g_GameManager.gui->CountRoundWin(local);applying=true;
            }
        } else if(event.type==DeathbombUsed && event.params[0]>=0 && event.params[0]<=10) {
            other->unk_a8=int(event.params[0]);other->StartClearCircle1();
        } else if(event.type==RoundOver && local==1 && event.params[0]>=0 && event.params[0]<=1 &&
            int(event.params[1])==g_GameManager.unk_100 && !g_GameManager.gui->GetUnknown11ea8()) {
            g_GameManager.gui->CountRoundWin(int(event.params[0]));
        } else if(event.type==SpawnExAttack && event.params[0]>=0 && event.params[0]<27 &&
            event.params[1]>=-1000 && event.params[1]<=1000 && event.params[2]>=-1000 && event.params[2]<=1000) {
            D3DXVECTOR3 pos(event.params[1],event.params[2],event.params[3]);
            SpawnSnapshot snapshot;
            Rng saved=g_Rng;g_Rng.SetSeed(WORD(event.rngSeed));
            g_GameManager.exAttackManager->AddAttack(int(event.params[0]),&pos,1-local,0);
            snapshot.Advance(EventAge(event.tick));g_Rng=saved;++receivedAttacks;
        } else if(event.type==SpawnBoss && event.params[0]>=0 && event.params[0]<9 &&
            event.params[1]>=0 && event.params[1]<3) {
            SpawnSnapshot snapshot;
            Rng saved=g_Rng;g_Rng.SetSeed(WORD(event.rngSeed));
            int kind=int(event.params[1]);other->unk_a0=other->unk_a4=int(event.params[3]);
            g_GameManager.sides[local].spellCard->Start(int(event.params[0]),kind,int(event.params[2]),other->shtFile->spellCardNames[kind]);
            snapshot.Advance(EventAge(event.tick));g_Rng=saved;++receivedAttacks;
        }
        applying=false;
    }
    if(NetworkPaused()) {
        Text(210,190,0xffffff80,"NETWORK PAUSED");Text(95,230,0xffffffff,"Esc: toggle your pause | X: leave game");
        if(g_PlayerInputs[2].unk_6&2){EndSession();g_App.scene=1;}
        return;
    }
    if(trace && tick%60==0) {
        Player *a=g_GameManager.sides[0].player,*b=g_GameManager.sides[1].player;
        fprintf(trace,"{\"tick\":%u,\"role\":%d,\"health\":[%d,%d],\"wins\":[%d,%d],\"sent\":%u,\"received\":%u,\"players\":[[%.2f,%.2f],[%.2f,%.2f]]}\n",
            tick,local,a->unk_a8,b->unk_a8,g_GameManager.unk_108[0],g_GameManager.unk_108[1],sentAttacks,receivedAttacks,
            a->pos.x,a->pos.y,b->pos.x,b->pos.y);fflush(trace);
    }
    if(testing && tick>=21600) g_GameWindow.isClosing=1;
    ++tick;
}
void BeforePlayer(Player *player) {
    if(coop::Active() || !RemotePlayer(player->sideIdx)) return;
    float x,y;U16 input;U32 frame=coop::Networked()?coop::Tick():tick;
    if(g_NetManager.RemotePosition(frame,x,y,input)) { player->pos.x=x;player->pos.y=y; }
}
void AfterPlayer(Player *player) {
    if(coop::Active() || !Active() || RemotePlayer(player->sideIdx) || NetworkPaused()) return;
    FrameSample sample;sample.tick=coop::Networked()?coop::Tick():tick;
    sample.x=player->pos.x;sample.y=player->pos.y;sample.inputMask=g_PlayerInputs[player->sideIdx].unk_0;
    g_NetManager.SendFrame(sample);
}
bool BeforeDeath(Player *player) { return !Active() || applying || !RemotePlayer(player->sideIdx); }
DeathScope::DeathScope(Player *player):player_(player),health_(player->unk_a8),state_(player->playerState) {}
DeathScope::~DeathScope() {
    coop::AfterDeath(player_,state_);
    if(coop::Active() || !Active() || applying || RemotePlayer(player_->sideIdx)) return;
    if(player_->unk_a8==health_ && player_->playerState==state_) return;
    PacketGameEvent event;memset(&event,0,sizeof(event));event.tick=tick;
    event.type=player_->playerState==4 || player_->unk_a8<health_?PlayerHit:DeathbombUsed;
    event.params[0]=float(player_->unk_a8);g_NetManager.SendEvent(event);
}
bool RouteExAttack(int type,D3DXVECTOR3 *pos,int side) {
    if(!Active() || applying || actorDepth || coop::Active()) return false;
    if(RemotePlayer(side)) return true;
    PacketGameEvent event;memset(&event,0,sizeof(event));event.tick=tick;event.type=SpawnExAttack;
    event.rngSeed=g_Rng.GetSeed();event.params[0]=float(type);
    event.params[1]=pos->x;event.params[2]=pos->y;event.params[3]=pos->z;
    g_NetManager.SendEvent(event);++sentAttacks;return false;
}
bool RouteSpell(SpellCard *card,int type,int kind,int unknown) {
    if(!Active() || applying || actorDepth || coop::Active()) return false;
    if(!RemotePlayer(card->sideIdx)) return true;
    PacketGameEvent event;memset(&event,0,sizeof(event));event.tick=tick;event.type=SpawnBoss;event.rngSeed=g_Rng.GetSeed();
    event.params[0]=float(type);event.params[1]=float(kind);event.params[2]=float(unknown);
    event.params[3]=float(kind<2?card->otherSide->player->unk_a0:card->otherSide->player->unk_a4);
    g_NetManager.SendEvent(event);++sentAttacks;return false;
}
}
#endif
