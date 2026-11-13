

#pragma once

#include <windows.h>
#include <math.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <d3dx8.h>

#include "common.h"
#include "dsutil.h"








inline float Atan2(float y, float x)
{
    return (float)atan2((double)y, (double)x);
}
float Fmod(float x, float y);
inline float Fabs(float x)
{
    return (float)fabs((double)x);
}
float Tan(float angle);
float Acos(float x);
float Atan(float x);


class Rng
{
public:

    WORD GetRandomU16();

    DWORD GetRandomU32();

    float GetRandomF32ZeroToOne();

    float GetRandomF32MinusOneToOne();


    WORD GetRandomU16InRange(WORD range)
    {
        return range != 0 ? this->GetRandomU16() % range : 0;
    }


    DWORD GetRandomU32InRange(DWORD range)
    {
        return range != 0 ? this->GetRandomU32() % range : 0;
    }


    float GetRandomF32InRange(float range)
    {
        return this->GetRandomF32ZeroToOne() * range;
    }
    float GetRandomF32SignedInRange(float range)
    {
        return this->GetRandomF32MinusOneToOne() * range;
    }


    WORD GetSeed();

    void SetSeed(WORD seed);

    void ResetGenerated();

    WORD seed;
    unsigned char pad_2[0x2];

    DWORD generated;
};
extern Rng g_Rng;


class Log
{
public:

    Log();

    ~Log() {}


    void Reset();



    void Flush();

    char buffer[0x2000];

    char *cursor;
    char hasError;
};
extern Log g_Logger;



#pragma pack(push, 4)
struct GameWindow
{
    GameWindow();


    double GetTime();



    int RunFrame();

    HWND hwnd;

    int isClosing;

    int isAppActive;
    int isAppInactive;
    char curFrame;
    unsigned char pad_11[0x3];


    DWORD perfFrequency[2];


    unsigned char forceClockPacing;
    unsigned char pad_1d[0x20 - 0x1d];


    UINT screenSaveActive;
    UINT lowPowerActive;
    UINT powerOffActive;
    double now;
    double lastTime;
    double nextFrameTime;
};
#pragma pack(pop)
ASSERT_SIZE(GameWindow, 0x44);
extern GameWindow g_GameWindow;

extern HANDLE g_AppMutex;



int CreateD3d();


void __stdcall ForceForegroundWindow(HWND hwnd);



LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);




int CheckSingleInstance();


int InitD3dDevice();

void SetDefaultRenderStates();




void PresentFrame();



int ChecksumExecutable();


int ResolveShortcut(char *path, char *out, int size);



int LogPrintf(Log *log, const char *fmt, ...);

int LogError(Log *log, const char *fmt, ...);




WORD InitJoystick();

extern JOYCAPSA g_JoyDeviceCaps[2];





WORD ReadJoystick(int input, int pad, WORD buttons);



BYTE *GetPadButtons(int player);

extern BYTE g_PadButtons[0x80];





WORD UpdatePlayerInput(int player, WORD buttons);


void ResetKeyboard();

int OpenGameArchive();


int MeasureRefreshRate();


void DrawFps(int draw);






WORD ReadButtons(int player);


WORD ReadNetInput();

void ReadInputs();



extern char g_DataDir[];
extern char g_PathBuffer[0x108];


char *__stdcall GetDataPath(char *name);



int FileExists(char *path);




int WriteToFile(char *path, void *data, int size);


void DebugPrint(const char *fmt, ...);



BYTE *DecryptData(BYTE *data, int size, BYTE key, BYTE step, int blockSize, int limit);



BYTE *EncryptData(BYTE *data, int size, BYTE key, BYTE step, int blockSize, int limit);


struct CryptParams
{


    BYTE type;
    BYTE key;
    BYTE step;
    BYTE pad_3;
    int blockSize;
    int limit;
};
ASSERT_SIZE(CryptParams, 0xc);

extern CryptParams g_CryptParams[8];


extern BYTE g_CryptMagic[];




BYTE *DecryptFileData(BYTE *data, int *size, int fileSize);




void *LoadFileData(char *path, int *size, int isExternal);



float AddNormalizeAngle(float a, float b);

inline void SinCos(float *sinOut, float *cosOut, float angle)
{
    __asm
    {
        fld angle
        fsincos
        mov eax, cosOut
        fstp dword ptr [eax]
        mov eax, sinOut
        fstp dword ptr [eax]
    }
}

