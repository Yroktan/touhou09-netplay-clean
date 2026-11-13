

#pragma once

#include "chain.h"
#include "sprite.h"
#include "timer.h"

struct GameSide;



struct StageInstr
{
    int frame;
    short opcode;
    short pad_06;
    D3DXVECTOR3 args;
};



struct StageQuad
{
    short type;

    short size;
    short script;

    short spriteIndex;
    D3DXVECTOR3 pos;
    float width;
    float height;
};



struct StageLineQuad
{
    short type;
    short size;
    short script;
    short spriteIndex;
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    float width;
};


struct StageObject
{
    BYTE pad_00[0x2];

    char layer;

    char flags;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 size;
    StageQuad quads[1];
};


struct StageInstance
{
    short id;
    short pad_02;
    D3DXVECTOR3 pos;
};


struct StageData
{
    short objectCount;
    short quadCount;
    int instancesOffset;
    int scriptOffset;
    DWORD pad_0c;
    char name[0x80];
    char songNames[4][0x80];
    char songPaths[4][0x80];

};


struct StageFog
{
    float nearPlane;
    float farPlane;
    D3DCOLOR color;
};



struct StageCamera
{
    D3DVECTOR eye;
    D3DVECTOR at;
    D3DVECTOR up;
    D3DVECTOR dir;
    D3DVECTOR cameraRight;
    D3DVECTOR offset;
    float fov;
    unsigned char pad_4c[0xf0 - 0x4c];
};



class Stage
{
public:

    Stage();



    static void RemoveChain(Stage *stage);




    static void OnRemoved(Stage *stage)
    {
    }

    void Reset();



    static int OnUpdate(Stage *stage);
    static int AddedCallback(Stage *stage);
    static int DeletedCallback(Stage *stage);

    static int OnDraw(Stage *stage);
    static int OnDraw2(Stage *stage);



    static Stage *RegisterChain(int sideIdx);



    int Load(char *dataPath);


    int DrawLayer(int layer);


    void JumpToWaitLabel();



    void RunScript();


    int StepObjects();


    void StartSpellCard()
    {
        this->spellCardState = 1;
        this->spellCardFrames = 0;
    }

    ChainElem *calcElem;
    ChainElem *drawElem;

    ChainElem *drawElem2;
    int sideIdx;

    GameSide *side;
    GameSide *otherSide;

    int waitLabel;


    int tintFadeMode;
    int tintFadeFrames;


    Sprite *sprites;
    StageInstr *script;


    Sprite backgroundLayer0;
    Sprite backgroundLayer1;
    Sprite backgroundLayer2;
    AnmLoaded *anm;

    AnmLoaded *playerAnm;

    StageData *data;
    int quadCount;
    int objectCount;
    StageObject **objects;
    StageInstance *instances;


    StageCamera cameraFrom;
    StageCamera cameraTo;
    StageCamera cameraFadeTarget;
    StageCamera cameraFadeStart;

    int cameraFadeFrames[5];
    Timer cameraFadeTimer[5];
    int cameraFadeMode[5];
    Timer scriptTime;
    int scriptIndex;

    int unk_c68;
    unsigned char pad_c6c[0xc70 - 0xc6c];
    D3DVECTOR cameraPos;


    D3DCOLOR clearColor;
    unsigned char pad_c80[0xc84 - 0xc80];
    Timer pad_c84;
    unsigned char pad_c90[0xc94 - 0xc90];
    StageFog fog;
    StageFog fogStart;
    StageFog fogTarget;
    int fogFadeFrames;
    Timer fogFadeTime;

    unsigned char fogColorChanged;
    unsigned char pad_cc9[0xccc - 0xcc9];

    int spellCardState;

    int spellCardFrames;

    int clearScreenOnce;


    int dialogueSpriteCount;

    int dialogueScriptOffset;
    Sprite dialogueSprites[0x20];
    Sprite pad_6160;

    void (*spellCardDrawCallback)();

    D3DVECTOR nextKeyPos;
    int nextKeyFrame;
    D3DVECTOR keyPos;
    int keyFrame;

    BYTE cameraOffsetChanged;
    unsigned char pad_6429[0x642c - 0x6429];

    union
    {
        D3DCOLOR color;
        struct
        {
            BYTE b;
            BYTE g;
            BYTE r;
            BYTE a;
        };
    } tintColor;


    int keepColorModulation;

    float drawDistanceSq;

    BYTE shakeMode;
    unsigned char pad_6439[0x643c - 0x6439];



    int effectSpawnPosCount;
    int recordEffectSpawnPos;
    D3DXVECTOR3 effectSpawnPos[0x20];
};
ASSERT_SIZE(Stage, 0x65c4);


extern char *g_StageFiles[][2];
