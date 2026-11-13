

#pragma once

#include <windows.h>
#include <mmsystem.h>

#include "common.h"



union MidiShortMsg
{
    DWORD dword;
    BYTE bytes[4];
};



WORD SwapBytes16(WORD value);



DWORD SwapBytes32(DWORD value);


struct MidiChunk
{
    char id[4];

    DWORD length;
};



DWORD ReadVarLen(BYTE **cursor);


class MidiDevice
{
public:

    MidiDevice();
    ~MidiDevice();


    int Close();




    int Open(UINT deviceId);



    int SendLongMsg(LPMIDIHDR header);



    int SendShortMsg(BYTE status, BYTE data1, BYTE data2);

    HMIDIOUT handle;
    UINT deviceId;
};


class MmTimer
{
public:

    MmTimer();
    ~MmTimer();




    UINT Start(UINT delay, LPTIMECALLBACK callback, DWORD_PTR user);



    int Stop();




    virtual void OnTimer();



    static void CALLBACK DefaultCallback(UINT id, UINT msg, DWORD_PTR user, DWORD_PTR dw1, DWORD_PTR dw2);

    UINT timerId;
    TIMECAPS caps;
};
ASSERT_SIZE(MmTimer, 0x10);



struct MidiTrack
{

    int active;

    int delta;

    DWORD length;

    BYTE runningStatus;

    BYTE *data;

    BYTE *cursor;


    BYTE *loopCursor;
    int loopDelta;
};
ASSERT_SIZE(MidiTrack, 0x20);



struct MidiChannel
{

    BYTE notes[0x10];
    BYTE program;

    BYTE bank;
    BYTE pan;
    BYTE reverb;
    BYTE chorus;

    BYTE volume;

    BYTE scaledVolume;
};
ASSERT_SIZE(MidiChannel, 0x17);



class MidiPlayer : public MmTimer
{
public:
    MidiPlayer();

    ~MidiPlayer();

    virtual void OnTimer();


    void UnloadFile(int slot);


    int LoadFile(int slot, char *path);


    void SendVolume(int offset);


    int Stop();

    int ReleaseHeader(LPMIDIHDR header);


    int Play();


    void Restart(int slot);

    void Rewind();


    int FadeOut(int duration);

    void FreeTracks();


    int ParseFile(int slot);


    int Load(char *path);


    void PlayFile(char *path)
    {
        this->Stop();
        this->Load(path);
        this->Play();
    }


    void ProcessEvent(MidiTrack *track);


    int playingSlot;

    LPMIDIHDR headers[0x20];

    int nextHeader;

    void *fileData[0x20];

    int trackCount;

    int format;
    int division;

    int tempo;

    unsigned __int64 ticks;


    unsigned __int64 baseTime;

    MidiTrack *tracks;
    MidiDevice device;
    unsigned char pad_144[0x10];
    MidiChannel channels[0x10];

    char transpose;

    float volumeScale;

    int lastVolume;
    int unk_2d0;
    int suppressVolume;
    int unk_2d8;
    int unk_2dc;

    int fading;

    int fadeDuration;

    int fadeElapsed;

    int loopTempo;
    unsigned __int64 loopTicks;
    unsigned __int64 loopBaseTime;
};
ASSERT_SIZE(MidiPlayer, 0x300);


extern LARGE_INTEGER g_MidiTimerLastCounter;




class MidiTimer : public MmTimer
{
public:

    virtual void OnTimer();


    void StartTimer();

    void StopTimer();

    int unknown10;
};
ASSERT_SIZE(MidiTimer, 0x14);