void Rotate2D(D3DXVECTOR3 *out, D3DXVECTOR3 *in, float angle);




void HermiteInterpolate(D3DXVECTOR3 *out, D3DXVECTOR3 *p1, D3DXVECTOR3 *p0, D3DXVECTOR3 *m1, D3DXVECTOR3 *m0,
                        float time, float duration);


struct Vector2
{

    void SetPolarEllipse(float angle, float radiusX, float radiusY);

    float x;
    float y;
};





static void SetPolar(Vector2 *v, float angle, float radius)
{
    __asm
    {
        mov eax, v
        fld angle
        fsincos
        fmul radius
        fstp dword ptr [eax]
        fmul radius
        fstp dword ptr [eax+4]
    }
}



struct GameConfigFlags
{
    unsigned int force16BitTextures : 1;
    unsigned int referenceRasterizer : 1;
    unsigned int noFog : 1;
    unsigned int noDirectInput : 1;
    unsigned int preloadBgm : 1;
    unsigned int noVsync : 1;

    unsigned int noTextRenderDetect : 1;
    unsigned int unusedBits : 25;
};



struct ControllerMapping
{
    short shootButton;
    short bombButton;
    short focusButton;
    short menuButton;
    short upButton;
    short downButton;
    short leftButton;
    short rightButton;
    short skipButton;
};
ASSERT_SIZE(ControllerMapping, 0x12);


struct PlayerInput
{

    PlayerInput();



    WORD IsPressed(WORD mask)
    {
        return (WORD)(this->unk_6 & mask) != 0 || (WORD)(this->unk_4 & mask) != 0;
    }




    WORD IsHeld(WORD mask)
    {
        return this->unk_0 & mask;
    }
    WORD IsJustPressed(WORD mask)
    {
        return this->unk_6 & mask;
    }
    WORD IsHeld2(WORD mask);
    WORD IsJustPressed2(WORD mask);





    void UpdateRepeat2();

    WORD unk_0;
    WORD unk_2;
    WORD unk_4;
    WORD unk_6;
    WORD unk_8;

    WORD heldFrames[16];


    WORD unk_2a;
    WORD unk_2c;
    WORD unk_2e;
    WORD unk_30;
    WORD unk_32;
    WORD unk_34;
    unsigned char pad_36[0x38 - 0x36];

    WORD heldFrames2[16];

    ControllerMapping mappings[3];
};
ASSERT_SIZE(PlayerInput, 0x8e);
extern PlayerInput g_PlayerInputs[3];


struct BgmDatHeader
{

    DWORD magic;
    int version;
    int gameVersion;
    DWORD unk_c;
};
ASSERT_SIZE(BgmDatHeader, 0x10);


struct GameConfig
{


    GameConfig()
    {
        this->SetDefaults();
    }

    void SetDefaults();


    ControllerMapping mappings[2][3];
    unsigned char pad_6c[0xa4 - 0x6c];
    DWORD version;
    short padThresholdX;
    short padThresholdY;


    BYTE unk_ac;


    BYTE colorMode16Bits;

    BYTE musicMode;

    BYTE playSounds;

    BYTE difficulty;

    BYTE windowed;

    BYTE frameSkip;
    BYTE unk_b3;

    BYTE unk_b4[3];



    BYTE pads[3];
    char bgmVolume;
    char seVolume;
    unsigned char pad_bc[0xc8 - 0xbc];
    GameConfigFlags flags;
};
ASSERT_SIZE(GameConfig, 0xcc);

class AnmLoaded;
class MidiPlayer;
class MidiTimer;
class Sprite;


struct RunRecord
{
    WORD unk_0;
    WORD seed;
    int value;
    int unk_8;
};
ASSERT_SIZE(RunRecord, 0xc);



struct Camera
{

    D3DXVECTOR3 eye;
    D3DXVECTOR3 at;
    D3DXVECTOR3 up;

    D3DXVECTOR3 dir;

    D3DXVECTOR3 cameraRight;
    D3DXVECTOR3 offset;

    float fov;
    D3DXMATRIX viewMatrix;
    D3DXMATRIX projectionMatrix;
    D3DVIEWPORT8 viewport;

    int index;

    float shakeX;
    float shakeY;
};
ASSERT_SIZE(Camera, 0xf0);

#include "app.h"
