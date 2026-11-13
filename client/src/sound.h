

#pragma once

#include <windows.h>

#include "common.h"
#include "dsutil.h"
#include "unsorted.h"


struct BgmInfo
{
    int id;

    char *path;
    int unk_08;
    int unk_0c;
};

extern BgmInfo g_BgmInfos[19];


extern int g_BgmUnknown08;



struct SoundCommand
{

    int type;
    int arg;

    int stage;
    char path[0x100];
};
ASSERT_SIZE(SoundCommand, 0x10c);



struct BgmFormat
{

    char name[0x10];
    DWORD offset;
    DWORD size;


    int loopStart;
    int loopEnd;
    WAVEFORMATEX format;
};
ASSERT_SIZE(BgmFormat, 0x34);


struct SoundInfo
{
    int buffer;
    short volume;
    short unk_6;
};

extern SoundInfo g_SoundInfos[0x36];


extern char *g_SoundPaths[0x27];


class SoundPlayer
{
public:

    SoundPlayer();


    int Initialize(HWND hwnd);

    int Release();


    void StopBgm();


    int FindTrackIndex(char *path);


    int LoadSound(int index, char *path);


    int LoadBgmFormat(char *path);




    int LoadBgm(char *path);


    int SwitchBgmTrack(char *path);



    int PreloadBgm(int slot, char *path);



    int PlayPreloadedBgm(int slot);

    int LoadSounds();


    int PlayQueuedSounds();


    void FadeOutMusic(float seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->FadeOut(seconds);
        }
    }


    void FreePreloadedBgm(int slot)
    {
        if (this->bgmPreloadData[slot] != NULL)
        {
            g_MemoryTracker.Free(this->bgmPreloadData[slot]);
            this->bgmPreloadData[slot] = NULL;
        }
    }



    void QueueSound(int idx, int arg);


    void QueueSoundPanned(int idx, float x);


    void QueueSoundCommand(int type, int arg, char *path);


    void UnPause();

    void UpdateBgmFades();

    IDirectSound8 *dsound;
    DWORD pad_004;

    IDirectSoundBuffer *soundBuffers[0x80];

    IDirectSoundBuffer *sounds[0x80];


    int unk_408[0x80];

    IDirectSoundBuffer *silence;

    HWND hwnd;

    CSoundManager *manager;

    DWORD notifyThreadId;
    HANDLE notifyThread;
    DWORD pad_61c;

    int queue[0xc];

    int queueArgCounts[0xc];
    int queueArgs[0xc][0x80];


    BgmFormat *bgmEntries[0x10];
    void *bgmPreloadData[0x10];
    void *bgmPreloadCursor[0x10];
    DWORD bgmPreloadSize[0x10];

    int bgmPlaying;

    BgmFormat *bgmFormat;

    SoundCommand commands[0x20];

    char bgmNames[0x20][0x100];

    char bgmPath[0x100];

    CStreamingSound *bgm;

    HANDLE notifyEvent;
    DWORD pad_6210;

    DWORD bgmFileOffset;

    int bgmVolume;
    int seVolume;
    DWORD bgmBufferVolume;
};
ASSERT_SIZE(SoundPlayer, 0x6224);
extern SoundPlayer g_SoundPlayer;
