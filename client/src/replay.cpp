
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "replay.h"
#include "gamemanager.h"
#include "gui.h"
#include "lzss.h"
#include "unsorted.h"




extern char *g_ReplayCharacterNames[16];
extern char *g_ReplayModeNames[6];
extern char *g_ReplayDifficultyNames[5];




static char *FormatAndAdvance(char *buf, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    return buf + strlen(buf);
}

void PlayerInput::UpdateRepeat2()
{
    WORD buttons = this->unk_2c;
    int i;
    WORD *held;

    this->unk_4 = 0;
    held = this->heldFrames2;
    for (i = 0; i < 16; i++, held++)
    {
        if (buttons & 1)
        {
            (*held)++;
            if (*held >= 26)
            {
                this->unk_30 |= 1;
                *held -= 8;
            }
        }
        else
        {
            *held = 0;
        }
        buttons >>= 1;
    }
    this->unk_32 = (this->unk_2c ^ this->unk_2e) & this->unk_2c;
    this->unk_34 = ~this->unk_2c & (this->unk_2c ^ this->unk_2e);
}

void Rng::SetSeed(WORD seed)
{
    this->seed = seed;
}

void Rng::ResetGenerated()
{
    this->generated = 0;
}

int ReplayManager::RecordOnUpdate2(ReplayManager *mgr)
{
    int i;
    GameConfig *config;

    mgr->frameEvents = 0;
    mgr->seed = g_Rng.GetSeed();
    g_Rng.ResetGenerated();
    if (g_GameManager.unk_130)
    {
        mgr->frameEvents |= 0x100;
    }
    g_GameManager.unk_130 = 0;
    g_PlayerInputs[0].unk_2e = g_PlayerInputs[0].unk_2c;
    g_PlayerInputs[0].unk_2c = g_PlayerInputs[0].unk_0;
    g_PlayerInputs[1].unk_2e = g_PlayerInputs[1].unk_2c;
    g_PlayerInputs[1].unk_2c = g_PlayerInputs[1].unk_0;
    g_PlayerInputs[2].unk_2e = g_PlayerInputs[2].unk_2c;
    g_PlayerInputs[2].unk_2c = g_PlayerInputs[2].unk_0;
    g_PlayerInputs[0].UpdateRepeat2();
    g_PlayerInputs[1].UpdateRepeat2();
    g_PlayerInputs[2].UpdateRepeat2();
    for (i = 0, config = g_GameManager.config; i < 2; i++)
    {
        if (config != NULL && config->unk_b4[i] == 1)
        {
            if (g_PlayerInputs[i].IsHeld2(1))
            {
                g_PlayerInputs[i].unk_2a++;
                if (g_PlayerInputs[i].unk_2a >= 8)
                {
                    g_PlayerInputs[i].unk_2a = 8;
                }
            }
            else
            {
                g_PlayerInputs[i].unk_2a = 0;
            }
        }
    }
    return 1;
}

