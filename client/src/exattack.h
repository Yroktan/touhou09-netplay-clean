

#pragma once

#include <d3dx8math.h>

#include "chain.h"
#include "gamemanager.h"
#include "sprite.h"
#include "timer.h"

struct ExAttack;

typedef int (*ExAttackCallback)(ExAttack *attack);


struct ExAttack
{

    GameSide *GetSide();


    void AllocSprites(int count, int dataSize)
    {
        this->spriteCount = count;
        dataSize += count * sizeof(Sprite);
        this->sprites = (Sprite *)g_MemoryTracker.Alloc(dataSize, "./system\\global.h");
        memset(this->sprites, 0, dataSize);
        this->data = &this->sprites[count];
    }


    int layer;

    int targetSide;
    int side;

    int active;
    Timer timer;


    Sprite *sprites;
    D3DXVECTOR3 pos;
    int spriteCount;

    ExAttack *next;
    void *data;

    D3DXVECTOR3 *target;

    float angle;


    ExAttackCallback onUpdate;
    ExAttackCallback onDraw;
    ExAttackCallback onFree;
};
ASSERT_SIZE(ExAttack, 0x4c);



class ExAttackManager
{
public:




    ExAttack *AddAttack(int type, D3DXVECTOR3 *pos, int side, D3DXVECTOR3 *target);


    static void RemoveChain(ExAttackManager *mgr);

    void Reset();


    static int OnUpdate(ExAttackManager *mgr);
    static int OnDraw(ExAttackManager *mgr);
    static int OnDraw2(ExAttackManager *mgr);



    static int AddedCallback(ExAttackManager *mgr);



    static ExAttackManager *RegisterChain();

    ChainElem *calcElem;
    ChainElem *drawElem;
    ChainElem *drawElem2;


    int counts[2];
    int limits[2];


    ExAttack attacks[0x101];

    ExAttack *layerHeads[3];
    ExAttack *layerTails[3];
};
ASSERT_SIZE(ExAttackManager, 0x4c80);


struct ExAttackData00
{
    int state;

    D3DXVECTOR3 velocity;


    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    D3DXVECTOR3 tangent1;
    D3DXVECTOR3 tangent0;

    D3DXVECTOR3 targetPos;
};
ASSERT_SIZE(ExAttackData00, 0x4c);



struct ExAttackData19
{
    ExAttackData00 base;

    VertexTex1DiffuseXyzrwh vertices[0x21];

    D3DXVECTOR3 points[0x21];
    float radii[0x21];
    float radiusSpeeds[0x21];

    float scrollU;
    float scrollV;
    int unk_684;
};
ASSERT_SIZE(ExAttackData19, 0x688);



struct ExAttackType
{
    ExAttackCallback onInit;
    ExAttackCallback onUpdate;
    ExAttackCallback onDraw;
    ExAttackCallback onFree;
};
extern ExAttackType g_ExAttackTypes[];



extern int g_Unknown4a1128[];
extern int g_Unknown4a117c[];
