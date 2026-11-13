#ifdef TH09_NATIVE_NETPLAY
#include "CoopGame.h"
#include "NetGame.h"
#include "UiText.h"
#include "mainmenu.h"
#include "ascii.h"
#include "gamemanager.h"
#include "player.h"
#include "enemy.h"
#include "gui.h"
#include <math.h>

namespace coop {
static bool enabled=false, networked=false, creating=false, autoPlay=false, autoStarted=false;
static bool paused=false, sentInput=false;
static bool transitionTest=false, endingSaved=false;
static unsigned visitedStages=0, stagePlayTicks=0;
static int selected[2]={0,1}, difficulty=1, cursor=0;
static Player *companion=0;
static Player *leader=0;
static PlayerInput controls[2];
static PlayerInput dialogueInput;
static unsigned tick=0, smokeTicks=0, shots[2]={0,0}, damage[2]={0,0};
static FILE *trace=0;
static ChainElem *inputChain=0;
static const unsigned Delay=6, StorySignature=0x53544f52;
struct InputSlot { unsigned tick; WORD mask; bool used; };
static InputSlot inputs[2][256];
static WORD masks[2]={0,0};
bool Active() { return enabled; }
bool Networked() { return enabled && networked; }
bool Spawning() { return true; }
unsigned Tick() { return tick; }
bool Paused() { return enabled && paused; }
bool OwnPlayerAssets(const Player *player) { return creating || player==companion; }
int CompanionCharacter() { return selected[1]; }
Player *Companion() { return enabled ? companion:0; }
bool IsCompanion(const Player *player) { return player && player==companion; }
NativeScope::NativeScope():engaged_(false) {}
NativeScope::~NativeScope() {}
bool BossProjection(const Enemy *) { return false; }
bool DamageBoss(Enemy *,int) { return false; }
bool Support(int,int) { return false; }
static void Text(float x,float y,DWORD color,const char *text) { ui::Format(x,y,color,"%s",text); }
static void SetInput(PlayerInput &input,WORD mask) {
    WORD previous=input.unk_2c;
    input.unk_0=input.unk_2c=mask;
    input.unk_6=input.unk_32=WORD(mask&~previous);
    input.unk_2=input.unk_2e=WORD(previous&~mask);
    input.unk_4=input.unk_30=0;
    input.unk_8=input.unk_34=WORD(mask^previous);
}
void OnEnding() {
    if(!enabled || endingSaved)return;
    endingSaved=true;
    FILE *file=fopen("coop-results.jsonl","ab");
    if(file) {
        fprintf(file,"{\"victory\":true,\"story\":true,\"tick\":%u,\"stages\":%u,\"transitionTest\":%s}\n",tick,visitedStages,transitionTest?"true":"false");
        fclose(file);
    }
    if(transitionTest && !networked)g_GameWindow.isClosing=1;
}
void Unload() {
    if(inputChain) {g_Chain.Remove(inputChain);inputChain=0;}
    if(companion) { Player::RemoveChain(companion);companion=0; }
    leader=0;
}
static int InputUpdate(void *) {
    if(!enabled)return 1;
    if(g_GameManager.loadingThreadState)return 1;
    if(thnet::WaitForPeer())return 3;
    thnet::BeginGameUpdate();
    return paused || g_App.scene!=2 ? 3:1;
}
void InstallInputChain() {
    if(!enabled || inputChain)return;
    inputChain=g_Chain.CreateElem(InputUpdate);
    g_Chain.AddToCalcChain(inputChain,0);
}
void Stop() {
    enabled=false;paused=false;g_GameManager.stateFlags&=~0x800;
    if(trace) { fclose(trace);trace=0; }
    if(networked)thnet::g_NetManager.Close();
    networked=false;
}
void OpenSetup(MainMenu *menu) { thnet::EndSession();cursor=0;menu->SetState(18);menu->helpSprite=0; }
static int StartGame(bool online) {
    if(!online)thnet::EndSession();
    enabled=true;networked=online;paused=sentInput=false;tick=0;
    memset(inputs,0,sizeof(inputs));memset(controls,0,sizeof(controls));memset(&dialogueInput,0,sizeof(dialogueInput));
    masks[0]=masks[1]=0;shots[0]=shots[1]=damage[0]=damage[1]=0;
    for(unsigned n=0;n<Delay;++n)for(int p=0;p<2;++p) {
        inputs[p][n].used=true;inputs[p][n].tick=n;inputs[p][n].mask=0;
    }
    char value[32];
    endingSaved=false;visitedStages=stagePlayTicks=0;
    autoPlay=GetEnvironmentVariableA("TH09_COOP_AUTOTEST",value,sizeof(value))!=0 ||
        GetEnvironmentVariableA("TH09_NET_AUTOTEST",value,sizeof(value))!=0;
    transitionTest=autoPlay && !online && GetEnvironmentVariableA("TH09_STORY_TRANSITION_TEST",value,sizeof(value))!=0;
    smokeTicks=0;
    if(autoPlay && GetEnvironmentVariableA("TH09_SMOKE_TICKS",value,sizeof(value)))smokeTicks=unsigned(atoi(value));
    if(autoPlay || GetEnvironmentVariableA("TH09_COOP_TRACE",value,sizeof(value)))trace=fopen("coop-trace.jsonl","wb");
    thnet::MarkGame(online);
    g_GameManager.stateFlags=0;g_GameManager.mode=0;g_GameManager.unk_114=0;
    g_GameManager.unk_fc=0;g_GameManager.unk_100=0;g_GameManager.difficulty=difficulty;
    g_GameManager.sides[0].character=selected[0];g_GameManager.sides[1].character=0;
    g_GameManager.sides[0].unk_28=0;g_GameManager.sides[1].unk_28=1;
    g_GameManager.sides[0].unk_30=g_GameManager.sides[1].unk_30=0;
    g_GameManager.initialLives[0]=10;g_GameManager.initialLives[1]=20;
    unsigned seed=online?thnet::g_NetManager.Match().seed:0x909;
    g_Rng.SetSeed(WORD(seed^(seed>>16)));g_Rng.ResetGenerated();
    g_App.StopMusic();g_App.scene=2;return 0;
}
void StartNetwork() {
    const thnet::MatchData &match=thnet::g_NetManager.Match();
    selected[0]=match.character0%14;selected[1]=match.character1;
    difficulty=match.difficulty;StartGame(true);
}
int AutoStart(MainMenu *) {
    char value[16];
    if(autoStarted || !GetEnvironmentVariableA("TH09_COOP_AUTOTEST",value,sizeof(value)))return -1;
    autoStarted=true;
    if(GetEnvironmentVariableA("TH09_TEST_DIFFICULTY",value,sizeof(value)))difficulty=atoi(value)%4;
    if(GetEnvironmentVariableA("TH09_TEST_CHARACTER",value,sizeof(value)))selected[0]=atoi(value)%14;
    if(GetEnvironmentVariableA("TH09_TEST_CHARACTER2",value,sizeof(value)))selected[1]=atoi(value)%16;
    return StartGame(false);
}
int UpdateSetup(MainMenu *menu) {
    if(g_PlayerInputs[2].unk_6&10) { menu->SetState(1);return 1; }
    int old=menu->cursor;menu->cursor=cursor;menu->MoveMenuCursor(4);cursor=menu->cursor;menu->cursor=old;
    int delta=(g_PlayerInputs[2].IsPressed(0x80)?1:0)-(g_PlayerInputs[2].IsPressed(0x40)?1:0);
    if(g_PlayerInputs[2].unk_6&0x1001) {if(cursor==3)return StartGame(false);delta=1;}
    if(cursor==0)selected[0]=(selected[0]+delta+14)%14;
    else if(cursor==1)selected[1]=(selected[1]+delta+16)%16;
    else if(cursor==2)difficulty=(difficulty+delta+4)%4;
    return 1;
}
void DrawSetup() {
    ui::Title(120,65,0xffffff80,"STORY CO-OP");
    for(int i=0;i<4;++i) {
        DWORD color=i==cursor?0xffffff80:0xffc0e0ff;
        if(i<2)ui::Format(100,125.0f+i*37,color,"Player %d: %s",i+1,g_ReplayMenuCharacterNames[selected[i]]);
        else if(i==2)ui::Format(100,199,color,"Difficulty: %s",g_ReplayMenuDifficultyNames[difficulty]);
        else Text(100,236,color,"START STORY TOGETHER");
    }
    Text(60,310,0xffffffff,"Both players share the LEFT field. Boss HP x2.");
    Text(60,350,0xffb0d0ff,"P1: Arrows / Z shoot-charge / X attack / Shift focus");
    Text(60,380,0xffb0d0ff,"P2: WASD / F shoot-charge / G attack / H focus");
    Text(60,420,0xffb0d0ff,"The host's character determines the story route.");
}
class CompanionScope {
    Player *leader_;int character_;PlayerInput input_;
public:
    CompanionScope():leader_(g_GameManager.sides[0].player),character_(g_GameManager.sides[0].character),input_(g_PlayerInputs[0]) {
        g_GameManager.sides[0].player=companion;g_GameManager.sides[0].character=selected[1];g_PlayerInputs[0]=controls[1];
    }
    ~CompanionScope() {
        g_GameManager.sides[0].player=leader_;g_GameManager.sides[0].character=character_;g_PlayerInputs[0]=input_;
    }
};
static int CompanionUpdate(Player *player) {
    if(player->GetPlayerState()==5 && player->unk_a8>0 && leader->GetPlayerState()!=5) {
        player->SetPlayerState(1);player->stateTimer=0;
    }
    CompanionScope scope;int result=Player::OnUpdate(player);
    if(player->GetPlayerState()!=5 && player->unk_a8>0)player->sprite.color=0xff80e8ff;
    return result;
}
static int CompanionDraw(Player *player) {CompanionScope scope;return Player::OnDraw(player);}
static int CompanionDraw2(Player *player) {CompanionScope scope;return Player::OnDraw2(player);}
void Prepare() {
    if(!enabled || companion || g_GameManager.loadingThreadState || !g_GameManager.sides[0].player)return;
    leader=g_GameManager.sides[0].player;
    visitedStages|=1u<<g_GameManager.unk_fc;stagePlayTicks=0;
    g_GameManager.config->unk_b4[0]=0;
    creating=true;int character=g_GameManager.sides[0].character;
    g_GameManager.sides[0].character=selected[1];companion=Player::RegisterChain(0,0,0);
    g_GameManager.sides[0].character=character;creating=false;
    companion->calcElem->callback=(ChainCallback)CompanionUpdate;
    companion->drawElem->callback=(ChainCallback)CompanionDraw;
    companion->drawElem2->callback=(ChainCallback)CompanionDraw2;
    unsigned seed=networked?thnet::g_NetManager.Match().seed:0x909;
    seed^=unsigned(g_GameManager.unk_fc)*0x9e3779b9U;
    g_Rng.SetSeed(WORD(seed^(seed>>16)));g_Rng.ResetGenerated();
}
void ResetRound() {if(companion) {CompanionScope scope;companion->Reset();}}
void BeforeLeader(Player *player) {
    if(!enabled || !companion || player==companion || player->sideIdx!=0)return;
    companion->unk_36c=player->unk_36c;companion->unk_36c.player=companion;
}
bool AllowDeath(Player *) {return true;}
void AfterDeath(Player *,int) {}
bool SuppressRoundWin(int side) {
    if(!enabled || side==0 || !companion)return false;
    return leader->unk_a8>0 || companion->unk_a8>0;
}
int BossHealth(const Enemy *enemy,int value) {
    return enabled && enemy->manager->sideIdx==0 && (enemy->isBoss || enemy->unknownType==3) && value>0?value*2:value;
}
void CountDamage(Player *player,int amount) {
    if(enabled && player->sideIdx==0 && amount>0)damage[player==companion?1:0]+=unsigned(amount);
}
static WORD ReadControls(int player) {
    if(autoPlay) {
        WORD mask=1;float target=sinf(float(tick)*.015f+player*2.1f)*95;
        Player *p=player==1?companion:g_GameManager.sides[0].player;
        if(p) {if(p->pos.x<target-3)mask|=0x80;if(p->pos.x>target+3)mask|=0x40;}
        if(tick%180==179)mask&=~1;
        if(p && p->unk_30388>=200 && tick%180==0)mask|=2;
        if(g_GameManager.gui->IsMsgRunning())mask=WORD(0x100|(tick%30==0?1:0));
        return mask;
    }
    if(networked)return WORD(g_PlayerInputs[2].unk_0&0x51ff);
    if(!g_GameWindow.isAppActive)return 0;
    const int keys[2][8]={{VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,'Z','X',VK_SHIFT,VK_ESCAPE},
        {'W','S','A','D','F','G','H',VK_ESCAPE}};
    const WORD bits[8]={0x10,0x20,0x40,0x80,1,2,4,8};WORD mask=0;
    for(int i=0;i<8;++i)if(GetAsyncKeyState(keys[player][i])&0x8000)mask|=bits[i];
    return mask;
}
bool WaitForInputs() {
    if(!enabled)return false;
    Prepare();
    if(!networked) {masks[0]=ReadControls(0);masks[1]=ReadControls(1);return false;}
    if(thnet::g_NetManager.GetState()!=thnet::NetManager::Matched) {Stop();g_App.scene=1;return true;}
    unsigned local=thnet::g_NetManager.Match().role;thnet::PacketGameEvent event;
    while(thnet::g_NetManager.PopEvent(event)) {
        if(event.type!=thnet::TeamState || event.rngSeed!=StorySignature) {Stop();g_App.scene=1;return true;}
        if(event.tick<tick || event.tick-tick>=256 || event.params[0]<0 || event.params[0]>65535)continue;
        InputSlot &slot=inputs[1-local][event.tick%256];slot.used=true;slot.tick=event.tick;slot.mask=WORD(event.params[0]);
    }
    if(!sentInput) {
        unsigned target=tick+Delay;InputSlot &slot=inputs[local][target%256];
        slot.used=true;slot.tick=target;slot.mask=ReadControls(local);
        memset(&event,0,sizeof(event));event.type=thnet::TeamState;event.tick=target;
        event.rngSeed=StorySignature;event.params[0]=float(slot.mask);
        if(!thnet::g_NetManager.SendEvent(event))return true;
        sentInput=true;
    }
    for(int p=0;p<2;++p) {
        InputSlot &slot=inputs[p][tick%256];
        if(!slot.used || slot.tick!=tick) {Text(160,450,0xffffff80,"Waiting for partner input...");return true;}
        masks[p]=slot.mask;
    }
    return false;
}
void Update() {
    if(!enabled)return;
    Prepare();for(int p=0;p<2;++p)SetInput(controls[p],masks[p]);
    if((controls[0].unk_6|controls[1].unk_6)&8)paused=!paused;
    g_PlayerInputs[0]=controls[0];
    SetInput(dialogueInput,WORD(masks[0]|masks[1]));g_PlayerInputs[2]=dialogueInput;g_PlayerInputs[2].unk_6&=~8;
    if(paused)g_GameManager.stateFlags|=0x800;else g_GameManager.stateFlags&=~0x800;
    if(paused && ((controls[0].unk_6|controls[1].unk_6)&2)) {Stop();g_App.scene=1;return;}
    if(companion && !paused) {
        Player *players[2]={g_GameManager.sides[0].player,companion};
        for(int p=0;p<2;++p)for(int b=0;b<128;++b)if(players[p]->bullets[b].state==1)++shots[p];
    }
    if(transitionTest && !paused && !g_GameManager.gui->IsMsgRunning() && !g_GameManager.gui->unk_1095c && ++stagePlayTicks==120) {
        Player *cpu=g_GameManager.sides[1].player;
        cpu->unk_a8=1;cpu->unk_30388=0;cpu->Die();
    }
    if(trace && tick%60==0 && companion) {
        Player *leader=g_GameManager.sides[0].player,*cpu=g_GameManager.sides[1].player;
        fprintf(trace,"{\"tick\":%u,\"stage\":%d,\"opponent\":%d,\"rng\":%u,\"health\":[%d,%d,%d],\"shots\":[%u,%u],\"damage\":[%u,%u],\"players\":[[%.2f,%.2f],[%.2f,%.2f]],\"score\":%u}\n",
            tick,g_GameManager.unk_fc,g_GameManager.sides[1].character,unsigned(g_Rng.GetSeed()),leader->unk_a8,companion->unk_a8,cpu->unk_a8,
            shots[0],shots[1],damage[0],damage[1],leader->pos.x,leader->pos.y,companion->pos.x,companion->pos.y,g_GameManager.sides[0].stats->score);fflush(trace);
    }
    ++tick;sentInput=false;
    if(smokeTicks && tick>=smokeTicks) {
        FILE *file=fopen("coop-smoke.json","wb");
        if(file) {fprintf(file,"{\"story\":true,\"tick\":%u,\"stage\":%d,\"shots\":[%u,%u],\"damage\":[%u,%u]}\n",tick,g_GameManager.unk_fc,shots[0],shots[1],damage[0],damage[1]);fclose(file);}
        g_GameWindow.isClosing=1;
    }
    Draw();
}
void Draw() {
    Text(30,2,0xff80ffff,"STORY CO-OP");
    if(companion) {
        ui::Format(30,68,0xffffffff,"P1 HP %d   P2 HP %d",leader->unk_a8,companion->unk_a8);
        ui::Format(350,68,0xffffffff,"BOSS HP %d / 20",g_GameManager.sides[1].player->unk_a8);
    }
    if(paused) {Text(180,190,0xffffff80,"CO-OP PAUSED");Text(110,230,0xffffffff,"Esc: resume | X: return to title");}
}
}
#endif
