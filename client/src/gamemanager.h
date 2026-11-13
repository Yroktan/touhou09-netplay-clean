

#pragma once

#include "chain.h"
#include "unsorted.h"


struct GameStats
{

    float lives;

    DWORD guiScore;
    DWORD score;

    DWORD scoreIncrement;

    int extendsGiven;

    BYTE unk_14;
    unsigned char pad_15[0x34 - 0x15];

    float unk_34;
    unsigned char pad_38[0xa0 - 0x38];
};
ASSERT_SIZE(GameStats, 0xa0);

class Stage;
class Player;
class BulletManager;
class EffectManager;
class EnemyManager;
class SpellCard;
class GuiSide;
class Gui;
class ExAttackManager;
class ReplayManager;


struct GameSide
{


    void AddScore(int points)
    {
        this->stats->score += points / 10;
    }


    Stage *stage;
    Player *player;
    BulletManager *bulletManager;
    EffectManager *effectManager;
    EnemyManager *enemyManager;
    SpellCard *spellCard;
    GuiSide *gui;
    GameStats *stats;


    int character;
    int unk_24;
    int unk_28;
    int unk_2c;

    int unk_30;
    union
    {
        DWORD flags;
        struct
        {


            DWORD flag0 : 1;
        };
    };
};
ASSERT_SIZE(GameSide, 0x38);



struct GameManagerUnknownF0
{


    short round;
    short stage;

    short character;
    short unk_6;
    short unk_8;
    short unk_a;
    short unk_c;

    short weight;
};
ASSERT_SIZE(GameManagerUnknownF0, 0x10);

struct GameRank
{


    void LowerUnknownCc(int amount)
    {
        this->unk_cc -= amount;
        if (this->unk_cc < 0)
        {
            this->unk_cc = 0;
        }
    }


    DWORD unk_b0;


    int unk_b4;
    int unk_b8;
    int unk_bc;
    DWORD unk_c0;

    DWORD unk_c4;
    int unk_c8;

    int unk_cc;
    int unk_d0;
};
ASSERT_SIZE(GameRank, 0x24);


class GameManager
{
public:
    GameManager()
    {
        this->Init();
    }



    void Init();




    static int OnDraw(GameManager *mgr);










    static int OnUpdate(GameManager *mgr);
    static int AddedCallback(GameManager *mgr);
    static int DeletedCallback(GameManager *mgr);

    static void LoadingThread(GameManager *arg);



    static int RegisterChain();



    float ArenaToScreenX(float x)
    {
        return g_App.camera->viewport.X + x - this->arena[0];
    }
    float ArenaToScreenY(float y)
    {
        return g_App.camera->viewport.Y + y - this->arena[1];
    }


    float ScreenToArenaX(float x)
    {
        return x - g_App.camera->viewport.X + this->arena[0];
    }
    float ScreenToArenaY(float y)
    {
        return y - g_App.camera->viewport.Y + this->arena[1];
    }


    BOOL IsInPlayArea(float x, float y, float width, float height);


    void ResetState();

    int IsDemo();


    int IsReplay()
    {
        return this->stateFlags >> 3 & 1;
    }


    int IsInPractice()
    {
        return this->stateFlags & 1;
    }

    BOOL IsMode2();



    static void ResetRound();



    void SetPlayArea()
    {
        this->arena[0] = -144.0f;
        this->arena[1] = 0.0f;
        this->arena[2] = 288.0f;
        this->arena[3] = 448.0f;
        this->playerArea[0] = -136.0f;
        this->playerArea[1] = 16.0f;
        this->playerArea[2] = 272.0f;
        this->playerArea[3] = 416.0f;
    }



    void AddExtend();

    BOOL IsMode0()
    {
        return this->mode == 0;
    }

    BOOL IsMode1()
    {
        return this->mode == 1;
    }





    void PickOpponent();

    GameSide sides[3];
    Gui *gui;
    ExAttackManager *exAttackManager;
    GameRank rank;
    ReplayManager *replayManager;


    BYTE unk_d8[0x10];


    GameConfig *config;

    float timelineSpeed;
    GameManagerUnknownF0 *unk_f0;


    int stage;
    int unk_f8;
    int unk_fc;
    int unk_100;

    int unk_104;
    int unk_108[2];
    int unk_110;
    int unk_114;
    int mode;
    int difficulty;


    int difficultyMask;

    int loadingThreadState;
    int unk_128;
    int unk_12c;
    int unk_130;
    union
    {
        DWORD stateFlags;
        struct
        {
            unsigned int unusedBits134_0 : 5;


            unsigned int stageEndState : 2;
            unsigned int unusedBits134_7 : 4;


            unsigned int unknownBits134_11 : 2;


            unsigned int unknownBit134_13 : 1;
            unsigned int unknownBit134_14 : 1;
        };
    };
    int unk_138;

    BYTE isPaused;

    BYTE isInContinueMenu;

    BYTE unk_13e;

    BYTE demoIndex;

    int demoFrames;
    unsigned char pad_144[0x145 - 0x144];

    char replayFile[0x200];
    unsigned char pad_345[0x346 - 0x345];

    WORD rngSeed;
    int frames;
    unsigned char pad_34c[0x350 - 0x34c];
    float arena[4];
    float playerArea[4];
    unsigned char pad_370[0x374 - 0x370];

    int unk_374;


    int initialLives[2];
    int unk_380;
    unsigned char pad_384[0x38c - 0x384];


    int unk_38c;
};
ASSERT_SIZE(GameManager, 0x390);
extern GameManager g_GameManager;


struct Hscr
{
    Hscr();

    DWORD magic;
    WORD size;

    WORD size2;
    BYTE version;

    BYTE isNew;
    unsigned char pad_a[0x2];
    DWORD score;

    float slowdown;
    BYTE character;
    BYTE difficulty;

    BYTE rank;
    BYTE unk_17;
    char name[9];

    char date[6];
    unsigned char pad_27[0x2b - 0x27];
    BYTE unk_2b;
};
ASSERT_SIZE(Hscr, 0x2c);


extern Hscr g_Hscr;


extern Hscr g_HighScores[16][5][5];



extern int g_Unknown4a1500[5][9][5];

extern int g_Unknown4a1884[5];


extern GameManagerUnknownF0 *g_StoryOpponents[14];
extern GameManagerUnknownF0 g_MatchStages[17];

#ifdef TH09_EXE

#define g_Unknown4d9698 (*(unsigned char (*)[0x2bc0])&g_AsciiText.popups)
#else
extern unsigned char g_Unknown4d9698[0x2bc0];
#endif


extern ChainElem g_GameManagerCalcChainElem;
extern ChainElem g_GameManagerDrawChainElem;




void RemoveGameManagerChain();


void FormatTodayDate(char *buf);