int ReplayManager::RecordOnUpdate(ReplayManager *mgr)
{
    DWORD flags = g_GameManager.stateFlags;

    if (!(flags & 4))
    {
        return 1;
    }
    if (g_App.GetNoScore())
    {
        return 1;
    }
    if (flags & 0x200)
    {
        if (mgr->framesAfterStageClear >= 3)
        {
            return 1;
        }
        mgr->framesAfterStageClear++;
    }
    if (g_GameManager.sides[0].unk_28 == 0)
    {
        *mgr->inputCursors[0] = g_PlayerInputs[0].unk_2c;
    }
    if (g_GameManager.sides[1].unk_28 == 0)
    {
        *mgr->inputCursors[1] = g_PlayerInputs[1].unk_2c;
    }
    *mgr->inputCursors[2] = g_PlayerInputs[2].unk_2c;
    mgr->curChunk->count++;
    mgr->inputCursors[0]++;
    mgr->inputCursors[1]++;
    mgr->inputCursors[2]++;
    if (mgr->frame % 30 == 0)
    {
        *mgr->extraCursor = g_App.roundedFps;
        mgr->extraCursor++;
    }
    if (mgr->curChunk->count >= 0xe0e)
    {
        mgr->curChunk->next =
            (ReplayChunk *)g_MemoryTracker.Register(new ReplayChunk, sizeof(ReplayChunk), "ReplayBufLinkInf");
        mgr->curChunk = mgr->curChunk->next;
        mgr->inputCursors[0] = (WORD *)g_MemoryTracker.Alloc(0x1c20, "rep data");
        mgr->inputCursors[1] = (WORD *)g_MemoryTracker.Alloc(0x1c20, "rep data");
        mgr->inputCursors[2] = (WORD *)g_MemoryTracker.Alloc(0x1c20, "rep data");
        mgr->extraCursor = (BYTE *)g_MemoryTracker.Alloc(0x79, "rep data");
        mgr->curChunk->inputs[0] = mgr->inputCursors[0];
        mgr->curChunk->inputs[1] = mgr->inputCursors[1];
        mgr->curChunk->inputs[2] = mgr->inputCursors[2];
        mgr->curChunk->extra = mgr->extraCursor;
        mgr->curChunk->next = NULL;
        mgr->curChunk->count = 0;
        memset(mgr->inputCursors[0], 0, 0x1c20);
        memset(mgr->inputCursors[1], 0, 0x1c20);
        memset(mgr->inputCursors[2], 0, 0x1c20);
        memset(mgr->extraCursor, 0, 0x79);
    }
    mgr->frame++;
    return 1;
}

int ReplayManager::PlayOnUpdate2(ReplayManager *mgr)
{
    if ((g_GameManager.stateFlags & 4) && !(g_GameManager.stateFlags & 0x200) && g_GameManager.gui->IsMsgRunning() &&
        mgr->frame % 3 != 2)
    {
        return 6;
    }
    return 1;
}

int ReplayManager::PlayOnUpdate(ReplayManager *mgr)
{
    int i;
    BYTE *types;

    if ((g_GameManager.stateFlags & 4) && !(g_GameManager.stateFlags & 0x200))
    {
        g_PlayerInputs[0].unk_2e = g_PlayerInputs[0].unk_2c;
        g_PlayerInputs[1].unk_2e = g_PlayerInputs[1].unk_2c;
        g_PlayerInputs[2].unk_2e = g_PlayerInputs[2].unk_2c;
        g_PlayerInputs[0].unk_2c = *mgr->inputCursors[0];
        g_PlayerInputs[1].unk_2c = *mgr->inputCursors[1];
        g_PlayerInputs[2].unk_2c = *mgr->inputCursors[2];
        g_PlayerInputs[0].UpdateRepeat2();
        g_PlayerInputs[1].UpdateRepeat2();
        g_PlayerInputs[2].UpdateRepeat2();
        types = g_GameManager.config->unk_b4;
        for (i = 0; i < 2; i++, types++)
        {
            if (*types == 1)
            {
                if (g_PlayerInputs[i].IsHeld2(1))
                {
                    g_PlayerInputs[i].unk_2a++;
                    if (g_PlayerInputs[i].unk_2a >= 8)
                    {
                        g_PlayerInputs[i].unk_2a = 8;
                    }
                }
                else
                {
                    g_PlayerInputs[i].unk_2a = 0;
                }
            }
        }
        mgr->inputCursors[0]++;
        mgr->inputCursors[1]++;
        mgr->inputCursors[2]++;
        if (mgr->frame % 30 == 0)
        {
            g_App.roundedFps = (char)mgr->extraCursor[1] & 0x7f;
            g_App.fpsWarning = (char)mgr->extraCursor[1] >> 7;
            mgr->extraCursor++;
        }
        mgr->frame++;
    }
    return 1;
}

