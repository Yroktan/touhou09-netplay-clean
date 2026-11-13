

#pragma once

#include <windows.h>

#include "common.h"



struct ScoreDat
{
    ScoreDat();

    BYTE pad_00;

    BYTE key;

    WORD checksum;
    WORD version;

    BYTE randomByte;
    BYTE pad_07;

    int dataOffset;

    int fileSize;

    int dataSize;
    int packedSize;
};
ASSERT_SIZE(ScoreDat, 0x18);

extern ScoreDat g_ScoreDatHeader;


struct ScoreChunk
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_9;
    unsigned char pad_a[0x2];
};
ASSERT_SIZE(ScoreChunk, 0xc);


struct Th9k
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    unsigned char pad_9[0x3];
};
ASSERT_SIZE(Th9k, 0xc);


struct Lsnm
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_9;
    unsigned char pad_a[0x2];
    char name[0xc];
};
ASSERT_SIZE(Lsnm, 0x18);

extern Lsnm g_Lsnm;


struct Vrsm
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE clearedOnSave;
    unsigned char pad_a[0x2];
    char gameVersion[0x8];
    DWORD exeSize;
    DWORD exeChecksum;
};
ASSERT_SIZE(Vrsm, 0x1c);



ScoreDat *LoadScoreDat(char *path);


int InsertHighScore(struct Hscr *entry);

int LoadHscrChunks(ScoreDat *dat);


int LoadLsnmChunk(ScoreDat *dat);


int LoadPlstChunk(ScoreDat *dat);

void ReleaseScoreDat(ScoreDat *dat);


struct PlayTime
{
    DWORD hours;
    DWORD minutes;
    DWORD seconds;
    DWORD milliseconds;
};



struct PlayRecord
{


    void AddClear(char character, int difficulty)
    {
        int *clears = &this->clearFlags[character][difficulty];
        if (*clears < 999999)
        {
            *clears += 1;
        }
    }


    void OpenEntry(char entry)
    {
        this->openEntries[entry] = 1;
    }
    void OpenEntry2(char entry)
    {
        this->openEntries2[entry] = 1;
    }
    void OpenEntry3(char entry)
    {
        this->openEntries3[entry] = 1;
    }


    BOOL HasCleared(char character)
    {
        if (this->clearFlags[character][0] != 0 || this->clearFlags[character][1] != 0 ||
            this->clearFlags[character][2] != 0 || this->clearFlags[character][3] != 0 ||
            this->clearFlags[character][4] != 0)
        {
            return TRUE;
        }
        return FALSE;
    }

    int GetClears5(char character)
    {
        return this->clearFlags[character][5];
    }


    void AddClear5(char character)
    {
        int *clears = &this->clearFlags[character][5];
        if (*clears < 999999)
        {
            *clears += 1;
        }
    }



    int GetClearFlags(char character, int difficulty);


    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_9;
    unsigned char pad_a[0xc - 0xa];
    PlayTime runTime;
    PlayTime playTime;

    BYTE bgmHeard[0x20];

    BYTE openEntries[16];
    BYTE openEntries2[16];
    BYTE openEntries3[16];


    int clearFlags[16][6];
};
ASSERT_SIZE(PlayRecord, 0x1fc);
extern PlayRecord g_PlayRecord;


#ifdef TH09_EXE
#define g_RunTime (g_PlayRecord.runTime)
#define g_PlayTime (g_PlayRecord.playTime)
#else
extern PlayTime g_RunTime;
extern PlayTime g_PlayTime;
#endif
