

#pragma once

#include "bullet.h"
#include "chain.h"
#include "sprite.h"
#include "timer.h"

class EnemyManager;
struct GameSide;
struct Enemy;
struct EnemyScriptState;
struct EnemyScriptInstr;

#include "enemyscript.h"



struct BulletShooter
{

    BulletShooter()
    {
        memset(this, 0, sizeof(BulletShooter));
        this->bulletSound = -1;
    }



    BulletCommand *SetCommand(int i, int concurrent, DWORD flag);


    void SetAngleAccelCommand(int i, int concurrent, int frames, float arg1, float arg0);

    void SetAccelCommand(int i, int concurrent, int frames, float arg0, float arg1);

    void SetWaitCommand(int i, int concurrent, int frames);


    short sprite;
    short color;
    D3DXVECTOR3 pos;
    float angle1;
    float angle2;
    float speed1;
    float speed2;

    BulletCommand commands[18];

    float exFloats[4];
    int exInts[4];
    int laserEndHitFrames;
    short count1;
    short count2;
    WORD aimMode;
    WORD unk_1fa;

    DWORD flags;

    int sound;
    int bulletSound;

    int commandIndex;

    char unk_20c;
    unsigned char pad_20d[0x210 - 0x20d];

    BulletTypeSprites *typeSprites;
};
ASSERT_SIZE(BulletShooter, 0x214);



struct EnemyBox
{
    EnemyBox()
    {
    }

    D3DXVECTOR3 pos;

    D3DXVECTOR3 velocity;
    float angle;
};
ASSERT_SIZE(EnemyBox, 0x1c);



struct Enemy
{


    void ResetRankSpeeds()
    {
        this->rankSpeedLow = -0.5f;
        this->rankSpeedHigh = 0.5f;
        this->rankCount1Low = 0;
        this->rankCount1High = 0;
        this->rankCount2Low = 0;
        this->rankCount2High = 0;
    }


    void ReleaseEffects();

    void FreeThreads();


    void Despawn();


    void ClampPos();


    void Move();


    void CheckPlayerContact(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);


    void DropItems(int kind);



    int CheckLifeCallback();
    static int CheckTimerCallback();

    EnemyManager *manager;

    Enemy *nextInDrawList;
    Sprite primarySprite;
    Sprite sprites[2];

    EnemyScriptState ctx;
    EnemyScriptState savedCtx[0x10];

    EnemyScriptState *currentCtx;

    EnemyScriptState *callStack;

    int enemyInts[8];
    float enemyFloats[8];
    short savedStackDepth;
    short stackDepth;
    unsigned char pad_2d2c[0x2d2e - 0x2d2c];
    short deathSub;

    short interrupts[0x20];

    short pendingInterrupt;
    unsigned char pad_2d72[0x2d74 - 0x2d72];
    D3DXVECTOR3 pos;

    D3DXVECTOR3 posOrigin;
    D3DXVECTOR3 velocity;

    D3DXVECTOR3 velocity2;

    D3DXVECTOR3 prevPos;
    D3DXVECTOR3 posDelta;
    D3DXVECTOR3 hitboxSize;
    D3DXVECTOR3 extraHitboxSize;
    D3DXVECTOR3 absPos;
    float angle;
    float angularVelocity;

    float orbitAngle;
    float orbitAngularVelocity;
    unsigned char pad_2df0[0x2df4 - 0x2df0];
    float speed;
    float acceleration;
    float orbitRadius;
    float orbitRadiusVelocity;
    D3DXVECTOR3 shootOffset;


    D3DXVECTOR3 moveDelta;
    D3DXVECTOR3 moveStart;
    Timer moveTimer;
    int moveDuration;

    float rankSpeedLow;
    float rankSpeedHigh;
    short rankCount1Low;
    short rankCount1High;
    short rankCount2Low;
    short rankCount2High;
    int life;

    int maxLife;
    int phaseStartLife;
    int score;

    int index;

    int unk_2e5c;
    int unk_2e60;
    Timer callbackTimer;

    D3DCOLOR color;
    BulletShooter shooter;

    unsigned char shootInstr[0x2c];
    int shootInterval;
    Timer shootTimer;
    BulletShooter laserShooter;

    Laser *lasers[0x20];
    int laserIndex;

    int item;

    int pointItems;
    int powerItems;

    char deathEffect;


    BYTE deathEffect2;
    BYTE deathEffect3;

    BYTE bossIndex;
    BYTE hitFlashTimer;
    unsigned char pad_336d[0x3370 - 0x336d];
    Timer pad_3370;

    union
    {
        DWORD flags;
        struct
        {
            DWORD active : 1;
            DWORD isBoss : 1;
            DWORD shootable : 1;
            DWORD contactDamage : 1;


            DWORD hidden : 1;

            DWORD hiddenByBomb : 1;
            DWORD hitByShots : 1;
            DWORD grazeable : 1;


            DWORD unknownFlag8 : 1;
            DWORD moveMode : 2;
            DWORD moveEasing : 3;
            DWORD holdFire : 1;
            DWORD mirrored : 1;
            DWORD clampPos : 1;
            DWORD deathMode : 3;
            DWORD unk_bit20 : 1;
            DWORD onScreen : 1;
            DWORD rotateSprite : 1;

            DWORD noCallStack : 1;
            DWORD keepSpellCard : 1;
            DWORD staysOffscreen : 1;
            DWORD isWarpingToBoss : 1;
            DWORD haltedByBomb : 1;
            DWORD intangibleDuringBomb : 1;
            DWORD pad_bit29 : 2;