ReplayHeader *ValidateReplay(void *data, int size)
{
    BYTE *bytes;
    int sum;
    BYTE b;
    ReplayHeader *file;
    int i;
    ReplayHeader *decoded;

    file = (ReplayHeader *)data;
    if (file == NULL)
    {
        goto fail;
    }
    if (*(DWORD *)file->magic != *(DWORD *)"T9RP")
    {
        goto fail;
    }
    if (file->version != 2)
    {
        goto fail;
    }
    if (file->unk_6 != 0)
    {
        goto fail;
    }
    bytes = (BYTE *)&file->compressedSize;
    b = file->key;
    for (i = 0; i < file->size - 0x18; i++, bytes++)
    {
        *bytes -= b;
        b += 7;
    }
    sum = 0x3f000318;
    for (i = 0; i < file->size - 0x15; i++)
    {
        sum += (&file->key)[i];
    }
    if (sum != file->checksum)
    {
        goto fail;
    }
    decoded = (ReplayHeader *)g_MemoryTracker.Alloc(file->dataSize + 0xc0 + (size - file->size), GLOBAL_H);
    memcpy(decoded, data, 0xc0);
    LzssDecompress((BYTE *)file + 0xc0, file->compressedSize, (BYTE *)decoded + 0xc0, file->dataSize);
    memcpy((BYTE *)decoded + file->dataSize + 0xc0, (BYTE *)data + file->size, size - file->size);
    if (g_App.CheckVersionFile(decoded->exeVersion, decoded->exeSize, decoded->exeChecksum) != 0)
    {
        goto failDecoded;
    }
    for (i = 0; i < 10; i++)
    {
        if (decoded->unk_20[0][i] != NULL)
        {
            decoded->unk_20[0][i] = (BYTE *)decoded->unk_20[0][i] + (int)decoded;
        }
        if (decoded->unk_20[1][i] != NULL)
        {
            decoded->unk_20[1][i] = (BYTE *)decoded->unk_20[1][i] + (int)decoded;
        }
        if (decoded->unk_20[2][i] != NULL)
        {
            decoded->unk_20[2][i] = (BYTE *)decoded->unk_20[2][i] + (int)decoded;
        }
        if (decoded->extras[i] != NULL)
        {
            decoded->extras[i] = (BYTE *)decoded->extras[i] + (int)decoded;
        }
    }
    g_MemoryTracker.Free(data);
    return decoded;

fail:
    g_MemoryTracker.Free(data);
    return NULL;

failDecoded:
    g_MemoryTracker.Free(data);
    g_MemoryTracker.Free(decoded);
    return NULL;
}

ReplayInfo::ReplayInfo()
{
}

