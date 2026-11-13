

#include "scoredat.h"
#include "lzss.h"
#include "unsorted.h"
#include "gamemanager.h"

int InsertHighScore(Hscr *entry)
{
    int rank;
    int i;

    for (rank = 0; rank < 5; rank++)
    {
        if (entry->score >= g_HighScores[entry->character][entry->difficulty][rank].score)
        {
            break;
        }
    }
    if (rank < 5)
    {
        for (i = 4; i > rank; i--)
        {
            g_HighScores[entry->character][entry->difficulty][i] =
                g_HighScores[entry->character][entry->difficulty][i - 1];
            g_HighScores[entry->character][entry->difficulty][i].rank = i;
        }
        g_HighScores[entry->character][entry->difficulty][rank] = *entry;
        g_HighScores[entry->character][entry->difficulty][rank].rank = rank;
        return rank;
    }
    return 99;
}

int LoadHscrChunks(ScoreDat *dat)
{
    int character;
    int difficulty;
    int rank;
    Hscr *chunk;
    int left;

    for (character = 0; character < 16; character++)
    {
        for (difficulty = 0; difficulty < 5; difficulty++)
        {
            for (rank = 0; rank < 5; rank++)
            {
                g_HighScores[character][difficulty][rank].score = 100000 - rank * 20000;
                g_HighScores[character][difficulty][rank].slowdown = 0.0f;
                g_HighScores[character][difficulty][rank].magic = 0x52435348;
                g_HighScores[character][difficulty][rank].character = character;
                g_HighScores[character][difficulty][rank].difficulty = difficulty;
                g_HighScores[character][difficulty][rank].rank = rank;
                g_HighScores[character][difficulty][rank].version = 2;
                g_HighScores[character][difficulty][rank].size2 = sizeof(Hscr);
                g_HighScores[character][difficulty][rank].size = sizeof(Hscr);
                g_HighScores[character][difficulty][rank].unk_17 = 0;
                g_HighScores[character][difficulty][rank].isNew = 0;
                strcpy(g_HighScores[character][difficulty][rank].name, "No Name ");
                strcpy(g_HighScores[character][difficulty][rank].date, "--/--");
                g_HighScores[character][difficulty][rank].unk_2b = 0;
            }
        }
    }
    chunk = (Hscr *)((BYTE *)dat + dat->dataOffset);
    left = dat->fileSize - dat->dataOffset;
    while (left > 0)
    {
        if (chunk->magic == 0x52435348 && chunk->version == 2)
        {
            g_HighScores[chunk->character][chunk->difficulty][chunk->rank] = *chunk;
        }
        left -= chunk->size;
        chunk = (Hscr *)((BYTE *)chunk + chunk->size);
    }
    g_Hscr.size2 = sizeof(Hscr);
    g_Hscr.size = sizeof(Hscr);
    strcpy(g_Hscr.name, "No Name ");
    g_Hscr.slowdown = 0.0f;
    g_Hscr.character = 0;
    g_Hscr.difficulty = 0;
    g_Hscr.magic = 0;
    g_Hscr.unk_17 = 0;
    g_Hscr.version = 2;
    g_Hscr.isNew = 0;
    strcpy(g_Hscr.date, "--/--");
    return 0;
}

int LoadLsnmChunk(ScoreDat *dat)
{
    Lsnm *chunk;
    int left;

    chunk = (Lsnm *)((BYTE *)dat + dat->dataOffset);
    memset(&g_Lsnm, 0, sizeof(g_Lsnm));
    g_Lsnm.magic = 0x4d4e534c;
    g_Lsnm.size = sizeof(Lsnm);
    g_Lsnm.size2 = sizeof(Lsnm);
    g_Lsnm.version = 1;
    g_Lsnm.unk_9 = 0;
    strcpy(g_Lsnm.name, "        ");
    left = dat->fileSize - dat->dataOffset;
    while (left > 0)
    {
        if (chunk->magic == 0x4d4e534c && chunk->version == 1)
        {
            g_Lsnm = *chunk;
            return 1;
        }
        left -= chunk->size;
        chunk = (Lsnm *)((BYTE *)chunk + chunk->size);
    }
    return 0;
}

