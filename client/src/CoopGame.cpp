#ifdef TH09_NATIVE_NETPLAY
#include "CoopGame.h"
#include "CoopRules.h"
#include "NetGame.h"
#include "UiText.h"
#include "mainmenu.h"
#include "ascii.h"
#include "gamemanager.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "gui.h"
#include "spellcard.h"
#include <math.h>

namespace coop {
static Rules rules;
static bool enabled=false, ready=false, spawning=false, saved=false, networked=false;
static bool autoPlay=false, autoStarted=false, paused=false, peerLost=false, clockStarted=false;
static int selected[2]={0,1}, difficulty=1, bossCharacter=0, cursor=0, scriptDepth=0;
static Enemy *bosses[2]={0,0};
static WORD previousInput[2]={0,0};
static unsigned supportUses=0, finishedFrames=0, hitSerial=0, acceptedHit=0;
static unsigned remoteScore=0, authoritativeScore=0;
static unsigned smokeTicks=0;
static FILE *trace=0;
static bool Host() { return !networked || thnet::g_NetManager.Match().role==0; }
static int Local() { return networked ? thnet::g_NetManager.Match().role:0; }
bool Active() { return enabled; }
bool Networked() { return enabled && networked; }
bool Spawning() { return spawning || scriptDepth>0; }
unsigned Tick() { return rules.tick; }
NativeScope::NativeScope(): engaged_(enabled) { if(engaged_) ++scriptDepth; }
NativeScope::~NativeScope() { if(engaged_) --scriptDepth; }
bool BossProjection(const Enemy *enemy) {
    return enabled && enemy && enemy->active && enemy->unknownType==3 &&
        (enemy==bosses[0] || enemy==bosses[1]);
}
static void ResetRules() {
    rules.Reset(); rules.phase=Rules::Boss; rules.wave=0;
    rules.bossHealth=rules.bossMaxHealth=1400;
}
static unsigned TeamScore() {
    if(networked && !Host()) return authoritativeScore;
    unsigned value=rules.teamBonus;
    for(int side=0;side<2;++side) {
        if(networked && side==1) value+=remoteScore;
        else if(g_GameManager.sides[side].stats) value+=g_GameManager.sides[side].stats->score*10;
    }
    return value;
}
void Stop() {
    enabled=ready=spawning=false; bosses[0]=bosses[1]=0; g_GameManager.stateFlags&=~0x800;
    if(trace) { fclose(trace); trace=0; }
    if(networked) thnet::g_NetManager.Close();
    networked=false;
}
void OpenSetup(MainMenu *menu) {
    Stop(); thnet::EndSession(); cursor=0; menu->SetState(18); menu->helpSprite=0;
}
static void Text(float x,float y,DWORD color,const char *fmt,int value=0) {
    D3DXVECTOR3 pos(x,y,0);
    g_AsciiText.color=color; g_AsciiText.SetScale(1,1); g_AsciiText.inArena=0;
    ui::Format(x,y,color,fmt,value);
}
static int StartGame(bool online) {
    if(!online) thnet::EndSession();
    networked=online; ResetRules(); enabled=true; ready=false; saved=false; paused=false; peerLost=false; clockStarted=false;
    thnet::MarkGame(online);
    finishedFrames=supportUses=hitSerial=acceptedHit=remoteScore=authoritativeScore=0;
    previousInput[0]=previousInput[1]=0;
    char value[32];
    autoPlay=GetEnvironmentVariableA("TH09_COOP_AUTOTEST",value,sizeof(value))!=0 ||
        GetEnvironmentVariableA("TH09_NET_AUTOTEST",value,sizeof(value))!=0;
    smokeTicks=0;
    if(autoPlay && GetEnvironmentVariableA("TH09_SMOKE_TICKS",value,sizeof(value)))smokeTicks=unsigned(atoi(value));
    if(autoPlay || GetEnvironmentVariableA("TH09_COOP_TRACE",value,sizeof(value))) trace=fopen("coop-trace.jsonl","wb");
    g_GameManager.stateFlags=0; g_GameManager.mode=2; g_GameManager.unk_114=0;
    g_GameManager.unk_fc=9; g_GameManager.unk_100=0; g_GameManager.difficulty=difficulty;
    for(int side=0;side<2;++side) {
        g_GameManager.sides[side].character=selected[side];
        g_GameManager.sides[side].unk_28=0; g_GameManager.sides[side].unk_30=0;
        g_GameManager.initialLives[side]=10;
    }
    unsigned seed=online ? thnet::g_NetManager.Match().seed:0x909;
    g_Rng.SetSeed(WORD(seed^(seed>>16))); g_Rng.ResetGenerated();
    g_App.StopMusic(); g_App.scene=2; return 0;
}
void StartNetwork() {
    const thnet::MatchData &match=thnet::g_NetManager.Match();
    selected[0]=match.character0; selected[1]=match.character1;
    difficulty=match.difficulty; bossCharacter=match.boss;
    StartGame(true);
}
int AutoStart(MainMenu *menu) {
    char value[16];
    if(autoStarted || !GetEnvironmentVariableA("TH09_COOP_AUTOTEST",value,sizeof(value))) return -1;
    autoStarted=true;
    if(GetEnvironmentVariableA("TH09_TEST_BOSS",value,sizeof(value))) bossCharacter=atoi(value)%16;
    if(GetEnvironmentVariableA("TH09_TEST_DIFFICULTY",value,sizeof(value))) difficulty=atoi(value)%4;
    if(GetEnvironmentVariableA("TH09_TEST_CHARACTER",value,sizeof(value)))selected[0]=atoi(value)%16;
    if(GetEnvironmentVariableA("TH09_TEST_CHARACTER2",value,sizeof(value)))selected[1]=atoi(value)%16;
    return StartGame(false);
}
int UpdateSetup(MainMenu *menu) {
    if(g_PlayerInputs[2].unk_6 & 10) { menu->SetState(1); return 1; }
    int old=menu->cursor; menu->cursor=cursor; menu->MoveMenuCursor(5); cursor=menu->cursor; menu->cursor=old;
    int delta=(g_PlayerInputs[2].IsPressed(0x80)?1:0)-(g_PlayerInputs[2].IsPressed(0x40)?1:0);
    if(cursor<2) selected[cursor]=(selected[cursor]+delta+16)%16;
    else if(cursor==2) bossCharacter=(bossCharacter+delta+16)%16;
    else if(cursor==3) difficulty=(difficulty+delta+4)%4;
    if(!(g_PlayerInputs[2].unk_6 & 0x1001)) return 1;
    if(cursor<2) { selected[cursor]=(selected[cursor]+1)%16; return 1; }
    if(cursor==2) { bossCharacter=(bossCharacter+1)%16; return 1; }
    if(cursor==3) { difficulty=(difficulty+1)%4; return 1; }
    return StartGame(false);
}
void DrawSetup() {
    DWORD color=g_AsciiText.color; float sx=g_AsciiText.scaleX,sy=g_AsciiText.scaleY; int arena=g_AsciiText.inArena;
    ui::Title(120,65,0xffffff80,"TWO FRIENDS VS BOSS");
    for(int i=0;i<5;++i) {
        D3DXVECTOR3 pos(125,125.0f+i*37,0);
        g_AsciiText.color=i==cursor?0xffffff80:0xffc0e0ff;
        if(i<2) ui::Format(pos.x,pos.y,g_AsciiText.color,"Player %d: %s",i+1,g_ReplayMenuCharacterNames[selected[i]]);
        else if(i==2) ui::Format(pos.x,pos.y,g_AsciiText.color,"Boss: %s",g_ReplayMenuCharacterNames[bossCharacter]);
        else if(i==3) ui::Format(pos.x,pos.y,g_AsciiText.color,"Difficulty: %s",g_ReplayMenuDifficultyNames[difficulty]);
        else ui::Format(pos.x,pos.y,g_AsciiText.color,"START LOCAL CO-OP");
    }
    Text(70,335,0xffffffff,"One boss health bar | 3 lives each");
    Text(70,365,0xffb0d0ff,"P1: Arrows / Z shoot / X support / Shift focus");
    Text(70,392,0xffb0d0ff,"P2: WASD / F shoot / G support / H focus");
    Text(70,420,0xffb0d0ff,"Focus + support: shield | Both: team attack");
    g_AsciiText.color=color; g_AsciiText.SetScale(sx,sy); g_AsciiText.inArena=arena;
}
static void ClearBullets(int side,float radius) {
    BulletManager *mgr=g_GameManager.sides[side].bulletManager;
    Player *player=g_GameManager.sides[side].player;
    if(!mgr || !player) return;
    Bullet *sets[]={mgr->bullets,mgr->bullets2};
    const unsigned counts[]={sizeof(mgr->bullets)/sizeof(Bullet),sizeof(mgr->bullets2)/sizeof(Bullet)};
    for(int set=0;set<2;++set) for(unsigned i=0;i<counts[set];++i) {
        Bullet &b=sets[set][i];
        float x=b.pos.x-player->pos.x,y=b.pos.y-player->pos.y;
        if(b.state && b.state!=6 && x*x+y*y<=radius*radius) b.state=5;
    }
}
static void ClearField() {
    bosses[0]=bosses[1]=0;
    for(int side=0;side<2;++side) {
        EnemyManager *mgr=g_GameManager.sides[side].enemyManager;
        if(!mgr) continue;
        SpellCard *card=g_GameManager.sides[side].spellCard;
        if(card && card->callbacks[1].end) {
            card->callbacks[1].end(card);
            memset(&card->callbacks[1],0,sizeof(card->callbacks[1]));
        }
        for(int i=0;i<128;++i) if(mgr->enemies[i].active) { mgr->enemies[i].Despawn(); mgr->enemies[i].active=0; }
        ClearBullets(side,1000);
    }
}
static void Respawn(int side) {
    Player *p=g_GameManager.sides[side].player;
    p->unk_a8=10; p->SetPlayerState(1); p->stateTimer=0; p->unk_1b74=180;
    p->pos=D3DXVECTOR3(0,384,0.49f); ClearBullets(side,160);
}
static bool LoadBoss() {


    g_SpriteEngine->ReleaseAnm(18);
    AnmLoaded *anm=g_SpriteEngine->LoadAnm(18,g_CharacterFiles[bossCharacter][1]);
    if(!anm) return false;
    for(int side=0;side<2;++side) {
        EnemyManager *mgr=g_GameManager.sides[side].enemyManager;
        mgr->playerEcl.Unload();
        if(mgr->playerEcl.Load(g_CharacterFiles[bossCharacter][2])) return false;
        mgr->otherPlayerAnm=anm;
    }
    return true;
}
static void SpawnBossProjection(int side) {
    SpellCard *card=g_GameManager.sides[side].spellCard;
    if(card->callbacks[1].end) card->callbacks[1].end(card);
    memset(&card->callbacks[1],0,sizeof(card->callbacks[1]));
    spawning=true;
    g_SpellCardTypes[2].init(card);
    spawning=false;
    card->callbacks[1]=g_SpellCardTypes[2].callbacks;
    card->slots[1].type=2; card->slots[1].kind=2;
    card->unk_a8=1;
    strncpy(card->name,g_ReplayMenuCharacterNames[bossCharacter],sizeof(card->name)-1);
    bosses[side]=g_GameManager.sides[side].enemyManager->FindByUnknownType(3);
    if(bosses[side]) {
        bosses[side]->life=rules.bossHealth>0 ? rules.bossHealth:1;
        bosses[side]->maxLife=rules.bossMaxHealth;
    }
}
static void Emit(thnet::EventType type,float a=0,float b=0,float c=0,float d=0,unsigned seed=0) {
    thnet::PacketGameEvent event; memset(&event,0,sizeof(event));
    event.tick=rules.tick; event.type=thnet::U8(type); event.rngSeed=seed;
    event.params[0]=a;event.params[1]=b;event.params[2]=c;event.params[3]=d;
    thnet::g_NetManager.SendEvent(event);
}
static void State() {
    if(!networked || !Host()) return;
    unsigned packed=unsigned(rules.lives[0]) | (unsigned(rules.lives[1])<<4) |
        (unsigned(rules.help)<<8) | (unsigned(rules.phase)<<12);
    Emit(thnet::TeamState,float(packed),float(rules.bossHealth),float(rules.phaseTick),float(TeamScore()),acceptedHit);
}
static void Effects(int side,int level) {
    ClearBullets(side,level==3 ? 1000.0f:140.0f);
    if(rules.Alive(1-side)) g_GameManager.sides[1-side].player->unk_1b74=level>=2 ? 180:90;
    if(level==3) ClearBullets(1-side,1000);
}
bool Support(int side,int level) {
    if(!enabled) return false;
    if(networked && !Host()) {
        if(!rules.supportCooldown[side]) {
            Emit(thnet::SupportRequest,float(level)); rules.supportCooldown[side]=180;
        }
        return true;
    }
    if(rules.Support(side,level)) {
        ++supportUses; if(level==3) rules.supportCooldown[1-side]=180;
        Effects(side,level);
        if(networked) { Emit(thnet::SupportCommit,float(side),float(level)); State(); }
    }
    return true;
}
static void Receive() {
    thnet::PacketGameEvent event;
    while(thnet::g_NetManager.PopEvent(event)) {
        if(thnet::HandlePause(event)) continue;
        if(Host()) {
            if(event.type==thnet::PlayerHit && event.rngSeed>acceptedHit) {
                acceptedHit=event.rngSeed; rules.Hit(1); State();
            } else if(event.type==thnet::BossDamage && event.params[0]>0 && event.params[0]<=10000) {
                rules.DamageBoss(int(event.params[0]));
            } else if(event.type==thnet::SupportRequest && event.params[0]>=1 && event.params[0]<=3) {
                Support(1,int(event.params[0]));
            } else if(event.type==thnet::ScoreReport && event.params[0]>=0 && event.params[0]<10000000) {
                remoteScore=unsigned(event.params[0])*10;
            }
        } else {
            if(event.type==thnet::TeamState) {
                unsigned packed=unsigned(event.params[0]);
                int a=packed&15,b=(packed>>4)&15,help=(packed>>8)&15,phase=(packed>>12)&15;
                if(a>3 || b>3 || help>5 || phase>Rules::Defeat || event.params[1]<0 || event.params[1]>rules.bossMaxHealth) continue;
                rules.lives[0]=a;
                if(event.rngSeed>=hitSerial) rules.lives[1]=b;
                rules.help=help; rules.phase=Rules::Phase(phase); rules.bossHealth=int(event.params[1]);
                if(!rules.Playing()) rules.tick=event.tick;
                authoritativeScore=unsigned(event.params[3]);
            } else if(event.type==thnet::SupportCommit && event.params[0]>=0 && event.params[0]<=1 &&
                event.params[1]>=1 && event.params[1]<=3) {
                Effects(int(event.params[0]),int(event.params[1])); ++supportUses;
            } else if(event.type==thnet::PlayerHit && event.params[0]>=0 && event.params[0]<=3) {
                Player *remote=g_GameManager.sides[0].player;
                remote->SetPlayerState(4); remote->stateTimer=0;
            }
        }
    }
}
bool DamageBoss(Enemy *enemy,int damage) {
    if(!BossProjection(enemy)) return false;
    if(!networked || enemy->manager->sideIdx==Local()) {
        if(Host()) rules.DamageBoss(damage);
        else if(damage>0) Emit(thnet::BossDamage,float(damage));
    }
    for(int side=0;side<2;++side) if(BossProjection(bosses[side])) bosses[side]->life=rules.bossHealth>0 ? rules.bossHealth:1;
    return true;
}
bool AllowDeath(Player *player) {
    if(!enabled) return true;
    if(!rules.Playing() || !rules.Alive(player->sideIdx) || (networked && player->sideIdx!=Local())) return false;
    player->unk_a8=10; return true;
}
void AfterDeath(Player *player,int previousState) {
    if(!enabled || !rules.Playing() || previousState==4 || player->playerState!=4) return;
    if(Host()) {
        rules.Hit(player->sideIdx);
        if(networked) { Emit(thnet::PlayerHit,float(rules.lives[player->sideIdx])); State(); }
    } else {
        if(rules.lives[1]>0) --rules.lives[1];
        Emit(thnet::PlayerHit,float(rules.lives[1]),0,0,0,++hitSerial);
    }
    if(!rules.Alive(player->sideIdx)) { player->SetPlayerState(5); player->unk_a8=0; }
}
static WORD Bot(int side) {
    Player *player=g_GameManager.sides[side].player;
    float target=BossProjection(bosses[side]) ? bosses[side]->pos.x:0;
    float best=1e30f; WORD result=1;
    const WORD moves[]={0,0x10,0x20,0x40,0x80,0x50,0x90,0x60,0xa0};
    BulletManager *mgr=g_GameManager.sides[side].bulletManager;
    for(int move=0;move<9;++move) {
        float dx=0,dy=0;
        if(move) player->GetMoveDelta(move,0,&dx,&dy);
        float x=player->pos.x+dx*12,y=player->pos.y+dy*12;
        if(x<-128 || x>128 || y<48 || y>424) continue;
        float cost=((x-target)*(x-target)+(y-384)*(y-384))*.003f;
        const Bullet *sets[]={mgr->bullets,mgr->bullets2};
        const unsigned counts[]={sizeof(mgr->bullets)/sizeof(Bullet),sizeof(mgr->bullets2)/sizeof(Bullet)};
        for(int set=0;set<2;++set) for(unsigned b=0;b<counts[set];++b) {
            const Bullet &bullet=sets[set][b];
            if(!bullet.state || bullet.state==5 || bullet.state==6 || bullet.harmless) continue;
            float rx=bullet.pos.x-player->pos.x,ry=bullet.pos.y-player->pos.y;
            float vx=bullet.velocity.x-dx,vy=bullet.velocity.y-dy,denom=vx*vx+vy*vy;
            float t=denom>.001f ? -(rx*vx+ry*vy)/denom:0;
            if(t<0)t=0;if(t>35)t=35;
            float nx=rx+vx*t,ny=ry+vy*t,d2=nx*nx+ny*ny;
            if(d2<3600) cost+=20000/(d2+16);
        }
        if(cost<best) { best=cost; result=WORD(1|moves[move]); }
    }
    if(rules.tick==200) result|=2;
    if(!rules.Alive(1-side) && rules.help>0 && !rules.supportCooldown[side] && rules.tick%30==0) result|=2;
    return result;
}
static void Controls() {
    const int keys[2][7]={{VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,'Z','X',VK_SHIFT},{'W','S','A','D','F','G','H'}};
    const WORD bits[]={0x10,0x20,0x40,0x80,1,2,4};
    const WORD localInput=g_PlayerInputs[2].unk_0;
    for(int side=0;side<2;++side) {
        if(networked && side!=Local()) {
            float x,y; WORD input=previousInput[side];
            thnet::g_NetManager.RemotePosition(rules.tick,x,y,input);
            PlayerInput &remote=g_PlayerInputs[side];
            remote.unk_0=remote.unk_2c=input; remote.unk_6=remote.unk_32=WORD(input&~previousInput[side]);
            remote.unk_4=remote.unk_30=0; previousInput[side]=input;
            continue;
        }
        WORD mask=networked ? localInput:0;
        if(!networked && g_GameWindow.isAppActive) for(int i=0;i<7;++i) if(GetAsyncKeyState(keys[side][i])&0x8000) mask|=bits[i];
        if(autoPlay) mask=Bot(side);
        PlayerInput &input=g_PlayerInputs[side];
        input.unk_0=input.unk_2c=mask; input.unk_6=input.unk_32=WORD(mask&~previousInput[side]);
        input.unk_4=input.unk_30=0; previousInput[side]=mask;
    }
    if(paused || !rules.Playing()) return;
    bool first=(g_PlayerInputs[0].unk_6&2)!=0,second=(g_PlayerInputs[1].unk_6&2)!=0;
    if(!networked && first && second) Support(0,3);
    else for(int side=0;side<2;++side) if((!networked || side==Local()) && (g_PlayerInputs[side].unk_6&2))
        Support(side,(g_PlayerInputs[side].unk_0&4)?2:1);
}
static void RestartRun() {
    paused=false;saved=false;supportUses=finishedFrames=0;
    g_GameManager.stateFlags&=~0x800;ResetRules();ClearField();
    g_GameManager.rank.unk_b0=g_GameManager.rank.unk_d0=0;
    for(int side=0;side<2;++side) {
        g_GameManager.sides[side].player->Reset();Respawn(side);previousInput[side]=0;
        GameStats *stats=g_GameManager.sides[side].stats;stats->score=stats->guiScore=stats->scoreIncrement=0;
        SpawnBossProjection(side);
    }
}
void Prepare() {
    if(!enabled || ready) return;
    ready=true;ClearField();
    for(int side=0;side<2;++side) Respawn(side);
    g_GameManager.gui->unk_1095c=0;
    if(!LoadBoss()) { peerLost=true;rules.phase=Rules::Defeat; }
    else for(int side=0;side<2;++side) SpawnBossProjection(side);
}
void Update() {
    if(!enabled) return;
    if(g_App.scene!=2) { Stop();return; }
    if(g_GameManager.loadingThreadState || !g_GameManager.sides[0].player || !g_GameManager.sides[1].player) return;
    Prepare();
    if(networked && !clockStarted) { rules.tick=thnet::g_NetManager.StartTick();clockStarted=true; }
    if(networked) {
        Receive();paused=thnet::NetworkPaused();
        if(thnet::g_NetManager.GetState()!=thnet::NetManager::Matched && rules.Playing()) {
            peerLost=true;rules.phase=Rules::Defeat;
        }
    }
    Controls();
    if(!networked && rules.Playing()) {
        if((g_PlayerInputs[2].unk_6&8) || (!autoPlay && !g_GameWindow.isAppActive && !paused)) {
            paused=!paused;g_PlayerInputs[2].unk_6&=~8;
            if(paused) g_GameManager.stateFlags|=0x800;else g_GameManager.stateFlags&=~0x800;
        }
        if(paused) {
            Draw();
            if(g_PlayerInputs[2].unk_6&2) { Stop();g_App.scene=1; }
            else if(g_PlayerInputs[2].unk_6&0x4000) RestartRun();
            return;
        }
    }
    if(!rules.Playing()) {
        g_GameManager.stateFlags|=0x800;
        if(networked && Host() && !peerLost && !saved) State();
        if(!saved) {
            saved=true;FILE *file=fopen("coop-results.jsonl","ab");
            if(file) {
                fprintf(file,"{\"victory\":%s,\"ticks\":%u,\"score\":%u,\"supports\":%u,\"boss\":%d,\"online\":%s,\"characters\":[%d,%d]}\n",
                    rules.phase==Rules::Victory?"true":"false",rules.tick,TeamScore(),supportUses,bossCharacter,networked?"true":"false",selected[0],selected[1]);
                fclose(file);
            }
        }
        Draw();
        if(autoPlay) { if(++finishedFrames>=180) { g_GameWindow.isClosing=1;Stop(); }return; }
        if(g_PlayerInputs[2].unk_6&10) { Stop();g_App.scene=1; }
        else if(g_PlayerInputs[2].unk_6&0x1001) {
            if(networked) { Stop();g_App.scene=1; }else RestartRun();
        }
        return;
    }
    if(paused) {
        Draw();
        if(networked && (g_PlayerInputs[2].unk_6&2)) {Stop();g_App.scene=1;}
        return;
    }
    g_GameManager.rank.unk_cc=0;
    if(Host()) rules.Step();
    else {
        ++rules.tick;++rules.phaseTick;
        for(int side=0;side<2;++side) if(rules.supportCooldown[side]) --rules.supportCooldown[side];
    }
    for(int side=0;side<2;++side) {
        Player *player=g_GameManager.sides[side].player;
        if(rules.Alive(side) && player->playerState==5) Respawn(side);
        if(!rules.Alive(side)) { player->playerState=5;player->unk_a8=0; }
        if(!BossProjection(bosses[side])) SpawnBossProjection(side);
    }
    if(networked && rules.tick%15==0) {
        if(Host()) State();
        else Emit(thnet::ScoreReport,float(g_GameManager.sides[1].stats->score));
    }
    if(trace && rules.tick%60==0) {
        fprintf(trace,"{\"tick\":%u,\"phase\":%d,\"lives\":[%d,%d],\"help\":%d,\"bossHealth\":%d,\"boss\":%d,\"bosses\":[%d,%d],\"players\":[[%.2f,%.2f],[%.2f,%.2f]]}\n",
            rules.tick,int(rules.phase),rules.lives[0],rules.lives[1],rules.help,rules.bossHealth,bossCharacter,
            BossProjection(bosses[0]),BossProjection(bosses[1]),
            g_GameManager.sides[0].player->pos.x,g_GameManager.sides[0].player->pos.y,
            g_GameManager.sides[1].player->pos.x,g_GameManager.sides[1].player->pos.y);fflush(trace);
    }
    if(smokeTicks && rules.tick>=smokeTicks) {
        FILE *file=fopen("coop-smoke.json","wb");
        if(file) {
            fprintf(file,"{\"boss\":%d,\"ticks\":%u,\"bosses\":[%d,%d],\"health\":%d,\"characters\":[%d,%d]}\n",
                bossCharacter,rules.tick,BossProjection(bosses[0]),BossProjection(bosses[1]),rules.bossHealth,selected[0],selected[1]);
            fclose(file);
        }
        g_GameWindow.isClosing=1;
    }
    Draw();
}
void Draw() {
    DWORD color=g_AsciiText.color;float sx=g_AsciiText.scaleX,sy=g_AsciiText.scaleY;int arena=g_AsciiText.inArena;
    Text(15,16,0xff80ffff,"P1 LIVES %d",rules.lives[0]);Text(475,16,0xffffff80,"P2 LIVES %d",rules.lives[1]);
    Text(235,16,0xffa0ffa0,"TEAM HELP %d",rules.help);
    D3DXVECTOR3 pos(220,40,0);g_AsciiText.color=0xffffffff;
    ui::Format(pos.x,pos.y,g_AsciiText.color,"BOSS: %s HP %d",g_ReplayMenuCharacterNames[bossCharacter],rules.bossHealth);
    Text(215,62,0xffffffff,"TEAM SCORE: %u",TeamScore());
    if(paused) {
        Text(255,190,0xffffff80,"PAUSED");
        Text(105,230,0xffffffff,networked?"Esc: toggle your pause | X: leave game":"Esc: resume | R: retry | X: title");
    }
    else if(!rules.Playing()) {
        Text(205,190,0xffffff80,peerLost?"CONNECTION / RESOURCE ERROR":rules.phase==Rules::Victory?"TEAM VICTORY":"TEAM DEFEAT");
        Text(180,225,0xffffffff,"Team score: %u",TeamScore());
        Text(160,270,0xffffffff,networked?"Z/Enter: title | X/Esc: title":"Z/Enter: retry | X/Esc: title");
    }
    g_AsciiText.color=color;g_AsciiText.SetScale(sx,sy);g_AsciiText.inArena=arena;
}
}
#endif