int ReplayManager::PlayAddedCallback(ReplayManager *mgr)
{
    int size;
    int idx;
    int i;
    ReplayRound *round0;
    ReplayRound *round1;
    ReplayRound *round2;
    BYTE *extra;

    mgr->frame = 0;
    if (mgr->data == NULL)
    {
        mgr->data = (ReplayHeader *)LoadFileData(mgr->path, &size, !(g_GameManager.stateFlags >> 1 & 1));
        mgr->data = ValidateReplay(mgr->data, size);
        if (mgr->data == NULL)
        {
            return -1;
        }
        mgr->unk_110 = NULL;
    }
    idx = g_GameManager.unk_fc;
    if (mgr->data->unk_20[idx] == NULL)
    {
        return -1;
    }
    if (g_GameManager.IsMode0() || g_GameManager.IsMode1())
    {
        for (i = 0; i < idx; i++)
        {
            g_GameManager.unk_d8[((ReplayRound *)mgr->data->unk_20[1][i])->character] = 1;
        }
    }
    round0 = (ReplayRound *)mgr->data->unk_20[0][idx];
    round1 = (ReplayRound *)mgr->data->unk_20[1][idx];
    round2 = (ReplayRound *)mgr->data->unk_20[2][idx];
    extra = (BYTE *)mgr->data->extras[idx];
    g_GameManager.sides[0].character = round0->character;
    g_GameManager.sides[1].character = round1->character;
    g_GameManager.sides[0].unk_28 = round0->unk_7;
    g_GameManager.sides[1].unk_28 = round1->unk_7;
    g_GameManager.sides[0].unk_2c = round0->unk_a;
    g_GameManager.sides[1].unk_2c = round1->unk_a;
    g_GameManager.stage = round0->stage;
    g_GameManager.unk_138 = round0->unk_9;
    g_GameManager.difficulty = mgr->data->info.difficulty;
    g_GameManager.initialLives[0] = mgr->data->unk_1e6[0];
    g_GameManager.initialLives[1] = mgr->data->unk_1e6[1];
    g_GameManager.sides[0].unk_30 = mgr->data->unk_1e8[0];
    g_GameManager.sides[1].unk_30 = mgr->data->unk_1e8[1];
    g_GameManager.sides[0].stats->score = round0->score;
    g_GameManager.sides[1].stats->score = round1->score;
    g_GameManager.sides[0].stats->guiScore = round0->score;
    g_GameManager.sides[1].stats->guiScore = round1->score;
    g_GameManager.sides[0].stats->lives = round0->lives;
    g_GameManager.sides[1].stats->lives = round1->lives;
    g_GameManager.sides[0].stats->extendsGiven = round0->extendsGiven;
    mgr->inputCursors[0] = (WORD *)(round0 + 1);
    mgr->inputCursors[1] = (WORD *)(round1 + 1);
    mgr->inputCursors[2] = (WORD *)(round2 + 1);
    mgr->extraCursor = extra;
    *g_GameManager.config = mgr->data->info.config;
    g_Rng.SetSeed(round0->rngSeed);
    mgr->framesAfterStageClear = 0;
    return 0;
}




static int FreeResources(ReplayManager *mgr)
{
    if (mgr->calcElem2 != NULL)
    {
        g_Chain.Remove(mgr->calcElem2);
        mgr->calcElem2 = NULL;
    }
    if (mgr->calcElem3 != NULL)
    {
        g_Chain.Remove(mgr->calcElem3);
        mgr->calcElem3 = NULL;
    }
    g_MemoryTracker.Free(mgr->data);
    if (mgr->unk_110 != NULL)
    {
        g_MemoryTracker.Free(mgr->unk_110);
    }
    return 0;
}

void ReplayManager::Release(ReplayManager *mgr)
{
    ReplayChunk *chunk;
    ReplayChunk *cur;
    int i;
    int j;

    if (mgr == NULL)
    {
        return;
    }
    for (i = 0; i < 10; i++)
    {
        for (chunk = &mgr->chunks[i]; chunk != NULL;)
        {
            cur = chunk;
            if (chunk->inputs[0] != NULL)
            {
                g_MemoryTracker.Free(chunk->inputs[0]);
            }
            if (chunk->inputs[1] != NULL)
            {
                g_MemoryTracker.Free(chunk->inputs[1]);
            }
            if (chunk->inputs[2] != NULL)
            {
                g_MemoryTracker.Free(chunk->inputs[2]);
            }
            if (chunk->extra != NULL)
            {
                g_MemoryTracker.Free(chunk->extra);
            }
            chunk = chunk->next;
            memset(cur, 0, sizeof(ReplayChunk));
            if (cur != &mgr->chunks[i])
            {
                g_MemoryTracker.Free(cur);
            }
        }
        if (!g_GameManager.IsReplay())
        {
            for (j = 0; j < 3; j++)
            {
                if (mgr->data->unk_20[j][i] != NULL)
                {
                    g_MemoryTracker.Free(mgr->data->unk_20[j][i]);
                }
                mgr->data->unk_20[j][i] = NULL;
            }
            if (mgr->data->extras[i] != NULL)
            {
                g_MemoryTracker.Free(mgr->data->extras[i]);
            }
            mgr->data->extras[i] = NULL;
        }
    }
    FreeResources(mgr);
    g_Chain.Remove(mgr->calcElem);
    delete mgr;
}

