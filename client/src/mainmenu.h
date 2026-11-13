

#pragma once

#include "chain.h"
#include "common.h"
#include "gamemanager.h"
#include "network.h"
#include "replay.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

class MainMenu;


extern char *g_TitleHelpTexts[8];

extern char *g_DemoReplayPaths[3];

extern char *g_NameAlphabet;



extern char *g_ReplayMenuCharacterNames[17];
extern char *g_ReplayMenuDifficultyNames[5];
extern char *g_ReplayMenuModeNames[2];
extern char *g_ReplayMenuMatchNames[4];

extern char *g_ScoreRankNames[5];


extern char *g_OptionsHelpTexts[9];


extern char *g_KeyConfigHelpTexts[10];
extern short g_LastPressedKey;

extern char *g_MusicNotHeardComment[8];


extern char g_VsCpuCharacters[16];


extern char g_VsCharacters[16];


extern char g_StoryCharacters[16];



extern char g_CharacterOrder[16];
extern char g_OpenCharacters[16];


extern char g_ScoreCharacters[16];


extern int g_CheatCodeProgress;


extern int g_Unknown4ac884;


struct MusicTune
{


    MusicTune();


    char path[0x40];
    char title[0x42];

    char comment[8][0x42];
};
ASSERT_SIZE(MusicTune, 0x292);

class MainMenu
{
public:



    MainMenu();



    void PlaySoundNow(int idx, int arg);




    void SetState(int state)
    {
        this->prevState = this->state;
        this->state = state;
        this->stateFrames = 0;
        this->inputLockout = 0;
        this->subState = 0;
        this->subMenuIdleFrames = 0;
    }


    void ReassignButton(ControllerMapping *mapping, short button, short replacement, int unused);


    int ShowButtonDigits(Sprite *vm, short value);




    void SkipClosedEntries(int idx, int delta, char *entries, int count);
    void SkipClosedEntries2(int idx, int delta, char *entries, int count);
    void SkipClosedEntries3(int idx, int delta, char *entries, int count);



    void ShowHalves(int left, int right);









    int DrawState11();
    int DrawState15();
    int DrawState13();
    int DrawState12();


    int DrawNothing()
    {
        return 1;
    }




    int MoveMenuCursor(int count);

    int MoveMenuCursorLR(int count);


    int MoveCursor4(int count);


    int MovePlayerCursor(int idx, int count);

    int MovePlayerCursor2(int player, int idx, int count);


    static int OnDraw(MainMenu *menu);


    static void LoadingThread(MainMenu *menu);


    int ReleaseTitleScene();


    static int DeletedCallback(MainMenu *menu);


    int InitTitleScene();

    static int AddedCallback(MainMenu *menu);



    void SelectEntry(int cursor, int first, int count);


    void ShowEntryFrame(int frame, int idx);


    void SelectEntryStep(int cursor, int first, int count, int stride);


    void SelectEntry2(int cursor, int first, int count, int stride);
    void SelectEntry3(int cursor, int first, int count, int stride);

    void SelectEntryInterrupts(int cursor, int first, int count);


    void FlashHalves(int left, int right);

    void FlashSprite(int idx, int on);






    void MoveCharacterCursor(int player, int selected, int selected2, char *entries);


    void MoveCharacterCursor2(int player, int selected, char *entries);










































    int UpdateState3();
    int UpdateState8b();
    int UpdateState16();
    int UpdateState13();
    int UpdateState11();
    int UpdateState12();
    int UpdateState9();
    int UpdateState10();
    int UpdateState8();
    int UpdateState14();
    int UpdateState15();
    int UpdateState1();
    int UpdateState2();
    int UpdateState6();

    void SelectCursorEntry();



    static int OnUpdate(MainMenu *menu);

    int cursor;

    int replayNameCursor;


    int scoreCharacter;
    int scoreRank;
    int cursors[5];

    int prevCursor;
    int subState;

    int stateFrames;
    unsigned char pad_30[0x84 - 0x30];

    int prevState;
    int unk_88;

    char replayName[9];
    unsigned char pad_95[0x28c - 0x95];


    char replayPaths[50][0x200];
    unsigned char pad_668c[0x681c - 0x668c];

    void *unk_681c[50];

    ReplayHeader replays[50];
    void *loadedReplay;

    int unk_c900;
    int replayCount;

    int replaySlot;
    unsigned char pad_c90c[0xc910 - 0xc90c];
    int subMenuIdleFrames;
    unsigned char pad_c914[0xc918 - 0xc914];
    int unk_c918;
    unsigned char pad_c91c[0xc920 - 0xc91c];
    int unk_c920;
    unsigned char pad_c924[0xc934 - 0xc924];

    MusicTune tunes[32];


    int playingTune;
    int tuneCount;
    int listTop;
    int musicPaused;

    int unk_11b84;
    int unk_11b88;


    int unk_11b8c;

    AnmLoaded *anm;
    AnmLoaded *resultTextAnm;

    Sprite *sprites;

    Sprite *helpSprite;

    Sprite helpSprites[14];
    Sprite textSprites[21];

    Sprite cardDetailSprites[14];

    Sprite commentSprites[8];
    int spriteCount;
    int state;

    int unk_1b22c;
    int inputLockout;

    int idleFrames;


    int loadingState;
    int unk_1b23c;


    int entryFirst;
    int entryCount;
    ChainElem *calcElem;
    ChainElem *drawElem;

    ControllerMapping mappings[2][3];
    GameConfig config;
};
ASSERT_SIZE(MainMenu, 0x1b388);




int SetUpTitleScene(int arg);
