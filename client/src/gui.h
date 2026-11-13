


#pragma once

#include "chain.h"
#include "sprite.h"
#include "timer.h"



struct MsgArgs
{
    short who;
    union
    {
        short index;
        BYTE value;
    };
    char text[1];
};




struct MsgPortraitArgs
{
    int who;
    int sprites[2];
};



struct MsgInstr
{
    WORD time;
    BYTE opcode;
    BYTE argsSize;
    union
    {
        MsgArgs args;
        MsgPortraitArgs portraitArgs;
        int i;
        BYTE b;
    };
};


struct MsgFile
{
    int count;
    struct
    {
        MsgInstr *script;

        int weight;
    } scripts[1];
};



extern MsgFile *g_MsgFile;
extern MsgFile *g_MsgFile2;


struct GuiMsg
{



    void Start(int index, int side);



    void StartRandom(int side);




    int Draw();



    int Run();




    void UnloadBgm(int firstScript);

    void Clear();





    int Load(char *path, char *path2, int firstScript);

    MsgFile *msgFile;

    MsgInstr *instr;

    int scriptIndex;

    MsgFile *msgFile2;

    int side;
    Sprite portraits[8];

    Sprite text[2];
    Sprite nameplate;

    D3DCOLOR textColors[4];
    D3DCOLOR shadowColors[4];

    Timer timer;

    int waitFrames;
    int minWaitFrames;
    int fontSize;


    Timer boxTimer;

    BYTE speaker;
    BYTE newPage;
    BYTE nextLine;
    BYTE activePortrait;


    DWORD timelineRelease;
    BYTE skippable;

    BYTE textBoxShown;
    unsigned char pad_1d6e[0x1d70 - 0x1d6e];
};
ASSERT_SIZE(GuiMsg, 0x1d70);




struct GuiUnknownE86c
{
    GuiUnknownE86c()
    {
    }

    D3DXVECTOR3 pos;
    float w;
    D3DCOLOR diffuse;
};



class Gui
{
public:

    void Init()
    {
        memset(this, 0, sizeof(Gui));
    }


















    static int OnUpdate(Gui *gui);
    static int AddedCallback(Gui *gui);
    static int OnDraw(Gui *gui);



    static Gui *RegisterChain();


    static int LoadAnm();

    static void ReleaseAnm();


    static int FreeResources(Gui *gui);


    static void RemoveChain(Gui *gui);



    void CountRoundWin(int side);



    void UpdateEnemyMarker(int side, D3DXVECTOR3 *pos);





    void ScoreRoundBonus();

    void Reset();

    int GetUnknown11ea8()
    {
        return this->unk_11ea8;
    }


    int IsMsgRunning()
    {
        return this->msg.scriptIndex >= 0 || this->msg.scriptIndex == -2 ? 1 : 0;
    }

    ChainElem *calcElem;
    ChainElem *drawElem;

    AnmLoaded *anm;
    Sprite unk_c[0x58];

    GuiUnknownE86c unk_e86c[2][5];

    D3DCOLOR unk_e934[2];
    int unk_e93c[2];
    GuiMsg msg;
    Sprite unk_106b4;

    int unk_10958;


    int unk_1095c;
    int unk_10960;
    int unk_10964;
    Sprite unk_10968[8];


    int unk_11e88;
    int unk_11e8c;
    int unk_11e90;
    int unk_11e94;
    int unk_11e98;
    int unk_11e9c;
    int bonusTotal;
    int unk_11ea4;

    int unk_11ea8;
    Sprite unk_11eac[2];

    Sprite enemyMarkers[2];
};
ASSERT_SIZE(Gui, 0x1293c);

struct GameSide;


extern char *g_Pl06FcAnmFiles[2];



struct GuiSideA678
{

    AnmLoaded *anm;



    AnmLoaded *unk_4;
    int unk_8;
    Sprite sprites[2];
};
ASSERT_SIZE(GuiSideA678, 0x554);



class GuiSide
{
public:

    void Init()
    {
        memset(this, 0, sizeof(GuiSide));
    }









    static int OnUpdate(GuiSide *side);
    static int AddedCallback(GuiSide *side);
    static int OnDraw(GuiSide *side);



    static void RemoveChain(GuiSide *side);

    void Reset();

    void StartUnknownAbcc();

    void StartUnknown59c8();


    void StartUnknown5c6c();

    void StopUnknown5c6c();


    void StartUnknown66fc(int digit);


    void StartUnknown6ee8(int idx);

    void StartUnknown7430();


    void StartUnknown76d4(int frames);


    void SetUnknown76d4Time(int frames);



    static GuiSide *RegisterChain(int sideIdx);

    Sprite unk_0[0xb];
    Sprite unk_1d0c[5];
    Sprite unk_2a40[7];
    Sprite unk_3cbc;
    Sprite unk_3f60[10];
    Sprite unk_59c8;
    Sprite unk_5c6c[4];
    Sprite unk_66fc[3];
    Sprite unk_6ee8[2];
    Sprite unk_7430;
    Sprite unk_76d4[6];
    Sprite unk_86ac[7];
    Sprite unk_9928[5];

    int unk_a65c;
    ChainElem *calcElem;
    ChainElem *drawElem;

    AnmLoaded *anm;
    int sideIdx;

    GameSide *side;
    GameSide *otherSide;
    GuiSideA678 unk_a678;

    int unk_abcc;
    Timer timerAbd0;
    Timer timerAbdc;
};
ASSERT_SIZE(GuiSide, 0xabe8);
