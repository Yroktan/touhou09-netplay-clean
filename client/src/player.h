

#pragma once

#include "chain.h"
#include "gamemanager.h"
#include "sprite.h"
#include "timer.h"

struct Effect;



struct PlayerHitArea
{

    void Reset();

    void Deactivate()
    {
        this->active = 0;
    }

    float x;
    float y;

    float radius;
    float radiusGrowth;

    float width;
    float height;

    float widthGrowth;
    float heightGrowth;

    float angle;

    int framesLeft;


    int damage;
    int hits;
    int maxDamage;

    int damageInterval;


    int kind;
    BYTE active;
    unsigned char pad_3d[0x40 - 0x3d];
    int unk_40;
};
ASSERT_SIZE(PlayerHitArea, 0x44);

class Player;



struct PlayerCpuLevel
{


    int seconds5c[3];
    int seconds68[3];

    int rank;
};
ASSERT_SIZE(PlayerCpuLevel, 0x1c);



struct PlayerUnknown24
{


    void Reset();





    void Run();

    Timer timer0;

    int unk_c;


    int history[8];
    int history2[8];

    int counter;
    int lastDirection;
    float unk_58;
    Timer timer5c;
    Timer timer68;
    DWORD flags;
    Player *player;
};
ASSERT_SIZE(PlayerUnknown24, 0x7c);



struct PlayerUnknown36cEntry
{
    D3DXVECTOR3 pos;

    D3DXVECTOR3 unk_c;
    D3DXVECTOR3 size;

    float radius;
    float angle;
    void *unk_2c;
};
ASSERT_SIZE(PlayerUnknown36cEntry, 0x30);



struct PlayerUnknown36c
{



    PlayerUnknown36cEntry *CheckHits();



    PlayerUnknown36cEntry *CheckHitsAt(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, float margin);

    void Add(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, void *unk_2c);


    void AddCircle(D3DXVECTOR3 *pos, float radius, void *unk_2c)
    {
        if (this->count < 0x80)
        {
            this->entries[this->count].pos = *pos;
            this->entries[this->count].radius = radius;
            this->entries[this->count].unk_2c = unk_2c;
            this->count++;
        }
    }


    void AddTurned(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 *origin, float angle, void *unk_2c);

    Player *player;
    PlayerUnknown36cEntry entries[0x80];

    int count;
};
ASSERT_SIZE(PlayerUnknown36c, 0x1808);

class Player;
struct PlayerOption;

typedef int (*PlayerOptionFunc)(Player *player, PlayerOption *option);



struct PlayerOption
{
    Sprite sprite;
    D3DXVECTOR3 pos;
    unsigned char pad_2b0[0x2e0 - 0x2b0];
    Timer timer;

    PlayerOptionFunc update;
    unsigned char pad_2f0[0x2f4 - 0x2f0];
};
ASSERT_SIZE(PlayerOption, 0x2f4);

struct Bullet;
struct Enemy;
struct PlayerBullet;
struct ShtFile;


typedef int (*PlayerFunc)(Player *player);

typedef int (*PlayerPosFunc)(Player *player, D3DXVECTOR3 *pos);

typedef int (*PlayerEnemyFunc)(Player *player, struct Enemy *enemy);