            DWORD useStageAnm : 1;
        };
    };
    union
    {
        DWORD flags2;
        struct
        {

            DWORD hasDied : 1;
            DWORD lifeCallbackProximity : 2;

            DWORD undying : 1;
            DWORD halted : 1;
            DWORD useMainSpriteOffset : 1;
            DWORD unk_3380 : 3;

            DWORD unknownBit9 : 1;

            DWORD unknownType : 2;


            DWORD useVelocity2 : 1;
            DWORD unknownBit13 : 1;
            DWORD unknownBit14 : 1;
        };
    };
    unsigned char pad_3384[0x3386 - 0x3384];

    BYTE spriteDirection;
    BYTE drawListIndex;

    BYTE difficultyMaskExtra;
    unsigned char pad_3389[0x338a - 0x3389];
    short turnStill;
    short turnStillFromLeft;
    short turnStillFromRight;
    short turnLeft;
    short turnRight;
    short turnExtra;
    unsigned char pad_3396[0x3398 - 0x3396];

    float lowerBound[2];
    float upperBound[2];
    float minShootDistSq;

    int lastDamage;
    int lifeCallbackLife[4];
    int lifeCallbackSub[4];
    int timerCallbackTime;
    int timerCallbackSub;

    EnemyScriptThread *threads[4];
    EnemyBox trailBoxes[0x60];

    EnemyBox spriteStripVertices[0xc2];

    BYTE trailFlags;
    unsigned char pad_53a1[0x1];
    short trailSegmentCount;
    short trailContactSegments;
    short trailSegmentStep;
    Timer invulnTimer;

    struct Effect *effects[0x18];
    int effectCount;
    float effectRadius;
    unsigned char pad_541c[0x5420 - 0x541c];

    int phaseEndSecondsLeft;
    Timer unk_5424;
};
ASSERT_SIZE(Enemy, 0x5430);


struct EnemyTimelineSpawn
{
    int sub;
    float x;
    float y;
    int life;
    int item;
    int score;
};
struct EnemyTimelineSpawnRandomX
{
    int sub;
    float xMin;
    float xMax;
    float y;
    int life;
    int item;
    int score;
};
struct EnemyTimelineSpawnAnyX
{
    int sub;
    float y;
    int life;
    int item;
    int score;
};
struct EnemyTimelineSpawnWithArgs
{
    int sub;
    float x;
    float y;
    int life;
    int pointItems;
    int powerItems;
    int score;
};



struct EnemyTimelineEntry
{
    int time;
    short opcode;
    BYTE size;
    BYTE difficulties;
    int args[1];
};

class EnemyManager;


struct EnemyTimeline
{




    int Run();

    Timer timer;

    Timer delay;

    int mirror;
    EnemyManager *mgr;
    EnemyTimelineEntry *pos;
};


class EnemyManager
{
public:

    EnemyManager();


    void Initialize();


    static void RemoveChain(EnemyManager *mgr);

    void Reset();


    static int OnUpdate(EnemyManager *mgr);
    static int AddedCallback(EnemyManager *mgr);
    static int OnDraw(EnemyManager *mgr);
    static int OnDraw2(EnemyManager *mgr);



    static EnemyManager *RegisterChain(int sideIdx);


    Enemy *FindByUnknownType(int type);




    int ClearEnemiesForBonus(int maxBonus, int start);



    Enemy *Spawn(int sub, D3DXVECTOR3 *pos, int life, int itemDrop, int score, int mirrored,
                      int usePlayerEcl);

    Enemy *SpawnPlayerEnemy(int sub, D3DXVECTOR3 *pos, int life, int itemDrop, int score, int mirrored);


    Enemy *SpawnWithVars(int sub, D3DXVECTOR3 *pos, int life, char itemDrop, int score, EnemyScriptVars *vars,
                              int usePlayerEcl);

    Enemy *SpawnPlayerEnemyWithVars(int sub, D3DXVECTOR3 *pos, int life, char itemDrop, int score, EnemyScriptVars *vars);


    EnemyScripts ecl;
    EnemyScripts playerEcl;
    ChainElem *calcElem;
    ChainElem *drawElem;
    ChainElem *drawElem2;

    int sideIdx;

    GameSide *side;
    GameSide *otherSide;

    Enemy enemyTemplate;

    Enemy enemies[0x81];

    Enemy *bosses[4];
    unsigned char pad_2ac398[0x2ac3ac - 0x2ac398];



    int enemyCount;
    int unk_2ac3b0;
    int unk_2ac3b4;
    int unk_2ac3b8;
    unsigned char pad_2ac3bc[0x2ac3c8 - 0x2ac3bc];
    Timer unk_2ac3c8;
    unsigned char unk_2ac3d4;
    unsigned char pad_2ac3d5[0x2ac3dc - 0x2ac3d5];
    int scriptArg163;

    EnemyTimeline timeline;
    Timer frameTimer;


    Enemy *drawListHeads[4];

    AnmLoaded *anm;

    AnmLoaded *otherPlayerAnm;
    unsigned char pad_2ac428[0x2ac42c - 0x2ac428];

    int spawnFailed;

    int timelineSignals[4];
    int holdTimelineSpawns;



    Enemy *unk_2ac444;
    Enemy *unk_2ac448;

    int unk_2ac44c;
};
ASSERT_SIZE(EnemyManager, 0x2ac450);



extern unsigned char g_TimelineOrder[0x100];



float LerpAngleShort(float from, float to, float t);
