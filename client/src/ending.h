

#pragma once

#include <windows.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"




class Ending
{
public:


    Ending();


    int ReadInt();


    void DrawFade();


    void UpdateFade();



    static int DeletedCallback(Ending *ending);


    static int OnDraw(Ending *ending);


    int Load(char *path);





    static int AddedCallback(Ending *ending);



    int RunScript();



    static int OnUpdate(Ending *ending);



    static int RegisterChain();

    ChainElem *calcElem;
    ChainElem *drawElem;

    float scrollX;
    float scrollY;
    float scrollSpeed;

    Sprite sprites[0x10];
    char *file;
    int canSkip;
    int nextCanSkip;
    AnmLoaded *anm;
    Timer timer;
    Timer lineTimer;
    Timer waitTimer;
    int waitMinFrames;
    int lineMinFrames;
    int lineDelay;
    int lineDelayFast;
    unsigned char pad_2a98[0x4];
    int line;
    D3DCOLOR textColor;

    D3DCOLOR fadeColor;
    int fadeTimer;
    int fadeDuration;
    int fadeType;

    char *cursor;
};
ASSERT_SIZE(Ending, 0x2ab8);


extern char *g_EndingFiles[14];