int LoadPlstChunk(ScoreDat *dat)
{
    PlayRecord *chunk;
    int left;

    memset(&g_PlayRecord, 0, sizeof(g_PlayRecord));
    g_PlayRecord.size = sizeof(PlayRecord);
    g_PlayRecord.size2 = sizeof(PlayRecord);
    g_PlayRecord.bgmHeard[0] = 1;
    g_PlayRecord.openEntries[0] = 1;
    g_PlayRecord.openEntries[1] = 1;
    g_PlayRecord.openEntries[2] = 1;
    g_PlayRecord.openEntries[3] = 1;
    g_PlayRecord.openEntries[4] = 1;
    g_PlayRecord.openEntries2[0] = 1;
    g_PlayRecord.openEntries2[1] = 1;
    g_PlayRecord.openEntries2[2] = 1;
    g_PlayRecord.openEntries2[3] = 1;
    g_PlayRecord.openEntries2[4] = 1;
    g_PlayRecord.magic = 0x54534c50;
    g_PlayRecord.version = 3;
    g_PlayRecord.unk_9 = 0;
    chunk = (PlayRecord *)((BYTE *)dat + dat->dataOffset);
    left = dat->fileSize - dat->dataOffset;
    while (left > 0)
    {
        if (chunk->magic == 0x54534c50 && chunk->version == 3)
        {
            g_PlayRecord = *chunk;
        }
        left -= chunk->size;
        chunk = (PlayRecord *)((BYTE *)chunk + chunk->size);
    }
    return 0;
}

ScoreDat::ScoreDat()
{
    memset(this, 0, sizeof(*this));
}

Hscr::Hscr()
{
    memset(this, 0, sizeof(*this));
}

ScoreDat *LoadScoreDat(char *path)
{
    BOOL found;
    Th9k *th9k;
    int left;
    ScoreChunk *chunk;
    int remaining;
    BYTE *p;
    WORD checksum;
    BYTE key;
    int n;
    BYTE *decoded;
    ScoreDat *dat;
    int readSize;
    ScoreDat *unpacked;

    LogPrintf(&g_Logger, "info : score load\r\n");
    dat = (ScoreDat *)LoadFileData(path, &readSize, 1);
    if (dat == NULL)
    {
    fail:
        LogPrintf(&g_Logger, "info : score recreate\r\n");
        if (dat != NULL)
        {
            g_MemoryTracker.Free(dat);
        }
        dat = (ScoreDat *)g_MemoryTracker.Alloc(sizeof(ScoreDat), "scorefile");
        dat->dataOffset = sizeof(ScoreDat);
        dat->fileSize = sizeof(ScoreDat);
        dat->version = 4;
        goto done;
    }
    if (readSize < sizeof(ScoreDat))
    {
        LogPrintf(&g_Logger, "warning : score.dat size is short\r\n");
        g_MemoryTracker.Free(dat);
        goto fail;
    }
    decoded = DecryptData((BYTE *)dat, readSize, 0x3a, 0xcd, 0x100, 0xc00);
    g_MemoryTracker.Free(dat);
    dat = (ScoreDat *)decoded;
    remaining = readSize - 2;
    checksum = 0;
    key = 0;
    n = 0;
    p = (BYTE *)dat + 1;
    while (remaining > 0)
    {
        key += *p;
        key = ((key & 0xe0) >> 5) | ((key & 0x1f) << 3);
        p[1] ^= key;
        if (n >= 2)
        {
            checksum += p[1];
        }
        p++;
        remaining--;
        n++;
    }
    if (dat->checksum != checksum)
    {
        LogPrintf(&g_Logger, "warning : score.dat chksum error\r\n");
        goto fail;
    }
    if (dat->dataOffset != sizeof(ScoreDat))
    {
        LogPrintf(&g_Logger, "warning : header size is mismatch\r\n");
        goto fail;
    }
    if (dat->version != 4)
    {
        LogPrintf(&g_Logger, "warning : score.dat version mismatch\r\n");
        goto fail;
    }
    unpacked = (ScoreDat *)g_MemoryTracker.Alloc(sizeof(ScoreDat) + 0xa0000, "scorefile2");
    *unpacked = *dat;
    LzssDecompress((BYTE *)dat + sizeof(ScoreDat), dat->packedSize, (BYTE *)unpacked + sizeof(ScoreDat),
                   dat->dataSize);
    g_MemoryTracker.Free(dat);
    dat = unpacked;
    left = dat->fileSize;
    found = FALSE;
    chunk = (ScoreChunk *)((BYTE *)dat + dat->dataOffset);
    left -= dat->dataOffset;
    while (left > 0)
    {
        if (chunk->magic == 0x4b394854)
        {
            found = TRUE;
            th9k = (Th9k *)chunk;
        }
        if (chunk->size == 0)
        {
            LogPrintf(&g_Logger, "warning : score.dat chapter size is ZERO\r\n");
            goto fail;
        }
        left -= chunk->size;
        chunk = (ScoreChunk *)((BYTE *)chunk + chunk->size);
    }
    if (!found || th9k->version != 1)
    {
        LogPrintf(&g_Logger, "warning : score.dat version mismatch\r\n");
        goto fail;
    }
done:
    return dat;
}

