

#include "../ascii.h"
#include "../archive.h"
#include "../bullet.h"
#include "../effect.h"
#include "../ending.h"
#include "../enemy.h"
#include "../exattack.h"
#include "../gamemanager.h"
#include "../gdisurface.h"
#include "../gui.h"
#include "../lzss.h"
#include "../mainmenu.h"
#include "../midi.h"
#include "../network.h"
#include "../player.h"
#include "../scoredat.h"
#include "../screeneffect.h"
#include "../sprite.h"
#include "../stage.h"
#include "../unsorted.h"
#include "../sound.h"

typedef char PlayRecordRunTimeOffset[offsetof(PlayRecord, runTime) == 0xc ? 1 : -1];
typedef char PlayRecordPlayTimeOffset[offsetof(PlayRecord, playTime) == 0x1c ? 1 : -1];
typedef char AsciiPopupStorageOffset[offsetof(AsciiText, popups) == 0xb240 ? 1 : -1];
typedef char AsciiPopupStorageSize[sizeof(((AsciiText *)0)->popups) == 0x2bc0 ? 1 : -1];

ScreenEffect *g_Unknown4a7c80;
MsgFile *g_MsgFile;
MsgFile *g_MsgFile2;
ShtFile *g_ShtFiles[2];
Lsnm g_Lsnm;
PlayRecord g_PlayRecord;
Hscr g_HighScores[16][5][5];
ScoreDat g_ScoreDatHeader;
Hscr g_Hscr;
int g_Unknown4ac884;
char g_OpenCharacters[16];
int g_CheatCodeProgress;
JOYCAPSA g_JoyDeviceCaps[2];
char g_PathBuffer[0x108];
char g_DataDir[0x104];
Rng g_Rng;
PlayerInput g_PlayerInputs[3];
Chain g_Chain;
DatArchive g_Archive;
MemoryManager g_MemoryTracker;
Log g_Logger;
BYTE g_PadButtons[0x80];
HANDLE g_AppMutex;
GameWindow g_GameWindow;
ScreenEffect *g_LoadingScreenEffect;
App g_App;
Sprite g_LoadingScreenSprites[3];
NetPacket g_NetInputPacket;
NetPacket g_NetSetupPacket;
Network *g_Network;
LzssNode g_LzssTree[0x2001];
BYTE g_LzssWindow[0x2000];
ChainElem g_AsciiDrawElem;
AsciiText g_AsciiText;
ChainElem g_AsciiCalcElem;
ChainElem g_AsciiDrawElem2;
IDirect3DSurface8 *g_TextSurface;
COLORREF g_TextOutlineColor;
SpriteEngine *g_SpriteEngine;
VertexTex1DiffuseXyzrwh g_DiffuseQuad[4];
VertexTex1Xyzrwh g_TexQuad[4];
VertexTex1DiffuseXyz g_DiffuseQuadXyz[4];
LARGE_INTEGER g_MidiTimerLastCounter;
int g_BgmUnknown08;
SoundPlayer g_SoundPlayer;
