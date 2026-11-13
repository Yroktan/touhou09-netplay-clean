

#pragma once

#include "chain.h"
#include "unsorted.h"



struct ReplayInfo
{

    ReplayInfo();

    BYTE randomByte;

    char versionLetter;
    WORD version;

    char date[0xa];
    char name[9];
    BYTE difficulty;
    unsigned char pad_18[0x1c - 0x18];
    GameConfig config;
};



struct ReplayRound
{

    DWORD score;

    WORD rngSeed;
    BYTE character;

    BYTE unk_7;
    BYTE lives;

    char unk_9;
    WORD unk_a;

    BYTE stage;
    unsigned char pad_d[0x10 - 0xd];

    int extendsGiven;
    unsigned char pad_14[0x20 - 0x14];
};
ASSERT_SIZE(ReplayRound, 0x20);



struct ReplayHeader
{

    ReplayHeader();


    char magic[4];
    short version;

    BYTE unk_6;

    BYTE isSaved;
    unsigned char pad_8[0xc - 0x8];

    int size;

    int checksum;

    BYTE randomFill;


    BYTE key;
    unsigned char pad_16[0x18 - 0x16];
    int compressedSize;

    int dataSize;



    void *unk_20[3][10];
    void *extras[10];
    ReplayInfo info;
    unsigned char pad_1a8[0x1d0 - 0x1a8];

    int replayFormatVersion;


    DWORD exeSize;
    DWORD exeChecksum;
    char exeVersion[8];

    BYTE mode;
    BYTE unk_1e5;

    BYTE unk_1e6[2];
    BYTE unk_1e8[2];
    unsigned char pad_1ea[0x1ec - 0x1ea];
};
ASSERT_SIZE(ReplayHeader, 0x1ec);





ReplayHeader *ValidateReplay(void *data, int size);



struct ReplayUserHeader
{
    DWORD magic;
    int size;
    BYTE type;
};



struct ReplayChunk
{
    WORD *inputs[3];
    BYTE *extra;
    int count;
    ReplayChunk *next;
};
ASSERT_SIZE(ReplayChunk, 0x18);



class ReplayManager
{
public:

    ReplayManager()
    {
    }





    static int RecordOnUpdate(ReplayManager *mgr);
    static int RecordAddedCallback(ReplayManager *mgr);
    static int RecordOnUpdate2(ReplayManager *mgr);
    static int PlayOnUpdate(ReplayManager *mgr);
    static int PlayAddedCallback(ReplayManager *mgr);
    static int PlayOnUpdate2(ReplayManager *mgr);





    static ReplayManager *RegisterChain(int mode, char *path);

    static void RunAddedCallback(ReplayManager *mgr, int mode);


    static void RemoveChain(ReplayManager *mgr);


    static void Release(ReplayManager *mgr);



    static int SaveReplay(ReplayManager *mgr, char *path, char *name);



    int frame;


    int framesAfterStageClear;
    ReplayHeader *data;


    WORD *inputCursors[3];
    BYTE *extraCursor;

    ReplayChunk chunks[10];

    ReplayChunk *curChunk;
    void *unk_110;
    int mode;
    char *path;
    unsigned char pad_11c[0x150 - 0x11c];
    ChainElem *calcElem;
    unsigned char pad_154[0x158 - 0x154];
    ChainElem *calcElem2;
    ChainElem *calcElem3;


    WORD seed;
    WORD frameEvents;
};
ASSERT_SIZE(ReplayManager, 0x164);
