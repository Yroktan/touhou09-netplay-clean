


#pragma once

#include "chain.h"
#include "common.h"
#include "timer.h"


class ScreenEffect
{
public:



    static ScreenEffect *StartEffect(int type, int duration, int a, int b, int c, int drawPriority, int side);


    static int AddedCallback(ScreenEffect *effect);


    static int DeletedCallback(ScreenEffect *effect);


    static int OnUpdateFadeIn(ScreenEffect *effect);



    static int OnUpdateFadeOut(ScreenEffect *effect);



    static int OnUpdateLoadingFade(ScreenEffect *effect);



    static int OnUpdateFlash(ScreenEffect *effect);




    static int OnUpdateShake(ScreenEffect *effect);


    static int OnUpdateShakeRamp(ScreenEffect *effect);



    static int OnDrawScreen(ScreenEffect *effect);
    static int OnDrawPlayArea(ScreenEffect *effect);
    static int OnDrawWholeScreen(ScreenEffect *effect);
    static int OnDrawFlash(ScreenEffect *effect);


    static int OnDrawPlayAreas(ScreenEffect *effect);


    void Finish();


    int side;
    int type;
    ChainElem *calcElem;
    ChainElem *drawElem;
    unsigned char pad_10[0x14 - 0x10];

    int alpha;
    int duration;
    union
    {
        DWORD color;

        int shakeFrom;

        int riseFrames;

        int flashes;
    };
    union
    {

        DWORD flashColor;

        int shakeTo;

        DWORD holdFrames;
    };
    union
    {
        int flashCount;

        DWORD fallFrames;
    };

    int finishing;
    Timer timer;
};
ASSERT_SIZE(ScreenEffect, 0x38);



extern ScreenEffect *g_Unknown4a7c80;


void ClearScreen(DWORD color);



void ClearViewport(DWORD color);



void DrawColoredLineStrip(struct VertexDiffuseXyzrwh *vertices, int count);



void DrawColorRect(float *rect, DWORD color);
