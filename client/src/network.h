

#pragma once

#include <windows.h>
#include <dplay8.h>

#include "common.h"


struct NetPacket
{

    BYTE type;

    char side;
    WORD seed;
    union
    {

        int value;

        BYTE syncFrames;
    };

    union
    {
        int unk_8;
        struct
        {
            WORD input;
            WORD checksum;
        };
    };
};
ASSERT_SIZE(NetPacket, 0xc);



struct Network
{

    Network();


    void Init();

    void SetUnknownA8();
    void ClearUnknownA8();


    int Close();

    int GetOtherUnknown1c();


    void ReleaseInterfaces();


    HRESULT Send(void *data, DWORD size);


    BOOL IsServiceProviderValid(const GUID *serviceProvider);


    HRESULT CreateDeviceAddress();


    HRESULT CreateHostAddress();

    HRESULT Host();


    HRESULT Connect();

    HRESULT HandleMessage(PVOID context, DWORD type, PVOID message);


    HRESULT CreatePeer();

    IDirectPlay8Peer *peer;
    IDirectPlay8Address *deviceAddress;
    unsigned char pad_8[4];
    IDirectPlay8Address *unk_c;
    IDirectPlay8Address *hostAddress;
    DPNHANDLE sendHandle;
    DPNHANDLE connectHandle;

    int unk_1c[2];

    char hostName[0x80];

    DWORD port;

    int unk_a8;

    int unk_ac;

    int unk_b0;

    int initialFramesQueued;

    int unk_b8;

    int isLan;

    int side;

    unsigned char syncFrames;
    unsigned char pad_c5[3];

    NetPacket lastPacket;
};
ASSERT_SIZE(Network, 0xd4);
extern Network *g_Network;


void LogNetworkError(HRESULT hr);


HRESULT WINAPI NetworkMessageHandler(PVOID context, DWORD type, PVOID message);