void ReleaseScoreDat(ScoreDat *dat)
{
    g_MemoryTracker.Free(dat);
}

int LoadScoreFile()
{
    ScoreDat *dat;

    dat = LoadScoreDat("score.dat");
    g_ScoreDatHeader = *dat;
    LoadHscrChunks(dat);
    LoadLsnmChunk(dat);
    LoadPlstChunk(dat);
    ReleaseScoreDat(dat);
    return 0;
}

int SaveScoreDat()
{
    int size;
    Vrsm vrsm;
    BYTE key;
    ScoreDat *dat;
    BYTE *p;
    BYTE *encrypted;
    BYTE prev;
    int m;
    Th9k th9k;
    void *packed;
    BYTE *buf;
    int left;

    left = 7;
    if (g_ScoreDatHeader.version == 0)
    {
        size = 1;
        return size;
    }
    buf = (BYTE *)g_MemoryTracker.Alloc(0x640000, "scoretmp");
    memcpy(buf, &g_ScoreDatHeader, sizeof(ScoreDat));
    size = sizeof(ScoreDat);
    th9k.magic = 0x4b394854;
    th9k.size = th9k.size2 = sizeof(Th9k);
    th9k.version = 1;
    memcpy(buf + size, &th9k, sizeof(Th9k));
    size += sizeof(Th9k);

    do
    {
        m = g_Rng.GetRandomU32InRange(3);
        if ((left & 1) && m == 0)
        {
            memcpy(buf + size, g_HighScores, sizeof(g_HighScores));
            size += sizeof(g_HighScores);
            left ^= 1;
        }
        else if ((left & 2) && m == 1)
        {
            memcpy(buf + size, &g_Lsnm, sizeof(Lsnm));
            size += sizeof(Lsnm);
            left ^= 2;
        }
        else if ((left & 4) && m == 2)
        {
            g_App.AddPlayTime();
            if (g_PlayRecord.version != 0)
            {
                memcpy(buf + size, &g_PlayRecord, sizeof(PlayRecord));
                size += sizeof(PlayRecord);
                left ^= 4;
            }
        }
    } while (left != 0);
    vrsm.magic = 0x4d535256;
    vrsm.version = 1;
    vrsm.size = vrsm.size2 = sizeof(Vrsm);
    vrsm.clearedOnSave = 0;
    strcpy(vrsm.gameVersion, "0150a");
    vrsm.exeSize = g_App.exeSize;
    vrsm.exeChecksum = g_App.exeChecksum;
    memcpy(buf + size, &vrsm, sizeof(Vrsm));
    size += sizeof(Vrsm);
    dat = (ScoreDat *)buf;
    dat->dataSize = size - sizeof(ScoreDat);
    dat->fileSize = size;
    packed = LzssCompress(buf + sizeof(ScoreDat), dat->dataSize, &dat->packedSize);
    memcpy(buf + sizeof(ScoreDat), packed, dat->packedSize);
    GlobalFree(packed);
    size = dat->packedSize + sizeof(ScoreDat);
    dat->dataOffset = sizeof(ScoreDat);
    dat->checksum = 0;
    dat->key = g_Rng.GetRandomU16InRange(256);
    dat->randomByte = g_Rng.GetRandomU16InRange(256);
    dat->version = 4;
    m = 4;
    while (m < size)
    {
        dat->checksum = dat->checksum + buf[m];
        m += 1;
    }
    key = dat->key;
    p = (BYTE *)dat + 1;
    for (m = size - 2; m > 0; m--)
    {
        prev = p[1];
        key = ((key & 0xe0) >> 5) | ((key & 0x1f) << 3);
        p[1] ^= key;
        p++;
        key += prev;
    }
    encrypted = EncryptData(buf, size, 0x3a, 0xcd, 0x100, 0xc00);
    WriteToFile("score.dat", encrypted, size);
    g_MemoryTracker.Free(buf);
    g_MemoryTracker.Free(encrypted);
    return 0;
}
