

#pragma once

#include "chain.h"
#include "sprite.h"
#include "timer.h"

struct BulletShooter;
struct GameSide;



struct BulletTypeSprites
{
    Sprite sprite0;
    Sprite sprite1;
    Sprite sprite2;
    Sprite sprite3;
    Sprite sprite4;

    D3DXVECTOR3 hitboxSize;
    BYTE unk_d40;

    BYTE height;

    BYTE sizeClass;
    unsigned char pad_d43[0xd44 - 0xd43];


    int unk_d44;
};
ASSERT_SIZE(BulletTypeSprites, 0xd48);


extern const int g_BulletTypeScripts[23][5];



struct BulletEx
{
    Timer timer;
    float turnSpeed;
    float turnAngle;

    D3DXVECTOR3 accel;

    int duration;
    int maxTurns;
    int turnCount;
};
ASSERT_SIZE(BulletEx, 0x2c);


struct BulletCommand
{
    float arg0;
    float arg1;

    int frames;
    int maxTurns;
    DWORD flag;
    int concurrentFlag;
};
ASSERT_SIZE(BulletCommand, 0x18);

class BulletManager;


struct Bullet
{



    BulletCommand *SetCommand(int i, int concurrent, DWORD flag);


    void SetAccelCommand(int i, int concurrent, int frames, float arg0, float arg1);


    void SetWaitCommand(int i, int concurrent, int frames);


    void SetSpriteCommand(int i, int concurrent, int type, int color);


    void ClearCommands()
    {
        int i;

        this->commandIndex = 0;
        this->flags = 0;
        this->activeFlags = 0;
        for (i = 0; i < 18; i++)
        {
            this->commands[i].flag = 0;
        }
    }


    void FreeBullet();


    BulletManager *mgr;
    BulletTypeSprites sprites;

    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 accel;
    float speed;
    float accelSpeed;
    float turnSpeed;
    float angle;
    float angularVelocity;
    float turnAngle;
    Timer timer;
    Timer grazeDelayTimer;
    unsigned char pad_da0[0xdb0 - 0xda0];

    int boundsCheckDelay;

    DWORD activeFlags;

    DWORD flags;
    unsigned char pad_dbc[0xdbe - 0xdbc];


    unsigned short state;
    WORD outOfBoundsFrames;

    BYTE spawnedFlag;
    BYTE grazed;


    BYTE spawnedInArea;
    unsigned char pad_dc5[0xdc8 - 0xdc5];

    Bullet *next;

    int scriptFuncState;

    int sound;

    int commandIndex;
    BulletCommand commands[18];
    BulletEx ex[7];

    char harmless;

    char unk_10bd;


    char unk_10be;
    unsigned char pad_10bf[0x10c0 - 0x10bf];

    short sprite;
    short color;
};
ASSERT_SIZE(Bullet, 0x10c4);



extern int g_BulletColorOffsets16[16];
extern int g_BulletColorOffsets32[8];


void RunBulletCommands(Bullet *bullet);


struct Laser
{
    Sprite sprite0;
    Sprite sprite1;
    D3DXVECTOR3 pos;
    float angle;
    float startOffset;
    float endOffset;
    float maxLength;
    float width;
    float baseWidth;
    float speed;
    int startFrames;
    int hitStartFrame;
    int durationFrames;
    int endFrames;
    int endHitFrames;
    int active;
    Timer timer;
    WORD flags;
    short color;
    BYTE state;

    BYTE drawDelayFlag;
    unsigned char pad_59a[0x59c - 0x59a];
};
ASSERT_SIZE(Laser, 0x59c);



class BulletManager
{
public:

    BulletManager();



    void ClearAllToItems();


    void ClearDrawLists()
    {
        this->drawLists[0] = this->drawLists[1] = this->drawLists[2] = this->drawLists[3] =
            this->drawLists[4] = this->drawLists[5] = 0;
    }


    void Initialize();


    static void RemoveChain(BulletManager *mgr);

    void Reset();


    void ClearAll(int arg);


    void ConvertBulletsToItems(D3DXVECTOR3 *pos, float arg);


    Laser *SpawnLaser(BulletShooter *shooter);


    static int LoadAnm();

    static void ReleaseAnm();


    void ShowSpawnSprite(Sprite *dst, Sprite *src, BulletTypeSprites *sprites, int color);



    Bullet *SpawnBullet(BulletShooter *shooter, int col, int row, float aimAngle, int set);



    Bullet *FirePattern(BulletShooter *shooter);

    Bullet *FirePattern2(BulletShooter *shooter);


    static int OnUpdate(BulletManager *mgr);
#ifdef TH09_NATIVE_NETPLAY

    void SnapshotProjectiles(bool *occupied) const;
    void AdvanceNewProjectiles(const bool *occupied);
#endif
    static int AddedCallback(BulletManager *mgr);
    static int OnDraw(BulletManager *mgr);



    static BulletManager *RegisterChain(int sideIdx);

    BulletTypeSprites typeSprites[0x20];

    Bullet bullets[0xb0];
    Bullet bullets2[0x169];
    Laser lasers[0x30];

    int bulletCount1;
    int bulletCount2;
    int bulletCount;

    int clearAllFrames;
    Timer updateTimer;
    int updateFrameCount;
    ChainElem *calcElem;
    ChainElem *drawElem;
    int sideIdx;

    GameSide *side;
    GameSide *otherSide;


    Bullet *drawLists[6];

    Bullet *nextBullet;
    Bullet *nextBullet2;

    int clearItemType;
    AnmLoaded *etamaAnm;
};
ASSERT_SIZE(BulletManager, 0x25e1c0);
