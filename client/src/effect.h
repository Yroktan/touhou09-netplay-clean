

#pragma once

#include <d3dx8.h>

#include "chain.h"
#include "timer.h"

struct GameSide;
class AnmLoaded;
class EffectManager;
class Sprite;
struct VertexTex1DiffuseXyzrwh;


struct Effect
{

    EffectManager *mgr;


    Sprite *sprites;
    int spriteCount;
    D3DXVECTOR3 pos;

    D3DXVECTOR3 velocity;
    unsigned char pad_24[0x3c - 0x24];



    D3DXVECTOR3 unk_3c;
    D3DXVECTOR3 unk_48;
    D3DXVECTOR3 unk_54;
    D3DXVECTOR3 unk_60;
    unsigned char pad_6c[0x7c - 0x6c];



    float unk_7c;
    float unk_80;
    float unk_84;
    float unk_88;
    int unk_8c;

    int reservedSlot;
    float unk_94;
    float unk_98;
    float unk_9c;
    float unk_a0;


    short unk_a4;
    short unk_a6;
    short unk_a8;
    short unk_aa;
    Timer timer;
    unsigned char pad_b8[0xbc - 0xb8];

    int (*update)(Effect *effect);
    int (*draw)(Effect *effect);

    BYTE active;

    BYTE type;

    BYTE fadeOut;
    unsigned char pad_c7[0xc8 - 0xc7];


    BYTE unk_c8;
    BYTE unk_c9;
    BYTE unk_ca;
    unsigned char pad_cb[0xcc - 0xcb];


    VertexTex1DiffuseXyzrwh *vertices;

    BYTE unk_d0;
    unsigned char pad_d1[0xd4 - 0xd1];

    Effect *next;
};
ASSERT_SIZE(Effect, 0xd8);



struct EffectInfo
{

    int script;
    int scriptStep;
    int spriteCount;


    int (*init)(Effect *effect);
    int (*update)(Effect *effect);
    int (*draw)(Effect *effect);
};
ASSERT_SIZE(EffectInfo, 0x18);
extern EffectInfo g_EffectInfos[];



class EffectManager
{
public:

    void Init()
    {
        memset(this, 0, sizeof(EffectManager));
    }


    EffectManager()
    {
        this->Init();
        this->colorScale[0] = 1.0f;
        this->colorScale[1] = 1.0f;
        this->colorScale[2] = 1.0f;
        this->colorScale[3] = 1.0f;
    }



    static int FreeEffects(EffectManager *mgr);


    static void RemoveChain(EffectManager *mgr);

    void Reset();



    Effect *StartEffect(Effect *effect, int effectIdx, D3DXVECTOR3 *pos, D3DCOLOR color, D3DXVECTOR3 *velocity);




    Effect *SpawnEffect(int effectIdx, D3DXVECTOR3 *pos, int count, D3DCOLOR color);


    Effect *SpawnEffectWithVelocity(int effectIdx, D3DXVECTOR3 *pos, D3DXVECTOR3 *velocity, int count,
                                       D3DCOLOR color);



    Effect *SpawnReservedEffect(int effectIdx, D3DXVECTOR3 *pos, int slot, D3DCOLOR color);


    static int OnUpdate(EffectManager *mgr);
    static int OnDraw(EffectManager *mgr);
    static int OnDraw2(EffectManager *mgr);



    static int DrawUnknown114(EffectManager *mgr);


    static int AddedCallback(EffectManager *mgr);




    static EffectManager *RegisterChain(int sideIdx, int count34, int count38);


    Effect *GetReservedEffect(int slot)
    {
        return &this->effects[slot + 0x100];
    }


    int next;
    unsigned char pad_4[0x8 - 0x4];

    int activeCount;
    int sideIdx;

    GameSide *side;
    GameSide *otherSide;
    ChainElem *calcElem;
    ChainElem *drawElem;
    float colorScale[4];
    Effect *effects;

    int unk_34;
    int unk_38;

    Effect drawList0;
    Effect drawList1;
    Effect drawList2;


    Effect *drawListTails[3];

    int frames;
    AnmLoaded *anm;
    unsigned char pad_2d8[0x2dc - 0x2d8];
};
ASSERT_SIZE(EffectManager, 0x2dc);




int InitOrientedEffect(Effect *effect);




int EffectUpdateFollowPlayer(Effect *effect);




int UpdateEffectFromSprite(Effect *effect);





int EffectInitRing(Effect *effect);




int InitEffectTypes1And2(Effect *effect);



int InitEffectTypes3And4(Effect *effect);





int UpdateEffectTypes3And4(Effect *effect);





int UpdateEffectTypes1And2(Effect *effect);






int EffectDrawRing(Effect *effect);