void ReplayManager::RemoveChain(ReplayManager *mgr)
{
    if (mgr != NULL)
    {
        FreeResources(mgr);
        g_Chain.Remove(mgr->calcElem);
        delete mgr;
    }
}

ReplayHeader::ReplayHeader()
{
}

int ReplayManager::SaveReplay(ReplayManager *mgr, char *path, char *name)
{
    BYTE *dp2;
    BYTE kbVal;
    BYTE *bpCur;
    int checksum;
    ReplayUserHeader user;
    char userInfo[0x400];
    BYTE *rpyBytes;
    ReplayChunk *chunk;
    int chunkSize;
    char *p2;
    int allocSize;
    int mode;
    int j;
    int i;

    if (mgr != NULL && path != NULL)
    {
        ReplayHeader hX;
        int offsets;

        allocSize = sizeof(ReplayHeader) + 0x10000;
        for (i = 0; i < 10; i++)
        {
            if (hX.unk_20[0][i] != NULL)
            {
                allocSize += 0x60;
                for (chunk = &mgr->chunks[i]; chunk != NULL; chunk = chunk->next)
                {
                    allocSize += 0x54d9;
                }
            }
        }
        rpyBytes = (BYTE *)g_MemoryTracker.Alloc(allocSize, "rep tmp");
        hX = *mgr->data;
        offsets = sizeof(ReplayHeader);
        for (j = 0; j < 3; j++)
        {
            for (i = 0; i < 10; i++)
            {
                if (hX.unk_20[j][i] != NULL)
                {
                    memcpy(rpyBytes + offsets - 0xc0, hX.unk_20[j][i], 0x20);
                    hX.unk_20[j][i] = (void *)offsets;
                    offsets += 0x20;
                    for (chunk = &mgr->chunks[i]; chunk != NULL; chunk = chunk->next)
                    {
                        chunkSize = chunk->count * 2;
                        memcpy(rpyBytes + offsets - 0xc0, chunk->inputs[j], chunkSize);
                        offsets += chunkSize;
                    }
                }
            }
        }
        for (i = 0; i < 10; i++)
        {
            if (hX.extras[i] != NULL)
            {
                hX.extras[i] = (void *)offsets;
                for (chunk = &mgr->chunks[i]; chunk != NULL; chunk = chunk->next)
                {
                    chunkSize = chunk->count / 30;
                    memcpy(rpyBytes + offsets - 0xc0, chunk->extra, chunkSize);
                    offsets += chunkSize;
                }
            }
        }
        memset(userInfo, 0, sizeof(userInfo));
        user.magic = *(DWORD *)"USER";
        user.type = 0;
        p2 = userInfo;
        p2 = FormatAndAdvance(p2, "プレイヤー名\t%s\r\n", name);
        p2 = FormatAndAdvance(p2, "プレイ時刻\t%s\r\n", g_Hscr.date);
        p2 = FormatAndAdvance(p2, "難易度\t\t%s\r\n", g_ReplayDifficultyNames[g_GameManager.difficulty]);
        mode = g_GameManager.mode;
        if (mode == 2)
        {
            mode = g_GameManager.unk_114 + 2;
        }
        p2 = FormatAndAdvance(p2, "モード\t\t%s\r\n", g_ReplayModeNames[mode]);
        if (!g_GameManager.IsMode2())
        {
            p2 = FormatAndAdvance(p2, "使用キャラ\t\t%s\r\n", g_ReplayCharacterNames[g_GameManager.sides[0].character]);
        }
        else
        {
            p2 = FormatAndAdvance(p2, "対戦キャラ\t\t%s vs %s\r\n", g_ReplayCharacterNames[g_GameManager.sides[0].character],
                                  g_ReplayCharacterNames[g_GameManager.sides[1].character]);
        }
        p2 = FormatAndAdvance(p2, "初期体力\t\t%1.1f vs %1.1f\r\n", g_GameManager.initialLives[0] * 0.5f,
                              g_GameManager.initialLives[1] * 0.5f);
        p2 = FormatAndAdvance(p2, "ゲームのバージョン\t%s\r\n", "東方花映塚 ver 1.50a");
        user.size = strlen(userInfo) + 1 + 0xc;
        user.size += user.size & 1;
        hX.isSaved = 1;
        strcpy(hX.info.name, name);
        strcpy(hX.info.date, g_Hscr.date);
        hX.key = g_Rng.GetRandomU16InRange(0x80) + 0x40;
        hX.replayFormatVersion = 30;
        hX.info.randomByte = g_Rng.GetRandomU16InRange(0x100);
        hX.randomFill = g_Rng.GetRandomU16InRange(0x100);
        memcpy(rpyBytes, &hX.info, sizeof(ReplayHeader) - 0xc0);
        hX.dataSize = offsets - 0xc0;
        {
            BYTE *compressedData;
            int compSize;
            HANDLE file;
            DWORD writtenSize;
            compressedData = (BYTE *)LzssCompress(rpyBytes, hX.dataSize, &hX.compressedSize);
            g_MemoryTracker.Free(rpyBytes);
            compSize = hX.compressedSize;
            bpCur = &hX.key;
            checksum = 0x3f000318;
            for (i = 0; i < 0xabU; i++, bpCur++)
            {
                checksum += *bpCur;
            }
            for (i = 0; i < compSize; i++)
            {
                checksum += compressedData[i];
            }
            hX.checksum = checksum;
            dp2 = (BYTE *)&hX.compressedSize;
            kbVal = hX.key;
            for (i = 0; i < 0xa8U; i++, dp2++)
            {
                *dp2 += kbVal;
                kbVal += 7;
            }
            dp2 = compressedData;
            for (i = 0; i < compSize; i++, dp2++)
            {
                *dp2 += kbVal;
                kbVal += 7;
            }
            hX.size = compSize + 0xc0;
            writtenSize = 0;
            file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (file != INVALID_HANDLE_VALUE)
            {
                WriteFile(file, &hX, 0xc0, &writtenSize, NULL);
                WriteFile(file, compressedData, compSize, &writtenSize, NULL);
                WriteFile(file, &user, 0xc, &writtenSize, NULL);
                WriteFile(file, userInfo, user.size - 0xc, &writtenSize, NULL);
                CloseHandle(file);
                GlobalFree(compressedData);
            }
        }
    }
    return 0;
}







