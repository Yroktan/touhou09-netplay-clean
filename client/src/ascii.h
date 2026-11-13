

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"


struct AsciiString
{
    char text[0x40];
    D3DXVECTOR3 pos;
    D3DCOLOR color;
    float scaleY;
    float scaleX;

    int highlight;

    int inArena;
};
ASSERT_SIZE(AsciiString, 0x60);


struct AsciiPopup
{

    BYTE digits[0xc];
    D3DXVECTOR3 pos;
    D3DCOLOR color;
    Timer timer;
    float scaleY;
    float scaleX;
    BYTE active;
    BYTE digitCount;
    unsigned char pad_32[0x6];
};
ASSERT_SIZE(AsciiPopup, 0x38);



class PauseMenu
{
public:




    int UpdatePauseMenu();

    void DrawPauseMenu();

    int state;

    int stateFrames;
    Sprite sprites[7];

    Sprite background;
};
ASSERT_SIZE(PauseMenu, 0x1528);


class ContinueMenu
{
public:






    int UpdateContinueMenu();

    void DrawContinueMenu();

    int state;
    int stateFrames;
    Sprite sprites[5];
    Sprite background;
};
ASSERT_SIZE(ContinueMenu, 0xfe0);



class UnknownMenu
{
public:



    int UpdateUnknownMenu();

    void DrawUnknownMenu();

    int state;
    int stateFrames;
    Sprite sprites[3];
};
ASSERT_SIZE(UnknownMenu, 0x7f4);


class AsciiText
{
public:

    AsciiText();



    static int DeletedCallback(AsciiText *text);

    static void RemoveChain();


    void AddString(D3DXVECTOR3 *pos, char *text);

    void AddFormat(D3DXVECTOR3 *pos, char *fmt, ...);

    void DrawPopups(int side);


    static int OnUpdate(AsciiText *text);


    static int OnDraw(AsciiText *text);
    static int OnDraw2(AsciiText *text);


    static int RegisterChain();


    void Initialize();



    void StartScripts();



    static int AddedCallback(AsciiText *text);

    void DrawStrings();


    void AddPopup(int side, D3DXVECTOR3 *pos, int value, D3DCOLOR color);


    void SetBossMarkerInterrupt(int i, short interrupt)
    {
        this->bossMarkers[i].SetPendingInterrupt(interrupt);
    }

    void SetBossMarkerPos(int i, D3DXVECTOR3 *pos)
    {
        this->bossMarkers[i].pos = *pos;
    }

    void SetBossMarkerState(int i, int value)
    {
        this->bossMarkerStates[i] = value;
    }



    void ClearStrings()
    {
        this->stringCount = 0;
    }


    void SetAlpha(int alpha)
    {
        this->alpha = alpha;
    }


    void SetScale(float x, float y);

    void SetGlyphWidth(int width)
    {
        this->glyphWidth = width;
    }



    Sprite glyph;
    Sprite digit;
    Sprite digit2;


    Sprite pad_7ec;
    Sprite pad_a90;
    Sprite pad_d34;
    Sprite pad_fd8;
    Sprite pad_127c;
    Sprite pad_1520;
    Sprite bossMarkers[4];
    int bossMarkerStates[4];
    AsciiString strings[0x100];

    int stringCount;

    union
    {
        D3DCOLOR color;
        struct
        {
            BYTE colorRgb[3];
            BYTE alpha;
        };
    };
    float scaleY;
    float scaleX;
    int inArena;
    int highlight;
    unsigned char pad_827c[0x8280 - 0x827c];

    int glyphWidth;

    int frameCounter;

    AnmLoaded *anm;
    AnmLoaded *anm2;

    int nextPopup;
    int nextPopup2;
    unsigned char pad_8298[0x829c - 0x8298];
    int unk_829c;
    PauseMenu pauseMenu;
    ContinueMenu continueMenu;
    UnknownMenu unknownMenu;

    Sprite demoIcon;
    AsciiPopup popups[2][100];
    unsigned char pad_de00[0xde08 - 0xde00];
    Sprite pad_de08;
};
ASSERT_SIZE(AsciiText, 0xe0ac);
extern AsciiText g_AsciiText;


extern ChainElem g_AsciiCalcElem;
extern ChainElem g_AsciiDrawElem;
extern ChainElem g_AsciiDrawElem2;
