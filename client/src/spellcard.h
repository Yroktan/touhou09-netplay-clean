

#pragma once

#include "chain.h"
#include "sprite.h"
#include "timer.h"

struct GameSide;

class SpellCard;

typedef int (*SpellCardCallback)(SpellCard *card);


struct SpellCardCallbacks
{
    SpellCardCallback update;
    SpellCardCallback draw;
    SpellCardCallback end;
};


struct SpellCardType
{
    SpellCardCallback init;
    SpellCardCallbacks callbacks;
};


struct SpellCardSlot
{
    int unk_0;
    int type;
    int kind;
};


class SpellCard
{
public:

    static int OnUpdate(SpellCard *card);
    static int OnDraw(SpellCard *card);


    static int AddedCallback(SpellCard *card);


    static void RemoveChain(SpellCard *card);

    void Reset();



    static SpellCard *RegisterChain(int sideIdx);






    void Start(int type, int kind, int unknown, char *name);

    int sideIdx;
    GameSide *side;
    GameSide *otherSide;
    ChainElem *calcElem;
    ChainElem *drawElem;

    AnmLoaded *anm;

    char name[0x80];
    Timer unk_98;


    int unk_a4;
    int unk_a8;
    Sprite unk_ac[2];
    Sprite unk_5f4[5];

    SpellCardCallbacks callbacks[2];
    unsigned char pad_1340[0x1348 - 0x1340];
    SpellCardSlot slots[2];

    void *unk_1360;
    void *unk_1364;
};
ASSERT_SIZE(SpellCard, 0x1368);


extern SpellCardType g_SpellCardTypes[];