int ReplayManager::RecordAddedCallback(ReplayManager *mgr)
{
    int idx;
    int i;
    ReplayRound *round0;
    ReplayRound *round1;
    ReplayRound *round2;
    BYTE *extra;
    ReplayChunk *chunk;
    ReplayChunk *cur;
    ReplayHeader *hdr;

    mgr->frame = 0;
    mgr->unk_110 = NULL;
    hdr = mgr->data;
    if (hdr == NULL)
    {
        mgr->data = (ReplayHeader *)g_MemoryTracker.Register(new ReplayHeader, sizeof(ReplayHeader),
                                                                  "ReplayFileHeaderInf");
        hdr = mgr->data;
        memset(hdr, 0, sizeof(ReplayHeader));
        hdr->isSaved = 0;
        hdr->unk_6 = 0;
        *(DWORD *)hdr->magic = *(DWORD *)"T9RP";
        hdr->version = 2;
        hdr->info.version = 0x150;
        hdr->info.versionLetter = 'a';
        strcpy(hdr->exeVersion, "0150a");
        hdr->exeSize = g_App.exeSize;
        hdr->exeChecksum = g_App.exeChecksum;
        hdr->info.difficulty = g_GameManager.difficulty;
        hdr->mode = g_GameManager.mode;
        hdr->unk_1e5 = g_GameManager.unk_114;
        hdr->unk_1e6[0] = g_GameManager.initialLives[0];
        hdr->unk_1e6[1] = g_GameManager.initialLives[1];
        mgr->data->unk_1e8[0] = g_GameManager.sides[0].unk_30;
        mgr->data->unk_1e8[1] = g_GameManager.sides[1].unk_30;
        *(DWORD *)hdr->info.name = *(DWORD *)"NO NAME";
        hdr->info.config = *g_GameManager.config;
        for (i = 0; i < 10; i++)
        {
            hdr->unk_20[0][i] = NULL;
            hdr->unk_20[1][i] = NULL;
            hdr->unk_20[2][i] = NULL;
            hdr->extras[i] = NULL;
        }
    }
    idx = g_GameManager.unk_fc;
    if (hdr->unk_20[0][idx] != NULL)
    {
        g_MemoryTracker.Free(hdr->unk_20[0][idx]);
    }
    if (hdr->unk_20[1][idx] != NULL)
    {
        g_MemoryTracker.Free(hdr->unk_20[1][idx]);
    }
    if (hdr->unk_20[2][idx] != NULL)
    {
        g_MemoryTracker.Free(hdr->unk_20[2][idx]);
    }
    if (hdr->extras[idx] != NULL)
    {
        g_MemoryTracker.Free(hdr->extras[idx]);
    }
    round0 = (ReplayRound *)g_MemoryTracker.Register(operator new(sizeof(ReplayRound)), sizeof(ReplayRound), "ReplayFrameDataStartInf");
    hdr->unk_20[0][idx] = round0;
    round1 = (ReplayRound *)g_MemoryTracker.Register(operator new(sizeof(ReplayRound)), sizeof(ReplayRound), "ReplayFrameDataStartInf");
    hdr->unk_20[1][idx] = round1;
    round2 = (ReplayRound *)g_MemoryTracker.Register(operator new(sizeof(ReplayRound)), sizeof(ReplayRound), "ReplayFrameDataStartInf");
    hdr->unk_20[2][idx] = round2;
    extra = (BYTE *)g_MemoryTracker.Register(operator new(sizeof(BYTE)), sizeof(BYTE), "ReplayFPSTblStartInf");
    hdr->extras[idx] = extra;
    memset(round0, 0, sizeof(ReplayRound));
    memset(round1, 0, sizeof(ReplayRound));
    memset(round2, 0, sizeof(ReplayRound));
    *extra = 0;
    round2->rngSeed = g_GameManager.rngSeed;
    round1->rngSeed = g_GameManager.rngSeed;
    round0->rngSeed = g_GameManager.rngSeed;
    round0->character = g_GameManager.sides[0].character;
    round1->character = g_GameManager.sides[1].character;
    round0->unk_7 = g_GameManager.sides[0].unk_28;
    round1->unk_7 = g_GameManager.sides[1].unk_28;
    round0->unk_a = g_GameManager.sides[0].unk_2c;
    round1->unk_a = g_GameManager.sides[1].unk_2c;
    round0->lives = g_GameManager.sides[0].stats->lives;
    round1->lives = g_GameManager.sides[1].stats->lives;
    round0->score = g_GameManager.sides[0].stats->score;
    round1->score = g_GameManager.sides[1].stats->score;
    round0->stage = g_GameManager.stage;
    round0->unk_9 = g_GameManager.unk_138;
    round0->extendsGiven = g_GameManager.sides[0].stats->extendsGiven;
    mgr->inputCursors[0] = (WORD *)g_MemoryTracker.Alloc(0x1c20, "rep data");
    mgr->inputCursors[1] = (WORD *)g_MemoryTracker.Alloc(0x1c20, "rep data");
    mgr->inputCursors[2] = (WORD *)g_MemoryTracker.Alloc(0x1c20, "rep data");
    mgr->extraCursor = (BYTE *)g_MemoryTracker.Alloc(0x79, "rep data");
    mgr->curChunk = &mgr->chunks[idx];
    for (chunk = mgr->curChunk; chunk != NULL;)
    {
        cur = chunk;
        if (chunk->inputs[0] != NULL)
        {
            g_MemoryTracker.Free(chunk->inputs[0]);
        }
        if (chunk->inputs[1] != NULL)
        {
            g_MemoryTracker.Free(chunk->inputs[1]);
        }
        if (chunk->inputs[2] != NULL)
        {
            g_MemoryTracker.Free(chunk->inputs[2]);
        }
        if (chunk->extra != NULL)
        {
            g_MemoryTracker.Free(chunk->extra);
        }
        chunk = chunk->next;
        memset(cur, 0, sizeof(ReplayChunk));
        if (cur != &mgr->chunks[idx])
        {
            g_MemoryTracker.Free(cur);
        }
    }
    mgr->curChunk->inputs[0] = mgr->inputCursors[0];
    mgr->curChunk->inputs[1] = mgr->inputCursors[1];
    mgr->curChunk->inputs[2] = mgr->inputCursors[2];
    mgr->curChunk->extra = mgr->extraCursor;
    mgr->curChunk->next = NULL;
    mgr->curChunk->count = 0;
    memset(mgr->inputCursors[0], 0, 0x1c20);
    memset(mgr->inputCursors[1], 0, 0x1c20);
    memset(mgr->inputCursors[2], 0, 0x1c20);
    memset(mgr->extraCursor, 0, 0x79);
    return 0;
}