typedef int (*PlayerComboFunc)(struct PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
struct ShotData;


typedef int (*ShotFireFunc)(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
typedef int (*ShotUpdateFunc)(Player *player, PlayerBullet *bullet);
typedef int (*ShotHitFunc)(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos);



struct PlayerBullet
{
    Sprite sprite;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 trail[0x20];
    D3DXVECTOR3 size;

    D3DVECTOR velocity;


    float unk_448;
    float speed;

    float angle;
    Timer timer;
    short damage;

    short state;

    short type;
    unsigned char pad_466[0x468 - 0x466];


    short unk_468;
    short unk_46a;

    short script;


    BYTE unk_46e;
    unsigned char pad_46f[0x470 - 0x46f];

    short unk_470;

    BYTE unk_472;
    unsigned char pad_473[0x474 - 0x473];

    ShotUpdateFunc update;
    ShotUpdateFunc draw;
    ShotHitFunc hit;

    ShotData *shot;
};
ASSERT_SIZE(PlayerBullet, 0x484);



struct PlayerLaser
{
    Timer timer;

    PlayerBullet *bullet;

    int unknown10;
    unsigned char pad_14[0x18 - 0x14];

    int unk_18;
};
ASSERT_SIZE(PlayerLaser, 0x1c);



struct PlayerUnknown30410
{






    void AddCombo(D3DXVECTOR3 *pos, int a, int b, int c, int d);

    void AddComboCounted(D3DXVECTOR3 *pos, int a, int b, int c, int d)
    {
        this->unk_38++;
        this->AddCombo(pos, a, b, c, d);
    }

    void ResetCombo()
    {
        this->timer20 = 0;
        this->count = 0;
        this->unk_38 = 0;
    }



    void Update();


    void EndCombo();

    Player *player;

    int count;
    int maxCount;
    int unk_c;

    int bonus;
    Timer timer14;
    Timer timer20;



    int unk_2c;



    int unk_30;

    int unk_34;
    int unk_38;
    int unk_3c;


    PlayerComboFunc func;
};
ASSERT_SIZE(PlayerUnknown30410, 0x44);



struct PlayerUnknown30454
{

    int script;
    D3DXVECTOR3 pos;

    D3DXVECTOR3 velocity;

    int unk_1c;
    Sprite sprite;
};
ASSERT_SIZE(PlayerUnknown30454, 0x2c4);


class Player
{
public:



    Player();





    static Player *RegisterChain(int arg, int sideIdx, int unk_20);

    static int OnUpdate(Player *player);
    static int OnDraw(Player *player);
    static int OnDraw2(Player *player);
    static int AddedCallback(Player *player);




    static int LoadAnms();


    static void RemoveChain(Player *player);





    void Reset();







    void Die();


    int CallUnknown30404(D3DXVECTOR3 *pos)
    {
        return this->unk_30404(this, pos);
    }


    float AngleFromPlayer(D3DXVECTOR3 *pos);


    float AngleToPlayer(D3DXVECTOR3 *pos);



    void AddUnknown30388(float amount);


    int CheckHitBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);


    int CheckHitCircle(D3DXVECTOR3 *pos, float radius);




    int CheckLaserHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 *origin, float angle, int canGraze);


    int TouchesGrazeBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);

    void SetPlayerState(int value)
    {
        this->playerState = value;
    }




    void StartClearCircle1();
    void StartClearCircle2();
    void StartClearCircle3();


    int GetPlayerState()
    {
        return this->playerState;
    }

    void SetRegisterChainArg(BYTE arg);



    PlayerHitArea *AllocHitArea(D3DXVECTOR3 *pos, int framesLeft, int unk_40);



    PlayerHitArea *AddHitBox(D3DXVECTOR3 *pos, float width, float height, int framesLeft, int unk_40);
    PlayerHitArea *AddHitCircle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int framesLeft, int unk_40);
    PlayerHitArea *AddDamageBox(D3DXVECTOR3 *pos, float width, float height, int unk_28, int framesLeft, int unk_40);
    PlayerHitArea *AddDamageCircle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int unk_28, int framesLeft, int unk_40);
    PlayerHitArea *AddKind4Box(D3DXVECTOR3 *pos, float width, float height, int unk_28, int framesLeft, int unk_40);
    PlayerHitArea *AddKind4Circle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int unk_28, int framesLeft, int unk_40);
    PlayerHitArea *AddKind2Circle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int unk_28, int framesLeft, int unk_40);





    int CalcDamage(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, int *damageTaken, int *hitByBomb, int *kind4Damage);

    void SubUnknown30388(float amount)
    {
        if (this->unk_30388 - amount < 1.0f)
        {
            this->unk_30388 = 1.0f;
        }
        else
        {
            this->unk_30388 -= amount;
        }
    }



    void UpdateBullets();



    void DrawBullets();

    int CheckHitAreas(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, Bullet *bullet);




    int CheckGraze(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, Bullet *bullet);



    void AddUnknown30454(int script, D3DXVECTOR3 *pos);





    void UpdateUnknown30454();


    void DrawUnknown30454();



    void DrawHitBullets();


    int UpdateShotInput();


    static int UpdateUnknown0Is2();






    int HandleInput();



    void GetMoveDelta(int direction, int focused, float *deltaX, float *deltaY);


    int playerState;

    BYTE registerChainArg;
    unsigned char pad_5[0x8 - 0x5];

    int sideIdx;
    GameSide *side;
    GameSide *otherSide;
    ChainElem *calcElem;
    ChainElem *drawElem;
    ChainElem *drawElem2;
    int unk_20;


    PlayerUnknown24 unk_24;
    int unk_a0;
    int unk_a4;


    int unk_a8;

    int unk_ac;
    int unk_b0;
    int unk_b4;
    int unk_b8;

    AnmLoaded *anm;
    Sprite sprite;


    Effect *unk_364;


    Effect *unk_368;
    PlayerUnknown36c unk_36c;
    Timer unk_1b74;
    union
    {
        DWORD pad_1b80;
        struct
        {



            DWORD focused : 2;


            DWORD charging : 1;


            DWORD unknownFlag3 : 1;
        };
    };

    int unk_1b84;
    D3DXVECTOR3 pos;
    unsigned char pad_1b94[0x1ba0 - 0x1b94];

    D3DXVECTOR3 posHistory[0x10];


    D3DXVECTOR3 hitboxMin;
    D3DXVECTOR3 hitboxMax;
    D3DXVECTOR3 grazeMin;
    D3DXVECTOR3 grazeMax;
    D3DXVECTOR3 grazeTestMin;
    D3DXVECTOR3 grazeTestMax;

    D3DXVECTOR3 hitboxSize;
    D3DXVECTOR3 grazeSize;
    D3DXVECTOR3 grazeTestHalfSize;




    D3DXVECTOR3 unk_1ccc;

    float unk_1cd8;

    float unk_1cdc;
    float unk_1ce0;

    float moveScaleX;
    float moveScaleY;
    PlayerOption options[4];


    PlayerHitArea hitAreas[0x201];

    PlayerHitArea *activeHitAreas[0x202];
    PlayerHitArea *freeHitAreas[0x202];
    int activeHitAreaCount;
    int freeHitAreaCount;
    unsigned char pad_c118[0xc11c - 0xc118];
    PlayerBullet bullets[0x80];
    PlayerLaser unk_3031c;

    ShtFile *shtFile;
    unsigned char pad_3033c[0x30354 - 0x3033c];

    BYTE unk_30354;
    unsigned char pad_30355[0x30358 - 0x30355];


    int direction;
    float velocityX;
    float velocityY;

    D3DXVECTOR3 orbHomingTarget;

    D3DXVECTOR3 homingTarget;

    Enemy *homingEnemy;

    int homingTargetSet;

    float unk_30384;


    float unk_30388;
    Timer unk_3038c;
    Timer unk_30398;
    Timer unk_303a4;
    Timer unk_303b0;
    Timer unk_303bc;
    Timer stateTimer;
    Timer pad_303d4;


    Effect *unk_303e0;
    unsigned char pad_303e4[0x303e8 - 0x303e4];

    int unk_303e8;
    unsigned char pad_303ec[0x303f0 - 0x303ec];


    int unk_303f0;
    int unk_303f4;
    int unk_303f8;
    int unk_303fc;
    int unk_30400;


    PlayerPosFunc unk_30404;
    PlayerEnemyFunc unk_30408;
    unsigned char pad_3040c[0x30410 - 0x3040c];
    PlayerUnknown30410 unk_30410;
    PlayerUnknown30454 unk_30454[4];


    float unk_30f64;
    float unk_30f68;
    float pad_30f6c;
};
ASSERT_SIZE(Player, 0x30f70);