ReplayManager *ReplayManager::RegisterChain(int mode, char *path)
{
    ReplayManager *mgr =
        (ReplayManager *)g_MemoryTracker.Register(new ReplayManager(), sizeof(ReplayManager), "ReplayInf");

    memset(mgr, 0, sizeof(ReplayManager));
    mgr->mode = mode;
    mgr->data = NULL;
    switch (mode)
    {
    case 0:
        mgr->calcElem = g_Chain.CreateElem((ChainCallback)ReplayManager::RecordOnUpdate);
        mgr->calcElem->addedCallback = (ChainCallback)ReplayManager::RecordAddedCallback;
        mgr->calcElem->arg = mgr;
        if (g_Chain.AddToCalcChain(mgr->calcElem, 0x1a) != 0)
        {
            return NULL;
        }
        mgr->calcElem2 = NULL;
        mgr->calcElem3 = g_Chain.CreateElem((ChainCallback)ReplayManager::RecordOnUpdate2);
        mgr->calcElem3->arg = mgr;
        g_Chain.AddToCalcChain(mgr->calcElem3, 7);
        ReplayManager::RecordOnUpdate2(mgr);
        break;
    case 1:
        mgr->path = path;
        mgr->calcElem = g_Chain.CreateElem((ChainCallback)ReplayManager::PlayOnUpdate);
        mgr->calcElem->addedCallback = (ChainCallback)ReplayManager::PlayAddedCallback;
        mgr->calcElem->arg = mgr;
        if (g_Chain.AddToCalcChain(mgr->calcElem, 6) != 0)
        {
            return NULL;
        }
        mgr->calcElem2 = g_Chain.CreateElem((ChainCallback)ReplayManager::PlayOnUpdate2);
        mgr->calcElem2->arg = mgr;
        g_Chain.AddToCalcChain(mgr->calcElem2, 0x1b);
        mgr->calcElem3 = NULL;
        break;
    }
    return mgr;
}

void ReplayManager::RunAddedCallback(ReplayManager *mgr, int mode)
{
    switch (mode)
    {
    case 0:
        ReplayManager::RecordAddedCallback(mgr);
        break;
    case 1:
        ReplayManager::PlayAddedCallback(mgr);
        break;
    }
}