struct ShotData
{

    short frame;
    unsigned char pad_2[0x4 - 0x2];

    float offsetX;
    float offsetY;
    float sizeX;
    float sizeY;
    float angle;
    float speed;
    short damage;
    unsigned char pad_1e[0x20 - 0x1e];

    short option;

    short type;

    short script;
    short sound;

    ShotFireFunc fire;

    ShotUpdateFunc update;
    ShotUpdateFunc draw;
    ShotHitFunc hit;
};
ASSERT_SIZE(ShotData, 0x38);



struct ShotTable
{
    ShotData *shots;
    unsigned char pad_4[0x8 - 0x4];
};
ASSERT_SIZE(ShotTable, 0x8);



struct ShtFile
{
    unsigned char pad_0[0x2];

    WORD tableCount;

    float hitboxSize;
    float grazeSize;
    unsigned char pad_c[0x10 - 0xc];
    float unknown10;


    float speed;
    float speedFocused;
    float speedDiagonal;
    float speedDiagonalFocused;

    float unk_24;

    float unk_28;

    char spellCardNames[3][0x40];
    unsigned char pad_ec[0x42c - 0xec];
    ShotTable tables[1];
};


void FireShot(Player *player, PlayerBullet *bullet, ShotData *sd);



int TryFireShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);



void FireShots(Player *player, int frame, int table);



extern char *g_CharacterFiles[][6];



extern ShtFile *g_ShtFiles[2];



extern ShotFireFunc g_ShotFireFuncs[9];
extern ShotUpdateFunc g_ShotUpdateFuncs[8];
extern ShotUpdateFunc g_ShotDrawFuncs[4];
extern ShotHitFunc g_ShotHitFuncs[4];




extern PlayerComboFunc g_CharacterFuncs9d0[16];

extern int g_CharacterValuesA10[16];


extern const int g_PlayerCpuDirButtons[9];
extern const int g_PlayerCpuAltDirs[2][9];


extern int g_PlayerCpuDirOrder[12][10];

extern PlayerCpuLevel g_PlayerCpuLevels[71];
extern PlayerPosFunc g_CharacterFuncsA50[16];
extern PlayerEnemyFunc g_CharacterFuncsA90[16];
extern int g_CharacterValuesAd0[16];
extern int g_CharacterValuesB10[16];
extern int g_CharacterValuesB50[16];
extern int g_CharacterValuesB90[16];
extern int g_CharacterValuesBd0[16];
