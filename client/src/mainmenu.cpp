
#include <direct.h>

#include "mainmenu.h"
#include "ascii.h"
#include "midi.h"
#include "scoredat.h"
#include "sound.h"
#ifdef TH09_NATIVE_NETPLAY
#include "NetGame.h"
#include "CoopGame.h"
#endif



static MainMenu *g_MainMenu;

MusicTune::MusicTune()
{
    memset(this, 0, sizeof(MusicTune));
}

void MainMenu::PlaySoundNow(int idx, int arg)
{
    g_SoundPlayer.QueueSound(idx, arg);
    g_SoundPlayer.PlayQueuedSounds();
}

void MainMenu::ReassignButton(ControllerMapping *mapping, short button, short replacement, int unused)
{
    if (mapping->shootButton == button)
    {
        mapping->shootButton = replacement;
    }
    if (mapping->bombButton == button)
    {
        mapping->bombButton = replacement;
    }
    if (mapping->focusButton == button)
    {
        mapping->focusButton = replacement;
    }
    if (mapping->upButton == button)
    {
        mapping->upButton = replacement;
    }
    if (mapping->downButton == button)
    {
        mapping->downButton = replacement;
    }
    if (mapping->leftButton == button)
    {
        mapping->leftButton = replacement;
    }
    if (mapping->rightButton == button)
    {
        mapping->rightButton = replacement;
    }
    if (mapping->menuButton == button)
    {
        mapping->menuButton = replacement;
    }
    if (mapping->skipButton == button)
    {
        mapping->skipButton = replacement;
    }
}

int MainMenu::ShowButtonDigits(Sprite *vm, short value)
{
    if (value < 0)
    {
        vm[0].flags &= ~2;
        vm[1].flags &= ~2;
    }
    else
    {
        this->anm->SetSprite(&vm[0], vm[0].baseSpriteIndex + value / 10 * 2);
        this->anm->SetSprite(&vm[1], vm[1].baseSpriteIndex + value % 10 * 2);
        vm[0].flags |= 2;
        vm[1].flags |= 2;
    }
    return 0;
}

void MainMenu::SkipClosedEntries(int idx, int delta, char *entries, int count)
{
    if (g_Network->unk_a8 != 0)
    {
        return;
    }
    int *cursor = &this->cursors[idx];
    while (!g_PlayRecord.openEntries[entries[*cursor]])
    {
        *cursor += delta;
        if (*cursor < 0)
        {
            *cursor += count;
        }
        if (*cursor >= count)
        {
            *cursor -= count;
        }
    }
}

void MainMenu::SkipClosedEntries2(int idx, int delta, char *entries, int count)
{
    if (g_Network->unk_a8 != 0)
    {
        return;
    }
    int *cursor = &this->cursors[idx];
    while (!g_PlayRecord.openEntries2[entries[*cursor]])
    {
        *cursor += delta;
        if (*cursor < 0)
        {
            *cursor += count;
        }
        if (*cursor >= count)
        {
            *cursor -= count;
        }
    }
}

void MainMenu::SkipClosedEntries3(int idx, int delta, char *entries, int count)
{
    if (g_Network->unk_a8 != 0)
    {
        return;
    }
    int *cursor = &this->cursors[idx];
    while (!g_PlayRecord.openEntries3[entries[*cursor]])
    {
        *cursor += delta;
        if (*cursor < 0)
        {
            *cursor += count;
        }
        if (*cursor >= count)
        {
            *cursor -= count;
        }
    }
}

void MainMenu::ShowHalves(int left, int right)
{
    this->anm->SetSprite(&this->sprites[142], left == 0 ? 0xc5 : left == 1 ? 0xc4 : 0xc3);
    left -= 2;
    this->anm->SetSprite(&this->sprites[143], left <= 0 ? 0xc5 : left == 1 ? 0xc4 : 0xc3);
    left -= 2;
    this->anm->SetSprite(&this->sprites[144], left <= 0 ? 0xc5 : left == 1 ? 0xc4 : 0xc3);
    left -= 2;
    this->anm->SetSprite(&this->sprites[145], left <= 0 ? 0xc5 : left == 1 ? 0xc4 : 0xc3);
    left -= 2;
    this->anm->SetSprite(&this->sprites[146], left <= 0 ? 0xc5 : left == 1 ? 0xc4 : 0xc3);
    this->anm->SetSprite(&this->sprites[148], right == 0 ? 0xc5 : right == 1 ? 0xc4 : 0xc3);
    int n = right - 2;
    this->anm->SetSprite(&this->sprites[149], n <= 0 ? 0xc5 : n == 1 ? 0xc4 : 0xc3);
    n -= 2;
    this->anm->SetSprite(&this->sprites[150], n <= 0 ? 0xc5 : n == 1 ? 0xc4 : 0xc3);
    n -= 2;
    this->anm->SetSprite(&this->sprites[151], n <= 0 ? 0xc5 : n == 1 ? 0xc4 : 0xc3);
    n -= 2;
    this->anm->SetSprite(&this->sprites[152], n <= 0 ? 0xc5 : n == 1 ? 0xc4 : 0xc3);
}

void MidiPlayer::Restart(int slot)
{
    this->Stop();
    this->ParseFile(slot);
    this->Play();
}

void App::ResetClockDriftCounters()
{
    this->unk_77c = 0;
    this->unk_784 = 0;
    this->unk_790 = 0;
    this->unk_788 = 0;
    this->unk_78c = 0;
}

void Sprite::SetMoveInterp(int frames, BYTE easing, D3DXVECTOR3 *from, D3DXVECTOR3 *to)
{
    this->interpDuration[0].current = 0;
    this->interpDuration[0].currentF = 0.0f;
    this->interpDuration[0].previous = -999999;
    Timer *frame = &this->interpFrame[0];
    frame->current = frames;
    frame->currentF = frames;
    frame->previous = -999999;
    this->posInterpEasing = easing;
    this->moveStart = *from;
    this->moveEnd = *to;
}

void Sprite::SetColorInterp(int frames, BYTE easing, D3DCOLOR from, D3DCOLOR to)
{
    this->interpDuration[1].current = 0;
    this->interpDuration[1].currentF = 0.0f;
    this->interpDuration[1].previous = -999999;
    Timer *frame = &this->interpFrame[1];
    frame->current = frames;
    frame->currentF = frames;
    frame->previous = -999999;
    this->colorInterpEasing = easing;
    ((BYTE *)&this->colorTween.y)[2] = from >> 16 & 0xff;
    ((BYTE *)&this->colorTween.y)[1] = from >> 8 & 0xff;
    ((BYTE *)&this->colorTween.y)[0] = from & 0xff;
    ((BYTE *)&this->colorTween.z)[2] = to >> 16 & 0xff;
    ((BYTE *)&this->colorTween.z)[1] = to >> 8 & 0xff;
    ((BYTE *)&this->colorTween.z)[0] = to & 0xff;
}

void SoundPlayer::UnPause()
{
    this->QueueSoundCommand(7, 0, "UnPause");
}

int PlayRecord::GetClearFlags(char character, int difficulty)
{
    if (difficulty >= 0)
    {
        return this->clearFlags[character][difficulty];
    }
    return this->clearFlags[character][0] | this->clearFlags[character][1] | this->clearFlags[character][2] |
           this->clearFlags[character][3];
}

void AsciiText::SetScale(float x, float y)
{
    this->scaleY = x;
    this->scaleX = y;
}

int MainMenu::DrawState11()
{
    D3DXVECTOR3 pos;
    int i;
    int slot;
    ReplayHeader *replay;
    char buf[0x100];

    if (this->subState == 1)
    {
        pos.x = 32.0f;
        pos.y = 90.0f;
        pos.z = 0.0f;
        for (i = 0; i < 50; i++)
        {
            if (i == 25)
            {
                pos.x = 352.0f;
                pos.y = 90.0f;
            }
            g_AsciiText.color = i == this->cursor ? 0xffffffff : 0xff808080;
            if (i < 25)
            {
                g_AsciiText.AddFormat(&pos, "No.%.2d %8s %8s", i + 1,
                                      *(int *)this->replays[i].magic != 0 ? this->replays[i].info.name : "--------",
                                      *(int *)this->replays[i].magic != 0 ? this->replays[i].info.date : "--/--/--");
            }
            else
            {
                g_AsciiText.AddFormat(&pos, "%c%c%c%c  %8s %8s",
                                      *(int *)this->replays[i].magic != 0 ? this->replayPaths[i][15] : 'u',
                                      *(int *)this->replays[i].magic != 0 ? this->replayPaths[i][16] : 's',
                                      *(int *)this->replays[i].magic != 0 ? this->replayPaths[i][17] : 'e',
                                      *(int *)this->replays[i].magic != 0 ? this->replayPaths[i][18] : 'r',
                                      *(int *)this->replays[i].magic != 0 ? this->replays[i].info.name : "--------",
                                      *(int *)this->replays[i].magic != 0 ? this->replays[i].info.date : "--/--/--");
            }
            pos.y += 13.0f;
        }
    }
    else if (this->subState == 2)
    {
        replay = &this->replays[this->replaySlot];
        slot = this->replaySlot;
        pos.x = 32.0f;
        if (slot >= 25)
        {
            pos.x = 352.0f;
        }
        pos.y = slot * 13.0f + 90.0f;
        pos.z = 0.0f;
        D3DXVECTOR3 end(216.5f, 160.0f, 0.0f);
        if (this->inputLockout < 10)
        {
            pos = pos + (end - pos) * (float)this->inputLockout * 0.1f;
        }
        else
        {
            pos = end;
        }
        if (slot < 25)
        {
            g_AsciiText.AddFormat(&pos, "No.%.2d %8s %8s", slot + 1,
                                  *(int *)replay->magic != 0 ? replay->info.name : "--------",
                                  *(int *)replay->magic != 0 ? replay->info.date : "--/--/--");
        }
        else
        {
            g_AsciiText.AddFormat(&pos, "%c%c%c%c  %8s %8s",
                                  *(int *)replay->magic != 0 ? this->replayPaths[slot][15] : 'u',
                                  *(int *)replay->magic != 0 ? this->replayPaths[slot][16] : 's',
                                  *(int *)replay->magic != 0 ? this->replayPaths[slot][17] : 'e',
                                  *(int *)replay->magic != 0 ? this->replayPaths[slot][18] : 'r',
                                  *(int *)replay->magic != 0 ? replay->info.name : "--------",
                                  *(int *)replay->magic != 0 ? replay->info.date : "--/--/--");
        }
        pos.x = 221.0f;
        pos.y = 240.0f;
        if (this->replays[slot].mode == 0)
        {
            sprintf(buf, "%s  -%s-", g_ReplayMenuModeNames[0], g_ReplayMenuDifficultyNames[replay->info.difficulty]);
            pos.x = 320.0f - strlen(buf) * 4.5f;
            g_AsciiText.AddFormat(&pos, buf);
        }
        else if (this->replays[slot].mode == 1)
        {
            sprintf(buf, "%s", g_ReplayMenuModeNames[1]);
            pos.x = 320.0f - strlen(buf) * 4.5f;
            g_AsciiText.AddFormat(&pos, buf);
        }
        else if (this->replays[slot].mode == 2)
        {
            sprintf(buf, "%s  -%s-", g_ReplayMenuMatchNames[replay->unk_1e5],
                    g_ReplayMenuDifficultyNames[replay->info.difficulty]);
            pos.x = 320.0f - strlen(buf) * 4.5f;
            g_AsciiText.AddFormat(&pos, buf);
        }
        pos.x = 230.0f;
        pos.y = 270.0f;
        for (i = 0; i < 10; i++)
        {
            if (replay->unk_20[0][i] != NULL)
            {
                g_AsciiText.color = i == this->cursor ? 0xffffffff : 0xff808080;
                g_AsciiText.AddFormat(&pos, "%.8s vs %.8s",
                                      g_ReplayMenuCharacterNames[((ReplayRound *)replay->unk_20[0][i])->character],
                                      g_ReplayMenuCharacterNames[((ReplayRound *)replay->unk_20[1][i])->character]);
                pos.y += 13.0f;
            }
        }
    }
    g_AsciiText.color = 0xffffffff;
    g_AsciiText.highlight = 0;
    return 1;
}

int MainMenu::DrawState15()
{
    D3DXVECTOR3 pos(32.0f, 64.0f, 0.0f);
    int i;
    int row;
    int col;
    float scale;
    char str[16];

    if (this->subState == 2)
    {
        pos.y = 90.0f;
        for (i = 0; i < 25; i++)
        {
            g_AsciiText.color = i == this->cursor ? 0xffffffff : 0xff808080;
            g_AsciiText.AddFormat(&pos, "No.%.2d %.8s %8s", i + 1,
                                  *(int *)this->replays[i].magic != 0 ? this->replays[i].info.name : "--------",
                                  *(int *)this->replays[i].magic != 0 ? this->replays[i].info.date : "--/--/--");
            pos.y += 13.0f;
        }
    }
    else if (this->subState == 3 || this->subState == 4)
    {
        row = this->replaySlot;
        pos.y = row * 13.0f + 90.0f;
        D3DXVECTOR3 end = pos;
        end.x = end.y = 224.0f;
        if (this->inputLockout < 10)
        {
            pos = pos + (end - pos) * (float)this->inputLockout * 0.1f;
        }
        else
        {
            pos = end;
        }
        g_AsciiText.color = 0xffffffff;
        g_AsciiText.AddFormat(&pos, "No.%.2d %.8s %8s", row + 1, this->replayName, g_Hscr.date);
        pos.x += (this->replayNameCursor + 6) * 9;
        g_AsciiText.AddFormat(&pos, "%.8s", "_");
        if (this->subState == 4)
        {
            pos.z = 0.0f;
            pos.y = 320.0f;
            for (row = 0; row < 6; row++)
            {
                pos.x = 224.0f;
                for (col = 0; col < 16; col++)
                {
                    pos.x += 12.0f;
                    float offsetX = 0.0f;
                    float offsetY = 0.0f;
                    if (this->cursor == col + row * 16)
                    {
                        g_AsciiText.color = 0xffffffc0;
                        if (this->stateFrames % 64 < 32)
                        {
                            scale = this->stateFrames % 32 * 0.025f + 1.2f;
                        }
                        else
                        {
                            scale = 2.0f - this->stateFrames % 32 * 0.025f;
                        }
                        g_AsciiText.scaleY = scale;
                        g_AsciiText.scaleX = scale;
                        offsetY = offsetX = (scale - 1.0f) * -8.0f;
                    }
                    else
                    {
                        g_AsciiText.color = 0xc0c0c0c0;
                        g_AsciiText.scaleY = 1.0f;
                        g_AsciiText.scaleX = 1.0f;
                    }
                    D3DXVECTOR3 charPos = D3DXVECTOR3(offsetX, offsetY, 0.0f) + pos;
                    str[0] = g_NameAlphabet[row * 16 + col];
                    str[1] = 0;
                    if (row == 5)
                    {
                        if (col == 14)
                        {
                            str[0] = 0x7f;
                        }
                        else if (col == 15)
                        {
                            str[0] = 0x80;
                        }
                    }
                    g_AsciiText.AddString(&charPos, str);
                }
                pos.y += 16.0f;
            }
        }
    }
    g_AsciiText.color = 0xffffffff;
    g_AsciiText.highlight = 0;
    g_AsciiText.scaleY = 1.0f;
    g_AsciiText.scaleX = 1.0f;
    return 1;
}

int MainMenu::DrawState13()
{
    D3DXVECTOR3 pos;
    int row;
    int col;
    int mask;
    int highlight;
    int alpha;
    float scale;
    char str[16];
    Hscr (*scores)[5];

    scores = g_HighScores[this->scoreCharacter];
    if (this->state == 13)
    {
        mask = 0xff;
        highlight = 99;
    }
    else if (this->state == 14)
    {
        mask = 1 << g_GameManager.difficulty;
        highlight = this->scoreRank;
    }
    alpha = this->inputLockout < 30 ? this->inputLockout * 255 / 30 : 255;
    g_AsciiText.color = mask & 1 ? (alpha << 24) | 0xd0ffd0 : (alpha << 24) | 0x808080;
    pos.x = 102.0f;
    pos.y = 80.0f;
    pos.z = 0.0f;
    g_AsciiText.AddFormat(&pos, "Easy Ranking");
    pos.y += 16.0f;


    {
        int i;

        for (i = 0; i < 5; i++)
        {
            if (mask & 1)
            {
                g_AsciiText.color = highlight == i ? (alpha << 24) | 0xfff0ef : (alpha << 24) | 0xc0f0ff;
            }
            else
            {
                g_AsciiText.color = (alpha << 24) | 0x809090;
            }
            if (mask & 1 && highlight == i)
            {
                pos.x += (this->replayNameCursor + 4) * 9;
                g_AsciiText.AddFormat(&pos, "%.8s", "_");
                pos.x -= (this->replayNameCursor + 4) * 9;
            }
            g_AsciiText.AddFormat(&pos, "%s %.8s %.9d%d", g_ScoreRankNames[i], (*scores)[i].name, (*scores)[i].score,
                                  (char)(*scores)[i].unk_2b);
            pos.y += 16.0f;
        }
    }
    scores++;
    g_AsciiText.color = mask & 2 ? (alpha << 24) | 0xd0ffd0 : (alpha << 24) | 0x808080;
    pos.x = 358.0f;
    pos.y = 80.0f;
    pos.z = 0.0f;
    g_AsciiText.AddFormat(&pos, "Normal Ranking");
    pos.y += 16.0f;
    {
        int i;
        for (i = 0; i < 5; i++)
        {
            if (mask & 2)
            {
                g_AsciiText.color = highlight == i ? (alpha << 24) | 0xfff0ef : (alpha << 24) | 0xc0f0ff;
            }
            else
            {
                g_AsciiText.color = (alpha << 24) | 0x809090;
            }
            if (mask & 2 && highlight == i)
            {
                pos.x += (this->replayNameCursor + 4) * 9;
                g_AsciiText.AddFormat(&pos, "%.8s", "_");
                pos.x -= (this->replayNameCursor + 4) * 9;
            }
            g_AsciiText.AddFormat(&pos, "%s %.8s %.9d%d", g_ScoreRankNames[i], (*scores)[i].name, (*scores)[i].score,
                                  (char)(*scores)[i].unk_2b);
            pos.y += 16.0f;
        }
    }
    scores++;
    g_AsciiText.color = mask & 4 ? (alpha << 24) | 0xfff0d0 : (alpha << 24) | 0x808080;
    pos.x = 102.0f;
    pos.y = 192.0f;
    pos.z = 0.0f;
    g_AsciiText.AddFormat(&pos, "Hard Ranking");
    pos.y += 16.0f;
    {
        int i;
        for (i = 0; i < 5; i++)
        {
            if (mask & 4)
            {
                g_AsciiText.color = highlight == i ? (alpha << 24) | 0xfff0ef : (alpha << 24) | 0xc0f0ff;
            }
            else
            {
                g_AsciiText.color = (alpha << 24) | 0x809090;
            }
            if (mask & 4 && highlight == i)
            {
                pos.x += (this->replayNameCursor + 4) * 9;
                g_AsciiText.AddFormat(&pos, "%.8s", "_");
                pos.x -= (this->replayNameCursor + 4) * 9;
            }
            g_AsciiText.AddFormat(&pos, "%s %.8s %.9d%d", g_ScoreRankNames[i], (*scores)[i].name, (*scores)[i].score,
                                  (char)(*scores)[i].unk_2b);
            pos.y += 16.0f;
        }
    }
    scores++;
    g_AsciiText.color = mask & 8 ? (alpha << 24) | 0xffc0c0 : (alpha << 24) | 0x808080;
    pos.x = 358.0f;
    pos.y = 192.0f;
    pos.z = 0.0f;
    g_AsciiText.AddFormat(&pos, "Lunatic Ranking");
    pos.y += 16.0f;
    {
        int i;
        for (i = 0; i < 5; i++)
        {
            if (mask & 8)
            {
                g_AsciiText.color = highlight == i ? (alpha << 24) | 0xfff0ef : (alpha << 24) | 0xc0f0ff;
            }
            else
            {
                g_AsciiText.color = (alpha << 24) | 0x809090;
            }
            if (mask & 8 && highlight == i)
            {
                pos.x += (this->replayNameCursor + 4) * 9;
                g_AsciiText.AddFormat(&pos, "%.8s", "_");
                pos.x -= (this->replayNameCursor + 4) * 9;
            }
            g_AsciiText.AddFormat(&pos, "%s %.8s %.9d%d", g_ScoreRankNames[i], (*scores)[i].name, (*scores)[i].score,
                                  (char)(*scores)[i].unk_2b);
            pos.y += 16.0f;
        }
    }
    scores++;
    g_AsciiText.color = mask & 16 ? (alpha << 24) | 0xffc0f0 : (alpha << 24) | 0x808080;
    pos.x = 102.0f;
    pos.y = 304.0f;
    pos.z = 0.0f;
    g_AsciiText.AddFormat(&pos, "Extra Ranking");
    pos.y += 16.0f;
    {
        int i;
        for (i = 0; i < 5; i++)
        {
            if (mask & 16)
            {
                g_AsciiText.color = highlight == i ? (alpha << 24) | 0xfff0ef : (alpha << 24) | 0xc0f0ff;
            }
            else
            {
                g_AsciiText.color = (alpha << 24) | 0x809090;
            }
            if (mask & 16 && highlight == i)
            {
                pos.x += (this->replayNameCursor + 4) * 9;
                g_AsciiText.AddFormat(&pos, "%.8s", "_");
                pos.x -= (this->replayNameCursor + 4) * 9;
            }
            g_AsciiText.AddFormat(&pos, "%s %.8s %.9d%d", g_ScoreRankNames[i], (*scores)[i].name, (*scores)[i].score,
                                  (char)(*scores)[i].unk_2b);
            pos.y += 16.0f;
        }
    }
    if (this->state == 13)
    {
    }
    else if (this->state == 14 && this->scoreRank < 5 && this->subState >= 1)
    {
        pos.z = 0.0f;
        pos.y = 336.0f;
        for (row = 0; row < 6; row++)
        {
            pos.x = 348.0f;
            for (col = 0; col < 16; col++)
            {
                pos.x += 12.0f;
                float offsetX = 0.0f;
                float offsetY = 0.0f;
                if (this->cursor == col + row * 16)
                {
                    g_AsciiText.color = 0xffffffc0;
                    if (this->stateFrames % 64 < 32)
                    {
                        scale = this->stateFrames % 32 * 0.025f + 1.2f;
                    }
                    else
                    {
                        scale = 2.0f - this->stateFrames % 32 * 0.025f;
                    }
                    g_AsciiText.scaleY = scale;
                    g_AsciiText.scaleX = scale;
                    offsetY = offsetX = (scale - 1.0f) * -8.0f;
                }
                else
                {
                    g_AsciiText.color = 0xc0c0c0c0;
                    g_AsciiText.scaleY = 1.0f;
                    g_AsciiText.scaleX = 1.0f;
                }
                D3DXVECTOR3 charPos = D3DXVECTOR3(offsetX, offsetY, 0.0f) + pos;
                str[0] = g_NameAlphabet[row * 16 + col];
                str[1] = 0;
                if (row == 5)
                {
                    if (col == 14)
                    {
                        str[0] = 0x7f;
                    }
                    else if (col == 15)
                    {
                        str[0] = 0x80;
                    }
                }
                g_AsciiText.AddString(&charPos, str);
            }
            pos.y += 16.0f;
        }
    }
    g_AsciiText.color = 0xffffffff;
    g_AsciiText.scaleY = 1.0f;
    g_AsciiText.scaleX = 1.0f;
    return 1;
}

int MainMenu::DrawState12()
{
    if (this->subState == 1)
    {
        int idx = this->playingTune + 0x9f;
        D3DXVECTOR3 savedPos = this->sprites[idx].pos;
        D3DCOLOR savedColor = this->sprites[idx].color;
        this->sprites[idx].color = 0xffffffff;
        this->sprites[idx].pos.x = 208.0f;
        this->sprites[idx].pos.y = 293.0f;
        g_SpriteEngine->DrawUnrotated(&this->sprites[idx]);
        this->sprites[idx].pos = savedPos;
        this->sprites[idx].color = savedColor;
        for (int i = 0; i < 8; i++)
        {
            g_SpriteEngine->DrawUnrotated(&this->commentSprites[i]);
        }
    }
    return 1;
}

int MainMenu::MoveMenuCursor(int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (g_PlayerInputs[2].IsPressed(0x10))
    {
        this->cursor--;
        g_SoundPlayer.QueueSound(12, 0);
        if (this->cursor < 0)
        {
            this->cursor = count - 1;
        }
        if (this->cursor >= count)
        {
            this->cursor = 0;
        }
        return -1;
    }
    if (g_PlayerInputs[2].IsPressed(0x20))
    {
        this->cursor++;
        g_SoundPlayer.QueueSound(12, 0);
        if (this->cursor < 0)
        {
            this->cursor = count - 1;
        }
        if (this->cursor >= count)
        {
            this->cursor = 0;
        }
        return 1;
    }
    return 0;
}

int MainMenu::MoveMenuCursorLR(int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (g_PlayerInputs[2].IsPressed(0x40))
    {
        this->cursor--;
        if (this->cursor < 0)
        {
            this->cursor += count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return -1;
    }
    if (g_PlayerInputs[2].IsPressed(0x80))
    {
        this->cursor++;
        if (this->cursor >= count)
        {
            this->cursor -= count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return 1;
    }
    return 0;
}

int MainMenu::MoveCursor4(int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (g_PlayerInputs[2].IsPressed(0x40))
    {
        this->cursor--;
        if (this->cursor < 0)
        {
            this->cursor += count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return -1;
    }
    if (g_PlayerInputs[2].IsPressed(0x80))
    {
        this->cursor++;
        if (this->cursor >= count)
        {
            this->cursor -= count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return 1;
    }
    if (g_PlayerInputs[2].IsPressed(0x10))
    {
        this->cursor--;
        g_SoundPlayer.QueueSound(12, 0);
        if (this->cursor < 0)
        {
            this->cursor = count - 1;
        }
        if (this->cursor >= count)
        {
            this->cursor = 0;
        }
        return -1;
    }
    if (g_PlayerInputs[2].IsPressed(0x20))
    {
        this->cursor++;
        g_SoundPlayer.QueueSound(12, 0);
        if (this->cursor < 0)
        {
            this->cursor = count - 1;
        }
        if (this->cursor >= count)
        {
            this->cursor = 0;
        }
        return 1;
    }
    return 0;
}

int MainMenu::MovePlayerCursor(int idx, int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (g_PlayerInputs[idx].IsPressed(0x40))
    {
        this->cursors[idx]--;
        if (this->cursors[idx] < 0)
        {
            this->cursors[idx] += count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return -1;
    }
    if (g_PlayerInputs[idx].IsPressed(0x80))
    {
        this->cursors[idx]++;
        if (this->cursors[idx] >= count)
        {
            this->cursors[idx] -= count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return 1;
    }
    if (g_PlayerInputs[idx].IsPressed(0x10))
    {
        this->cursors[idx]--;
        if (this->cursors[idx] < 0)
        {
            this->cursors[idx] += count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return -1;
    }
    if (g_PlayerInputs[idx].IsPressed(0x20))
    {
        this->cursors[idx]++;
        if (this->cursors[idx] >= count)
        {
            this->cursors[idx] -= count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return 1;
    }
    return 0;
}

int MainMenu::MovePlayerCursor2(int player, int idx, int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (g_PlayerInputs[player].IsPressed(0x40))
    {
        this->cursors[idx]--;
        if (this->cursors[idx] < 0)
        {
            this->cursors[idx] += count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return -1;
    }
    if (g_PlayerInputs[player].IsPressed(0x80))
    {
        this->cursors[idx]++;
        if (this->cursors[idx] >= count)
        {
            this->cursors[idx] -= count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return 1;
    }
    if (g_PlayerInputs[player].IsPressed(0x10))
    {
        this->cursors[idx]--;
        if (this->cursors[idx] < 0)
        {
            this->cursors[idx] += count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return -1;
    }
    if (g_PlayerInputs[player].IsPressed(0x20))
    {
        this->cursors[idx]++;
        if (this->cursors[idx] >= count)
        {
            this->cursors[idx] -= count;
        }
        g_SoundPlayer.QueueSound(12, 0);
        return 1;
    }
    return 0;
}

int MainMenu::OnDraw(MainMenu *menu)
{
    int i;
    Sprite *vm;

    g_App.SetView(2);
    if (menu->loadingState != 0)
    {
        return 1;
    }
    g_SpriteEngine->currentTexture = NULL;
    g_SpriteEngine->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
    vm = menu->sprites;
    for (i = 0; i < menu->spriteCount; i++, vm++)
    {
#ifdef TH09_NATIVE_NETPLAY
        if ((menu->state == 1 || menu->state == 17 || menu->state == 18) && i >= 1 && i <= 8)
            continue;
#endif
        if (g_SpriteEngine->HasTexture(vm))
        {
            D3DXVECTOR3 savedPos = vm->pos;
            vm->pos += vm->posOffset;
            if (vm->rotation.z != 0.0f)
            {
                g_SpriteEngine->Draw(vm);
            }
            else
            {
                g_SpriteEngine->DrawUnrotated(vm);
            }
            vm->pos = savedPos;
        }
    }
    if (menu->helpSprite != NULL
#ifdef TH09_NATIVE_NETPLAY
        && menu->state != 1 && menu->state != 17 && menu->state != 18
#endif
        )
    {
        g_SpriteEngine->DrawUnrotated(menu->helpSprite);
    }
    switch (menu->state)
    {
    case 3:
        menu->DrawNothing();
        break;
    case 5:
        menu->DrawNothing();
        break;
    case 7:
        menu->DrawNothing();
        break;
    case 11:
        menu->DrawState11();
        break;
    case 12:
        menu->DrawState12();
        break;
    case 13:
    case 14:
        menu->DrawState13();
        break;
    case 15:
        menu->DrawState15();
        break;
    }
#ifdef TH09_NATIVE_NETPLAY
    thnet::DrawMenu(menu);
#endif
    g_SpriteEngine->FlushQuadBatch();
    return 1;
}

void MainMenu::LoadingThread(MainMenu *menu)
{
    int i;

    while (g_SpriteEngine->pendingSurfaceCopySurface >= 0)
    {
        Sleep(1);
    }
    g_MainMenu->anm = g_SpriteEngine->PreloadAnm(0xf, "title01.anm");
    if (g_MainMenu->anm == NULL)
    {
        g_MainMenu->loadingState = 2;
        return;
    }
    g_MainMenu->resultTextAnm = g_SpriteEngine->PreloadAnm(0x11, "resulttext.anm");
    if (g_MainMenu->resultTextAnm == NULL)
    {
        g_MainMenu->loadingState = 2;
        return;
    }
    if (g_App.closeSubThread != 0)
    {
        return;
    }
    for (i = 0; i < 14; i++)
    {
        g_App.textAnm->StartScript(&g_MainMenu->helpSprites[i], 5);
        g_App.textAnm->SetSprite(&g_MainMenu->helpSprites[i], g_MainMenu->helpSprites[i].spriteIndex + i);
        g_App.textAnm->SetSprite(&g_MainMenu->cardDetailSprites[i], i + 0x15);
        if (i <= 4)
        {
            g_MainMenu->cardDetailSprites[i].pos = D3DXVECTOR3(64.0f, i * 16.0f + 352.0f, 0.0f);
        }
        else
        {
            g_MainMenu->cardDetailSprites[i].pos = D3DXVECTOR3(64.0f, i * 16.0f + 362.0f, 0.0f);
        }
        g_MainMenu->cardDetailSprites[i].flags |= 0x1800;
        g_MainMenu->cardDetailSprites[i].fontWidth = 15;
        g_MainMenu->cardDetailSprites[i].fontHeight = 15;
        g_MainMenu->cardDetailSprites[i].flags |= 2;
    }
    if (g_App.closeSubThread != 0)
    {
        return;
    }
    if (g_MainMenu->state == 1)
    {
        if (g_SpriteEngine->PreloadSurface(0, "title/title00.png") != 0)
        {
            g_MainMenu->loadingState = 2;
            return;
        }
    }
    else if (g_SpriteEngine->PreloadSurface(0, "title/result00.png") != 0)
    {
        g_MainMenu->loadingState = 2;
        return;
    }
    if (!(g_GameManager.stateFlags & 2))
    {
        if (g_App.prevScene != 5)
        {
            g_App.LoadBgm(0);
        }
        if (g_App.lastPlayTimeTick == 0)
        {
            ScreenEffect::StartEffect(0, 70, 0xffffff, 0, 0, 0x23, 2);
        }
        else
        {
            ScreenEffect::StartEffect(0, 70, 0xffffff, 0, 0, 0x23, 2);
        }
    }
    g_MainMenu->helpSprite = g_MainMenu->helpSprites;
    g_MainMenu->loadingState = 0;
    if (g_Unknown4a7c80 != NULL)
    {
        g_Unknown4a7c80->Finish();
    }
    g_Unknown4a7c80 = NULL;
    g_App.StopLoadingSprites();
    g_App.subThread = NULL;
    g_App.closeSubThread = 0;
    g_App.subThreadRunning = 0;
}


int MainMenu::InitTitleScene()
{
    g_Unknown4ac884 = 0;
    g_AsciiText.Initialize();
    g_AsciiText.StartScripts();
    g_App.gameSpeed = 1.0f;
    if (g_GameManager.stateFlags & 8)
    {
        g_GameManager.sides[0].character = 0;
        g_GameManager.sides[1].character = 1;
    }
    if (g_GameManager.stateFlags & 2)
    {
        g_GameManager.stateFlags &= ~8;
    }
    this->SetState(1);
    g_App.ResetClockDriftCounters();
    if (g_Network->unk_a8 != 0 && g_GameManager.mode != 2)
    {
        this->unk_1b23c = 2;
        g_GameManager.unk_114 = 0;
        g_App.config.difficulty = 1;
    }
    switch (this->unk_1b22c)
    {
    case 0:
        if (g_GameManager.stateFlags & 0xa)
        {
            break;
        }
        if (g_GameManager.mode == 0)
        {
            this->cursor = 0;
        }
        else if (g_GameManager.mode == 1)
        {
            this->cursor = 1;
        }
        else if (g_GameManager.mode == 2)
        {
            this->unk_1b23c = 1;
        }
        break;
    case 1:
        this->state = 14;
        break;
    default:
        this->cursor = 0;
        break;
    }
    this->unk_88 = 0;
#ifdef TH09_NATIVE_NETPLAY
    if(thnet::ConsumeTitleReturn()) {
        this->unk_1b23c=0;
        this->SetState(1);
        this->prevState=0;
    }
#endif
    g_GameManager.stateFlags &= ~1;
    D3DXVECTOR3 pos(500.0f, 440.0f, 0.0f);
    if (g_App.prevScene == 2)
    {
        g_App.StartLoadingSpritesOverScreen(&pos);
        g_App.StartLoadingScreenEffect(0);
    }
    else if (g_App.prevScene != 0)
    {
        g_App.StartLoadingSprites(&pos);
    }
    g_GameManager.stateFlags &= ~2;
    this->loadingState = 1;
    g_App.StartSubThread((LPTHREAD_START_ROUTINE)LoadingThread, NULL);
    return 0;
}

int MainMenu::AddedCallback(MainMenu *menu)
{
    return menu->InitTitleScene();
}

int MainMenu::ReleaseTitleScene()
{
    if (this->loadedReplay != NULL)
    {
        free(this->loadedReplay);
        this->loadedReplay = NULL;
    }
    if (this->sprites != NULL)
    {
        delete[] this->sprites;
        this->sprites = NULL;
    }
    for (int i = 0; i < 50; i++)
    {
        if (this->unk_681c[i] != NULL)
        {
            free(this->unk_681c[i]);
        }
        this->unk_681c[i] = NULL;
    }
    return 0;
}

int MainMenu::DeletedCallback(MainMenu *menu)
{
    g_App.d3dDevice->ResourceManagerDiscardBytes(0);
    g_SpriteEngine->ReleaseAnm(0xf);
    g_SpriteEngine->ReleaseAnm(0x11);
    g_SpriteEngine->ReleaseSurface(0);
    g_Chain.Remove(menu->drawElem);
    menu->drawElem = NULL;
    menu->ReleaseTitleScene();
    delete menu;
    return 0;
}

void MainMenu::SelectEntry(int cursor, int first, int count)
{
    for (int i = first; i < first + count; i++)
    {
        this->anm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        this->sprites[i].pendingInterrupt = 8;
    }
    this->anm->SetSprite(&this->sprites[cursor + first], this->sprites[cursor + first].baseSpriteIndex);
    this->sprites[cursor + first].pendingInterrupt = 7;
}

void MainMenu::ShowEntryFrame(int frame, int idx)
{
    this->anm->SetSprite(&this->sprites[idx], this->sprites[idx].baseSpriteIndex + frame * 2);
    this->sprites[idx].pendingInterrupt = 7;
}

void MainMenu::SelectEntryStep(int cursor, int first, int count, int stride)
{
    for (int i = first; i < first + count * stride; i += stride)
    {
        this->anm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        this->sprites[i].pendingInterrupt = 8;
        g_SpriteEngine->Step(&this->sprites[i]);
    }
    this->anm->SetSprite(&this->sprites[cursor * stride + first], this->sprites[cursor * stride + first].baseSpriteIndex);
    this->sprites[cursor * stride + first].pendingInterrupt = 7;
    g_SpriteEngine->Step(&this->sprites[cursor * stride + first]);
}

void MainMenu::SelectEntry2(int cursor, int first, int count, int stride)
{
    for (int i = first; i < first + count * stride; i += stride)
    {
        this->anm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        this->sprites[i].pendingInterrupt = 15;
    }
    this->anm->SetSprite(&this->sprites[cursor * stride + first], this->sprites[cursor * stride + first].baseSpriteIndex);
    this->sprites[cursor * stride + first].pendingInterrupt = 14;
}

void MainMenu::SelectEntry3(int cursor, int first, int count, int stride)
{
    for (int i = first; i < first + count * stride; i += stride)
    {
        this->anm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        this->sprites[i].pendingInterrupt = 18;
    }
    this->anm->SetSprite(&this->sprites[cursor * stride + first], this->sprites[cursor * stride + first].baseSpriteIndex);
    this->sprites[cursor * stride + first].pendingInterrupt = 17;
}

void MainMenu::SelectEntryInterrupts(int cursor, int first, int count)
{
    for (int i = first; i < first + count; i++)
    {
        this->sprites[i].pendingInterrupt = 8;
    }
    this->sprites[cursor + first].pendingInterrupt = 7;
}

void MainMenu::FlashHalves(int left, int right)
{
    this->sprites[141].pendingInterrupt = 9;
    this->sprites[147].pendingInterrupt = 9;
    this->sprites[142].pendingInterrupt = 9;
    this->sprites[143].pendingInterrupt = 9;
    this->sprites[144].pendingInterrupt = 9;
    this->sprites[145].pendingInterrupt = 9;
    this->sprites[146].pendingInterrupt = 9;
    this->sprites[148].pendingInterrupt = 9;
    this->sprites[149].pendingInterrupt = 9;
    this->sprites[150].pendingInterrupt = 9;
    this->sprites[151].pendingInterrupt = 9;
    this->sprites[152].pendingInterrupt = 9;
    this->ShowHalves(left, right);
}

void MainMenu::FlashSprite(int idx, int on)
{
    this->sprites[idx + 153].pendingInterrupt = on ? 9 : 10;
}

void MainMenu::MoveCharacterCursor(int player, int selected, int selected2, char *entries)
{
    Sprite *vm = &this->sprites[player + 0xbd];
    int row = 0;
    int i;
    for (i = 0; i < 16; i++)
    {
        if (selected == entries[i])
        {
            break;
        }
        if (this->state == 8)
        {
            if (g_PlayRecord.openEntries[entries[i]] || g_Network->unk_a8 != 0)
            {
                row++;
            }
        }
        else if (this->state == 3)
        {
            if (g_PlayRecord.openEntries2[entries[i]])
            {
                row++;
            }
        }
        else if (g_PlayRecord.openEntries3[entries[i]])
        {
            row++;
        }
    }
    D3DXVECTOR3 to;
    to.x = vm->pos.x;
    to.y = row * 16.0f + 128.0f;
    to.z = 0.0f;
    vm->SetMoveInterp(8, 4, &vm->pos, &to);
    int row2 = 0;
    for (i = 0; i < 16; i++)
    {
        if (selected2 == entries[i])
        {
            break;
        }
        if (this->state == 8)
        {
            if (g_PlayRecord.openEntries[entries[i]] || g_Network->unk_a8 != 0)
            {
                row2++;
            }
        }
        else if (this->state == 3)
        {
            if (g_PlayRecord.openEntries2[entries[i]])
            {
                row2++;
            }
        }
        else if (g_PlayRecord.openEntries3[entries[i]])
        {
            row2++;
        }
    }
    int j = 0;
    for (i = 0; i < 16; i++)
    {
        if (g_PlayRecord.openEntries[entries[i]] || g_Network->unk_a8 != 0)
        {
            vm = &this->sprites[j + 191];
            if (row == j || row2 == j)
            {
                vm->SetColorInterp(8, 4, vm->color, 0xffffffff);
            }
            else
            {
                vm->SetColorInterp(8, 4, vm->color, 0xffa0a0a0);
            }
            j++;
        }
    }
}

void MainMenu::MoveCharacterCursor2(int player, int selected, char *entries)
{
    int row = 0;
    Sprite *vm = &this->sprites[189];
    int i;
    for (i = 0; i < 16; i++)
    {
        if (selected == entries[i])
        {
            break;
        }
        if (i > 13 || g_PlayRecord.openEntries[entries[i]] || g_Network->unk_a8 != 0)
        {
            row++;
        }
    }
    D3DXVECTOR3 to;
    to.x = vm->pos.x;
    to.y = row * 16.0f + 128.0f;
    to.z = 0.0f;
    vm->SetMoveInterp(8, 4, &vm->pos, &to);
    vm = &this->sprites[190];
    to.x = vm->pos.x;
    vm->SetMoveInterp(8, 4, &vm->pos, &to);
    int j = 0;
    for (i = 0; i < 16; i++)
    {
        if (g_PlayRecord.openEntries[entries[i]] || i > 13 || g_Network->unk_a8 != 0)
        {
            vm = &this->sprites[j + 207];
            if (row == j)
            {
                vm->SetColorInterp(8, 4, vm->color, 0xffffffff);
            }
            else
            {
                vm->SetColorInterp(8, 4, vm->color, 0xffa0a0a0);
            }
            j++;
        }
    }
}

int MainMenu::UpdateState3()
{
    int i;
    int n;
    char characters[2];

    switch (this->subState)
    {
    case 1:
        if (this->cursors[2] == 0)
        {
            if (!(g_PlayerInputs[2].unk_0 & 4))
            {
                if ((i = this->MovePlayerCursor2(2, 0, 14)) != 0)
                {
                    if (g_App.config.difficulty == 4)
                    {
                        this->SkipClosedEntries3(0, i, g_StoryCharacters, 14);
                    }
                    else
                    {
                        this->SkipClosedEntries2(0, i, g_StoryCharacters, 14);
                    }
                    characters[0] = g_StoryCharacters[this->cursors[0]];
                    if (g_App.config.difficulty == 4)
                    {
                        this->FlashSprite(0, g_PlayRecord.clearFlags[characters[0]][4]);
                    }
                    else
                    {
                        this->FlashSprite(0, g_PlayRecord.GetClearFlags(characters[0], g_App.config.difficulty));
                    }
                    if (i > 0)
                    {
                        this->SelectEntryStep(characters[0], 0x5c, 14, 2);
                        this->SelectEntryStep(characters[0], 0x3c, 14, 2);
                    }
                    else
                    {
                        this->SelectEntry2(characters[0], 0x5c, 14, 2);
                        this->SelectEntry2(characters[0], 0x3c, 14, 2);
                    }
                    this->MoveCharacterCursor(0, characters[0], characters[0], g_StoryCharacters);
                }
            }
            else if (this->unk_11b8c != 0)
            {
                if (g_PlayerInputs[2].IsPressed(0x40))
                {
                    if (this->unk_11b84 > 1)
                    {
                        this->unk_11b84--;
                    }
                    this->FlashHalves(this->unk_11b84, this->unk_11b88);
                }
                if (g_PlayerInputs[2].IsPressed(0x80))
                {
                    if (this->unk_11b84 < 10)
                    {
                        this->unk_11b84++;
                    }
                    this->FlashHalves(this->unk_11b84, this->unk_11b88);
                }
            }
        }
        characters[0] = g_StoryCharacters[this->cursors[0]];
        if ((g_PlayerInputs[2].unk_6 & 0x1001) && this->cursors[2] == 0)
        {
            this->PlaySoundNow(10, 0);
            this->cursors[2] = 1;
            this->sprites[characters[0] * 2 + 92].colorRgb[2] = 0x80;
            this->sprites[characters[0] * 2 + 92].colorRgb[1] = 0x80;
            this->sprites[characters[0] * 2 + 92].colorRgb[0] = 0x80;
            this->sprites[characters[0] * 2 + 60].colorRgb[2] = 0x80;
            this->sprites[characters[0] * 2 + 60].colorRgb[1] = 0x80;
            this->sprites[characters[0] * 2 + 60].colorRgb[0] = 0x80;
            g_GameManager.sides[0].unk_30 = g_PlayerInputs[2].unk_0 >> 2 & 1;
        }
        if (this->cursors[2] != 0 && this->cursors[3] != 0)
        {
            g_App.StopMusic();
            g_GameManager.sides[0].character = characters[0];
            g_GameManager.sides[1].character = 0;
            g_GameManager.sides[1].unk_30 = g_PlayerInputs[2].unk_0 >> 14 & 1;
            g_GameManager.unk_fc = 0;
            if (this->state == 3)
            {
                g_GameManager.sides[0].unk_28 = 0;
                g_GameManager.mode = 0;
                g_GameManager.sides[1].unk_28 = 1;
                g_App.scene = 2;
            }
            else if (this->state == 5)
            {
                g_GameManager.sides[0].unk_28 = 0;
                g_GameManager.mode = 1;
                g_GameManager.sides[1].unk_28 = 1;
                g_App.scene = 2;
            }
            else if (g_GameManager.unk_114 == 0)
            {
                g_GameManager.sides[0].unk_28 = 0;
                g_GameManager.sides[1].unk_28 = 0;
                g_GameManager.mode = 2;
                g_App.scene = 2;
            }
            else if (g_GameManager.unk_114 == 1)
            {
                g_GameManager.sides[0].unk_28 = 0;
                g_GameManager.mode = 2;
                g_GameManager.sides[1].unk_28 = 1;
                g_App.scene = 2;
            }
            else if (g_GameManager.unk_114 == 2)
            {
                g_GameManager.sides[0].unk_28 = 1;
                g_GameManager.sides[1].unk_28 = 0;
                g_GameManager.mode = 2;
                g_App.scene = 2;
            }
            else if (g_GameManager.unk_114 == 3)
            {
                g_GameManager.sides[0].unk_28 = 1;
                g_GameManager.mode = 2;
                g_GameManager.sides[1].unk_28 = 1;
                g_App.scene = 2;
            }
            g_GameManager.difficulty = g_App.config.difficulty;
            g_GameManager.initialLives[0] = this->unk_11b84;
            g_GameManager.initialLives[1] = this->unk_11b88;
            return 0;
        }
        if ((g_PlayerInputs[2].unk_6 & 0xa) && this->cursors[2] == 0)
        {
            g_GameManager.difficulty = this->cursor;
            this->PlaySoundNow(11, 0);
            this->stateFrames = 0;
            g_GameManager.sides[0].character = characters[0];
            if (this->state == 3)
            {
                this->SetState(2);
                return 1;
            }
            if (this->state == 5)
            {
                this->SetState(4);
                return 1;
            }
            this->SetState(7);
            return 1;
        }
        break;
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 13);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            n = 0;
            for (i = 0; i < 14; i++)
            {
                if ((g_App.config.difficulty == 4 && g_PlayRecord.openEntries3[g_StoryCharacters[i]]) ||
                    (g_App.config.difficulty != 4 && g_PlayRecord.openEntries2[g_StoryCharacters[i]]))
                {
                    this->anm->SetSprite(&this->sprites[n + 191], g_StoryCharacters[i] + 0xeb);
                    this->sprites[n + 191].flags |= 2;
                    n++;
                }
            }
            for (; n < 16; n++)
            {
                this->sprites[n + 191].flags &= ~2;
            }
            this->sprites[189].flags |= 2;
            this->sprites[190].flags &= ~2;
            for (this->cursors[0] = 0; g_StoryCharacters[this->cursors[0]] != g_GameManager.sides[0].character;
                 this->cursors[0]++)
            {
            }
            if (g_App.config.difficulty == 4)
            {
                this->SkipClosedEntries3(0, 1, g_StoryCharacters, 14);
            }
            else
            {
                this->SkipClosedEntries2(0, 1, g_StoryCharacters, 14);
            }
            characters[0] = g_StoryCharacters[this->cursors[0]];
            characters[1] = g_StoryCharacters[this->cursors[1]];
            this->sprites[characters[0] * 2 + 93].colorRgb[2] = 0xff;
            this->sprites[characters[0] * 2 + 93].colorRgb[1] = 0xff;
            this->sprites[characters[0] * 2 + 93].colorRgb[0] = 0xff;
            this->sprites[characters[0] * 2 + 61].colorRgb[2] = 0xff;
            this->sprites[characters[0] * 2 + 61].colorRgb[1] = 0xff;
            this->sprites[characters[0] * 2 + 61].colorRgb[0] = 0xff;
            this->sprites[characters[1] * 2 + 93].colorRgb[2] = 0xff;
            this->sprites[characters[1] * 2 + 93].colorRgb[1] = 0xff;
            this->sprites[characters[1] * 2 + 93].colorRgb[0] = 0xff;
            this->sprites[characters[1] * 2 + 61].colorRgb[2] = 0xff;
            this->sprites[characters[1] * 2 + 61].colorRgb[1] = 0xff;
            this->sprites[characters[1] * 2 + 61].colorRgb[0] = 0xff;
            this->SelectEntryStep(characters[0], 0x5c, 14, 2);
            this->SelectEntryStep(characters[0], 0x3c, 14, 2);
            this->cursors[2] = 0;
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
            this->unk_c918 = 1;
            this->cursors[3] = 1;
            this->MoveCharacterCursor(0, characters[0], characters[0], g_StoryCharacters);
            this->unk_11b8c = 0;
            this->unk_11b84 = 10;
            this->unk_11b88 = 10;
            if (g_PlayerInputs[2].unk_0 & 4)
            {
                this->unk_11b8c = 1;
                this->FlashHalves(10, 10);
            }
            if (g_App.config.difficulty == 4)
            {
                this->FlashSprite(0, g_PlayRecord.clearFlags[characters[0]][4]);
            }
            else
            {
                this->FlashSprite(0, g_PlayRecord.GetClearFlags(characters[0], g_App.config.difficulty));
            }
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    }
    this->stateFrames++;
    this->subMenuIdleFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateState8b()
{
    int i;
    int n;
    char characters[2];

    switch (this->subState)
    {
    case 1:
        characters[0] = g_VsCpuCharacters[this->cursors[0]];
        characters[1] = g_VsCpuCharacters[this->cursors[1]];
        if (this->cursors[2] == 0)
        {
            if (!(g_PlayerInputs[2].unk_0 & 4))
            {
                if ((i = this->MovePlayerCursor2(2, 0, 16)) != 0)
                {
                    this->SkipClosedEntries(0, i, g_VsCpuCharacters, 16);
                    characters[0] = g_VsCpuCharacters[this->cursors[0]];
                    characters[1] = g_VsCpuCharacters[this->cursors[1]];
                    if (i > 0)
                    {
                        this->SelectEntryStep(characters[0], 0x5c, 16, 2);
                        this->SelectEntryStep(characters[0], 0x3c, 16, 2);
                    }
                    else
                    {
                        this->SelectEntry2(characters[0], 0x5c, 16, 2);
                        this->SelectEntry2(characters[0], 0x3c, 16, 2);
                    }
                    this->MoveCharacterCursor(0, characters[0], characters[0], g_VsCpuCharacters);
                }
            }
            else
            {
                if (g_PlayerInputs[2].IsPressed(0x40))
                {
                    if (this->unk_11b84 > 1)
                    {
                        this->unk_11b84--;
                    }
                    this->FlashHalves(this->unk_11b84, this->unk_11b88);
                }
                if (g_PlayerInputs[2].IsPressed(0x80))
                {
                    if (this->unk_11b84 < 10)
                    {
                        this->unk_11b84++;
                    }
                    this->FlashHalves(this->unk_11b84, this->unk_11b88);
                }
            }
        }
        if (this->cursors[2] != 0 && this->cursors[3] == 0)
        {
            if (!(g_PlayerInputs[2].unk_0 & 4))
            {
                if ((i = this->MovePlayerCursor2(2, 1, 16)) != 0)
                {
                    this->SkipClosedEntries(1, i, g_VsCpuCharacters, 16);
                    characters[0] = g_VsCpuCharacters[this->cursors[0]];
                    characters[1] = g_VsCpuCharacters[this->cursors[1]];
                    if (i < 0)
                    {
                        this->SelectEntryStep(characters[1], 0x5d, 16, 2);
                        this->SelectEntryStep(characters[1], 0x3d, 16, 2);
                    }
                    else
                    {
                        this->SelectEntry2(characters[1], 0x5d, 16, 2);
                        this->SelectEntry2(characters[1], 0x3d, 16, 2);
                    }
                    this->MoveCharacterCursor(1, characters[1], characters[0], g_VsCpuCharacters);
                }
            }
            else
            {
                if (g_PlayerInputs[2].IsPressed(0x40))
                {
                    if (this->unk_11b88 > 1)
                    {
                        this->unk_11b88--;
                    }
                    this->FlashHalves(this->unk_11b84, this->unk_11b88);
                }
                if (g_PlayerInputs[2].IsPressed(0x80))
                {
                    if (this->unk_11b88 < 10)
                    {
                        this->unk_11b88++;
                    }
                    this->FlashHalves(this->unk_11b84, this->unk_11b88);
                }
            }
        }
        if ((g_PlayerInputs[2].unk_6 & 0x1001) && this->cursors[2] == 0)
        {
            this->PlaySoundNow(10, 0);
            this->cursors[2] = 1;
            this->sprites[characters[0] * 2 + 92].colorRgb[2] = 0x80;
            this->sprites[characters[0] * 2 + 92].colorRgb[1] = 0x80;
            this->sprites[characters[0] * 2 + 92].colorRgb[0] = 0x80;
            this->sprites[characters[0] * 2 + 60].colorRgb[2] = 0x80;
            this->sprites[characters[0] * 2 + 60].colorRgb[1] = 0x80;
            this->sprites[characters[0] * 2 + 60].colorRgb[0] = 0x80;
            g_GameManager.sides[0].unk_30 = g_PlayerInputs[2].unk_0 >> 2 & 1;
            if (this->cursors[3] != 0 && characters[0] == characters[1])
            {
                g_GameManager.sides[0].unk_30 = 1 - g_GameManager.sides[1].unk_30;
            }
            this->sprites[190].flags |= 2;
            this->MoveCharacterCursor(1, characters[1], characters[0], g_VsCpuCharacters);
        }
        else if (this->cursors[2] != 0 && (g_PlayerInputs[2].unk_6 & 0x1001) && this->cursors[3] == 0)
        {
            this->PlaySoundNow(10, 0);
            this->cursors[3] = 1;
            this->sprites[characters[1] * 2 + 93].colorRgb[2] = 0x80;
            this->sprites[characters[1] * 2 + 93].colorRgb[1] = 0x80;
            this->sprites[characters[1] * 2 + 93].colorRgb[0] = 0x80;
            this->sprites[characters[1] * 2 + 61].colorRgb[2] = 0x80;
            this->sprites[characters[1] * 2 + 61].colorRgb[1] = 0x80;
            this->sprites[characters[1] * 2 + 61].colorRgb[0] = 0x80;
            g_GameManager.sides[1].unk_30 = g_PlayerInputs[2].unk_0 >> 2 & 1;
            if (this->cursors[2] != 0 && characters[0] == characters[1])
            {
                g_GameManager.sides[1].unk_30 = 1 - g_GameManager.sides[0].unk_30;
            }
        }
        if (this->cursors[2] != 0 && this->cursors[3] != 0)
        {
            g_GameManager.sides[0].character = characters[0];
            g_GameManager.sides[1].character = characters[1];
            g_GameManager.initialLives[0] = this->unk_11b84;
            g_GameManager.initialLives[1] = this->unk_11b88;
            this->PlaySoundNow(10, 0);
            this->SetState(16);
            this->UpdateState16();
            return 1;
        }
        if ((g_PlayerInputs[2].unk_6 & 0xa) && this->cursors[2] == 0)
        {
            g_GameManager.difficulty = this->cursor;
            this->PlaySoundNow(11, 0);
            this->stateFrames = 0;
            g_GameManager.sides[0].character = characters[0];
            g_GameManager.sides[1].character = characters[1];
            if (this->state == 3)
            {
                this->SetState(2);
                return 1;
            }
            if (this->state == 5)
            {
                this->SetState(4);
                return 1;
            }
            this->SetState(7);
            return 1;
        }
        else if (this->cursors[2] != 0 && (g_PlayerInputs[2].unk_6 & 0xa) && this->cursors[3] != 0)
        {
            this->cursors[3] = 0;
            this->PlaySoundNow(11, 0);
            this->sprites[characters[1] * 2 + 93].colorRgb[2] = 0xff;
            this->sprites[characters[1] * 2 + 93].colorRgb[1] = 0xff;
            this->sprites[characters[1] * 2 + 93].colorRgb[0] = 0xff;
            this->sprites[characters[1] * 2 + 61].colorRgb[2] = 0xff;
            this->sprites[characters[1] * 2 + 61].colorRgb[1] = 0xff;
            this->sprites[characters[1] * 2 + 61].colorRgb[0] = 0xff;
        }
        else if ((g_PlayerInputs[2].unk_6 & 0xa) && this->cursors[2] == 1)
        {
            this->cursors[2] = 0;
            this->PlaySoundNow(11, 0);
            this->sprites[characters[0] * 2 + 92].colorRgb[2] = 0xff;
            this->sprites[characters[0] * 2 + 92].colorRgb[1] = 0xff;
            this->sprites[characters[0] * 2 + 92].colorRgb[0] = 0xff;
            this->sprites[characters[0] * 2 + 60].colorRgb[2] = 0xff;
            this->sprites[characters[0] * 2 + 60].colorRgb[1] = 0xff;
            this->sprites[characters[0] * 2 + 60].colorRgb[0] = 0xff;
            this->sprites[190].flags &= ~2;
            this->MoveCharacterCursor(0, characters[0], characters[0], g_VsCpuCharacters);
        }
        break;
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 13);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            n = 0;
            for (i = 0; i < 16; i++)
            {
                if (g_PlayRecord.openEntries[g_VsCpuCharacters[i]])
                {
                    this->anm->SetSprite(&this->sprites[n + 191], g_VsCpuCharacters[i] + 0xeb);
                    this->sprites[n + 191].flags |= 2;
                    n++;
                }
            }
            for (; n < 16; n++)
            {
                this->sprites[n + 191].flags &= ~2;
            }
            this->sprites[189].flags |= 2;
            this->sprites[190].flags &= ~2;
            for (this->cursors[0] = 0; g_VsCpuCharacters[this->cursors[0]] != g_GameManager.sides[0].character;
                 this->cursors[0]++)
            {
            }
            for (this->cursors[1] = 0; g_VsCpuCharacters[this->cursors[1]] != g_GameManager.sides[1].character;
                 this->cursors[1]++)
            {
            }
            this->SkipClosedEntries(0, 1, g_VsCpuCharacters, 16);
            this->SkipClosedEntries(1, 1, g_VsCpuCharacters, 16);
            characters[0] = g_VsCpuCharacters[this->cursors[0]];
            characters[1] = g_VsCpuCharacters[this->cursors[1]];
            this->sprites[characters[0] * 2 + 93].colorRgb[2] = 0xff;
            this->sprites[characters[0] * 2 + 93].colorRgb[1] = 0xff;
            this->sprites[characters[0] * 2 + 93].colorRgb[0] = 0xff;
            this->sprites[characters[0] * 2 + 61].colorRgb[2] = 0xff;
            this->sprites[characters[0] * 2 + 61].colorRgb[1] = 0xff;
            this->sprites[characters[0] * 2 + 61].colorRgb[0] = 0xff;
            this->sprites[characters[1] * 2 + 93].colorRgb[2] = 0xff;
            this->sprites[characters[1] * 2 + 93].colorRgb[1] = 0xff;
            this->sprites[characters[1] * 2 + 93].colorRgb[0] = 0xff;
            this->sprites[characters[1] * 2 + 61].colorRgb[2] = 0xff;
            this->sprites[characters[1] * 2 + 61].colorRgb[1] = 0xff;
            this->sprites[characters[1] * 2 + 61].colorRgb[0] = 0xff;
            this->SelectEntryStep(characters[0], 0x5c, 16, 2);
            this->SelectEntryStep(characters[1], 0x5d, 16, 2);
            this->SelectEntryStep(characters[0], 0x3c, 16, 2);
            this->SelectEntryStep(characters[1], 0x3d, 16, 2);
            this->unk_c918 = 1;
            this->cursors[3] = 0;
            this->cursors[2] = 0;
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
            this->unk_11b84 = g_GameManager.initialLives[0];
            this->unk_11b88 = g_GameManager.initialLives[1];
            this->FlashHalves(this->unk_11b84, this->unk_11b88);
            this->MoveCharacterCursor(0, characters[0], characters[0], g_VsCpuCharacters);
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    }
    this->stateFrames++;
    this->subMenuIdleFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateState16()
{
    int i;
    int n;
    int delta;
    char character;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 0x1a);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            n = 0;
            for (i = 0; i < 16; i++)
            {
                char entryCharacter = g_CharacterOrder[i];
                if (entryCharacter > 13 || g_PlayRecord.openEntries[entryCharacter] ||
                    g_Network->unk_a8)
                {
                    this->anm->SetSprite(&this->sprites[n + 207], entryCharacter + 0xfb);
                    this->sprites[n + 207].flags |= 2;
                    g_OpenCharacters[n] = g_CharacterOrder[i];
                    n++;
                }
            }
            this->cursor = n - 2;
            if (n < 16)
            {
                int index = n;
                char *entry = g_OpenCharacters + index;
                do
                {
                    this->sprites[207 + index].flags &= ~2;
                    index++;
                    *entry++ = -1;
                } while (index < 16);
            }
            this->sprites[189].flags |= 2;
            this->sprites[190].flags |= 2;
            character = g_OpenCharacters[this->cursor];
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
            this->unk_c918 = 1;
            this->MoveCharacterCursor2(0, character, g_CharacterOrder);
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
        if ((delta = this->MoveCursor4(16)) != 0)
        {
            while (g_OpenCharacters[this->cursor] < 0)
            {
                this->cursor = (this->cursor + delta) % 16;
            }
            this->MoveCharacterCursor2(0, g_OpenCharacters[this->cursor], g_CharacterOrder);
        }
        if (g_PlayerInputs[2].unk_0 & 0x100)
        {
            this->cursor += g_Rng.GetRandomU32() & 0xf;
            if (this->cursor >= 16)
            {
                this->cursor -= 16;
            }
            this->MoveCharacterCursor2(0, g_OpenCharacters[this->cursor], g_CharacterOrder);
        }
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            this->PlaySoundNow(10, 0);
            g_App.StopMusic();
            g_GameManager.unk_fc = 0;
            if (this->state == 3)
            {
                g_GameManager.sides[1].unk_28 = 1;
                g_App.scene = 2;
                g_GameManager.mode = 0;
                g_GameManager.sides[0].unk_28 = 0;
            }
            else if (this->state == 5)
            {
                g_GameManager.sides[1].unk_28 = 1;
                g_App.scene = 2;
                g_GameManager.mode = 1;
                g_GameManager.sides[0].unk_28 = 0;
            }
            else
            {
                if (g_GameManager.unk_114 == 0 || g_GameManager.unk_114 == 4)
                {
                    g_GameManager.sides[1].unk_28 = 0;
                    g_GameManager.sides[0].unk_28 = 0;
                    g_App.scene = 2;
                    g_GameManager.mode = 2;
                }
                else if (g_GameManager.unk_114 == 1)
                {
                    g_GameManager.sides[1].unk_28 = 1;
                    g_GameManager.sides[0].unk_28 = 0;
                    g_App.scene = 2;
                    g_GameManager.mode = 2;
                }
                else if (g_GameManager.unk_114 == 2)
                {
                    g_GameManager.sides[1].unk_28 = 0;
                    g_GameManager.sides[0].unk_28 = 1;
                    g_App.scene = 2;
                    g_GameManager.mode = 2;
                }
                else if (g_GameManager.unk_114 == 3)
                {
                    g_GameManager.sides[1].unk_28 = 1;
                    g_GameManager.sides[0].unk_28 = 1;
                    g_App.scene = 2;
                    g_GameManager.mode = 2;
                }
            }
            g_GameManager.difficulty = g_App.config.difficulty;
            g_GameManager.initialLives[0] = this->unk_11b84;
            g_GameManager.initialLives[1] = this->unk_11b88;
            g_GameManager.unk_138 = g_OpenCharacters[this->cursor];
            if (g_GameManager.unk_138 == 14)
            {
                g_GameManager.unk_138 = -1;
            }
            else if (g_GameManager.unk_138 == 15)
            {
                g_GameManager.unk_138 = -2;
            }
            else
            {
                g_GameManager.unk_138 = g_OpenCharacters[this->cursor];
            }
            return 0;
        }
        if (g_PlayerInputs[2].unk_6 & 10)
        {
            this->PlaySoundNow(11, 0);
            this->stateFrames = 0;
            this->SetState(8);
            switch (g_GameManager.unk_114)
            {
            case 0:
                this->UpdateState8();
                break;
            case 1:
                this->UpdateState8b();
                break;
            case 2:
                this->UpdateState8b();
                break;
            case 3:
                this->UpdateState8b();
                break;
            }
            return 1;
        }
        break;
    }
    this->stateFrames++;
    this->subMenuIdleFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateState13()
{
    int i;
    int delta;
    char character;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            if (g_SpriteEngine->LoadSurface(0, "title/result00.png") != 0)
            {
                return 0;
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 16);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            this->cursor = 0;
            this->scoreCharacter = 0;
            this->SelectEntry3(g_ScoreCharacters[0], 0x3c, 0x10, 2);
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
            this->scoreRank = 0;
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
        if (g_CheatCodeProgress < 3)
        {
            if (g_PlayerInputs[2].unk_6 & 0x2000)
            {
                g_CheatCodeProgress++;
            }
            else if (g_PlayerInputs[2].unk_6)
            {
                g_CheatCodeProgress = 0;
            }
        }
        else if (g_CheatCodeProgress < 7)
        {
            if (g_PlayerInputs[2].unk_6 & 0x400)
            {
                g_CheatCodeProgress++;
            }
            else if (g_PlayerInputs[2].unk_6)
            {
                g_CheatCodeProgress = 0;
            }
        }
        else if (g_CheatCodeProgress < 8)
        {
            if (g_PlayerInputs[2].unk_6 & 0x200)
            {
                g_CheatCodeProgress++;
            }
            else if (g_PlayerInputs[2].unk_6)
            {
                g_CheatCodeProgress = 0;
            }
        }
        else if (g_CheatCodeProgress < 10)
        {
            if (g_PlayerInputs[2].unk_6 & 0x4000)
            {
                g_CheatCodeProgress++;
            }
            else if (g_PlayerInputs[2].unk_6)
            {
                g_CheatCodeProgress = 0;
            }
        }
        if (g_CheatCodeProgress >= 10)
        {

            for (i = 0; i < 16; i++)
            {
                g_PlayRecord.openEntries[(char)i] = 1;
            }
            for (i = 0; i <= 13; i++)
            {
                g_PlayRecord.openEntries2[(char)i] = 1;
                g_PlayRecord.openEntries3[(char)i] = 1;
            }
            g_SoundPlayer.QueueSound(0x1c, 0);
            g_CheatCodeProgress = 0;
        }
        if ((delta = this->MoveMenuCursorLR(14)) != 0)
        {
            while (!g_PlayRecord.openEntries2[this->cursor])
            {
                if (g_PlayRecord.openEntries3[this->cursor])
                {
                    break;
                }
                this->cursor += delta;
                if (this->cursor < 0)
                {
                    this->cursor += 16;
                }
                if (this->cursor >= 16)
                {
                    this->cursor -= 16;
                }
            }
            character = g_ScoreCharacters[this->cursor];
            this->SelectEntry3(character, 0x3c, 0x10, 2);
            this->scoreCharacter = character;
        }
        if (g_PlayerInputs[2].unk_6 & 10)
        {
            this->PlaySoundNow(11, 0);
            this->stateFrames = 0;
            this->SetState(1);
            this->cursor = 4;
            return 1;
        }
        break;
    }
    this->stateFrames++;
    this->subMenuIdleFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateState11()
{
    int i;
    int delta;
    HANDLE findHandle;
    int size;
    ReplayHeader *replay;
    char path[0x40];
    WIN32_FIND_DATAA fd;
    char userPath[0x400];

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_GameManager.stateFlags &= ~8;
            if (this->prevState != 11 && g_SpriteEngine->LoadSurface(0, "title/replay00.png") != 0)
            {
                return 0;
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 0x16);
            this->cursor = 0;
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
            memset(this->replays, 0, sizeof(this->replays));
            for (i = 0; i < 25; i++)
            {
                sprintf(path, "./replay/th9_%.2d.rpy", i + 1);
                replay = (ReplayHeader *)LoadFileData(path, &size, 1);
                if (replay == NULL)
                {
                    continue;
                }
                replay = ValidateReplay(replay, size);
                this->unk_681c[i] = replay;
                if (replay != NULL)
                {
                    this->replays[i] = *replay;
                    strcpy(this->replayPaths[i], path);
                }
            }
            _mkdir("replay");
            _chdir("replay");
            findHandle = FindFirstFileA("th9_ud????.rpy", &fd);
            if (findHandle != INVALID_HANDLE_VALUE)
            {
                for (i = 0; i < 25; i++)
                {
                    strcpy(userPath, "replay/");
                    strcat(userPath, fd.cFileName);
                    replay = (ReplayHeader *)LoadFileData(userPath, &size, 1);
                    if (replay == NULL)
                    {
                        continue;
                    }
                    replay = ValidateReplay(replay, size);
                    this->unk_681c[i + 25] = replay;
                    if (replay != NULL)
                    {
                        this->replays[i + 25] = *replay;
                        sprintf(this->replayPaths[i + 25], "./replay/%s", fd.cFileName);
                    }
                    if (!FindNextFileA(findHandle, &fd))
                    {
                        break;
                    }
                }
            }
            FindClose(findHandle);
            _chdir("../");
            this->replayCount = 50;
            this->unk_c900 = 0;
        }
        if (this->inputLockout >= 8)
        {
            this->subState = 1;
            this->stateFrames = 0;
        }
        break;
    case 1:
        if (g_PlayerInputs[2].IsPressed(0x10))
        {
            g_SoundPlayer.QueueSound(12, 0);
            this->cursor--;
            if (this->cursor < 0)
            {
                this->cursor += 50;
            }
            this->stateFrames = 0;
        }
        if (g_PlayerInputs[2].IsPressed(0x20))
        {
            g_SoundPlayer.QueueSound(12, 0);
            this->cursor++;
            if (this->cursor >= 50)
            {
                this->cursor -= 50;
            }
            this->stateFrames = 0;
        }
        if (g_PlayerInputs[2].IsPressed(0x40))
        {
            g_SoundPlayer.QueueSound(12, 0);
            this->cursor -= 25;
            if (this->cursor < 0)
            {
                this->cursor += 50;
            }
            this->stateFrames = 0;
        }
        if (g_PlayerInputs[2].IsPressed(0x80))
        {
            g_SoundPlayer.QueueSound(12, 0);
            this->cursor += 25;
            if (this->cursor >= 50)
            {
                this->cursor -= 50;
            }
            this->stateFrames = 0;
        }
        if (g_PlayerInputs[2].unk_6 & 10)
        {
            this->PlaySoundNow(11, 0);
            this->stateFrames = 0;
            this->SetState(1);
            this->cursor = 3;
            for (i = 0; i < 50; i++)
            {
                if (this->unk_681c[i] != NULL)
                {
                    free(this->unk_681c[i]);
                }
                this->unk_681c[i] = NULL;
            }
            return 1;
        }
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            if (*(int *)this->replays[this->cursor].magic != 0)
            {
                this->PlaySoundNow(10, 0);
                this->replaySlot = this->cursor;
                this->subState = 2;
                this->stateFrames = 0;
                this->cursor = 0;
                this->inputLockout = 0;
                while (this->replays[this->replaySlot].unk_20[0][this->cursor] == NULL)
                {
                    this->cursor++;
                }
            }
            else
            {
                this->PlaySoundNow(0x27, 0);
            }
        }
        break;
    case 2:
        if ((delta = this->MoveMenuCursor(10)) != 0)
        {
            while (this->replays[this->replaySlot].unk_20[0][this->cursor] == NULL)
            {
                this->cursor += delta;
                if (this->cursor >= 10)
                {
                    this->cursor -= 10;
                }
                if (this->cursor < 0)
                {
                    this->cursor += 10;
                }
            }
        }
        if (g_PlayerInputs[2].unk_6 & 10)
        {
            this->subState = 1;
            this->stateFrames = 0;
            this->cursor = this->replaySlot;
        }
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            g_GameManager.sides[0].character = this->cursors[0];
            g_GameManager.sides[1].character = 0;
            g_GameManager.unk_fc = this->cursor;
            g_GameManager.difficulty = this->replays[this->replaySlot].info.difficulty;
            strcpy(g_GameManager.replayFile, this->replayPaths[this->replaySlot]);
            g_GameManager.stateFlags |= 8;
            if (this->replays[this->replaySlot].mode == 0)
            {
                g_GameManager.sides[1].unk_28 = 1;
                g_App.scene = 2;
                g_GameManager.mode = 0;
                g_GameManager.sides[0].unk_28 = 0;
            }
            else if (this->replays[this->replaySlot].mode == 1)
            {
                g_GameManager.sides[1].unk_28 = 1;
                g_App.scene = 2;
                g_GameManager.mode = 1;
                g_GameManager.sides[0].unk_28 = 0;
            }
            else
            {
                if (this->replays[this->replaySlot].unk_1e5 == 0)
                {
                    g_GameManager.sides[1].unk_28 = 0;
                    g_App.scene = 2;
                    g_GameManager.mode = 2;
                    g_GameManager.sides[0].unk_28 = 0;
                }
                else if (this->replays[this->replaySlot].unk_1e5 == 1)
                {
                    g_GameManager.sides[1].unk_28 = 1;
                    g_App.scene = 2;
                    g_GameManager.mode = 2;
                    g_GameManager.sides[0].unk_28 = 0;
                }
                else if (this->replays[this->replaySlot].unk_1e5 == 2)
                {
                    g_GameManager.sides[1].unk_28 = 0;
                    g_GameManager.sides[0].unk_28 = 1;
                    g_App.scene = 2;
                    g_GameManager.mode = 2;
                }
                else if (this->replays[this->replaySlot].unk_1e5 == 3)
                {
                    g_GameManager.sides[1].unk_28 = 1;
                    g_GameManager.sides[0].unk_28 = 1;
                    g_App.scene = 2;
                    g_GameManager.mode = 2;
                }
                else if (this->replays[this->replaySlot].unk_1e5 == 4)
                {
                    g_GameManager.sides[1].unk_28 = 0;
                    g_App.scene = 2;
                    g_GameManager.mode = 2;
                    g_GameManager.sides[0].unk_28 = 0;
                }
            }
            for (i = 0; i < 50; i++)
            {
                if (this->unk_681c[i] != NULL)
                {
                    free(this->unk_681c[i]);
                }
                this->unk_681c[i] = NULL;
            }
            g_GameManager.difficulty = g_App.config.difficulty;
            return 0;
        }
        break;
    }
    this->subMenuIdleFrames++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateState12()
{
    int i;
    int k;
    int row;
    int end;
    char *rp;
    char *data;
    int fileSize;

    switch (this->subState)
    {
    case 1:
        if (this->inputLockout == 2)
        {
            this->commentSprites[0].pendingInterrupt = 1;
            if (g_PlayRecord.bgmHeard[this->playingTune])
            {
                g_SpriteEngine->RenderText(&this->commentSprites[0], 0xe0e0ff, 0x302080, this->tunes[this->playingTune].comment[0]);
            }
            else
            {
                g_SpriteEngine->RenderText(&this->commentSprites[0], 0xe0e0ff, 0x302080, g_MusicNotHeardComment[0]);
            }
        }
        else if (this->inputLockout == 4)
        {
            this->commentSprites[1].pendingInterrupt = 1;
            if (g_PlayRecord.bgmHeard[this->playingTune])
            {
                g_SpriteEngine->RenderText(&this->commentSprites[1], 0xe0e0ff, 0x302080, this->tunes[this->playingTune].comment[1]);
            }
            else
            {
                g_SpriteEngine->RenderText(&this->commentSprites[1], 0xe0e0ff, 0x302080, g_MusicNotHeardComment[1]);
            }
        }
        else if (this->inputLockout == 6)
        {
            this->commentSprites[2].pendingInterrupt = 1;
            if (g_PlayRecord.bgmHeard[this->playingTune])
            {
                g_SpriteEngine->RenderText(&this->commentSprites[2], 0xe0e0ff, 0x302080, this->tunes[this->playingTune].comment[2]);
            }
            else
            {
                g_SpriteEngine->RenderText(&this->commentSprites[2], 0xe0e0ff, 0x302080, g_MusicNotHeardComment[2]);
            }
        }
        else if (this->inputLockout == 8)
        {
            this->commentSprites[3].pendingInterrupt = 1;
            if (g_PlayRecord.bgmHeard[this->playingTune])
            {
                g_SpriteEngine->RenderText(&this->commentSprites[3], 0xe0e0ff, 0x302080, this->tunes[this->playingTune].comment[3]);
            }
            else
            {
                g_SpriteEngine->RenderText(&this->commentSprites[3], 0xe0e0ff, 0x302080, g_MusicNotHeardComment[3]);
            }
        }
        else if (this->inputLockout == 10)
        {
            this->commentSprites[4].pendingInterrupt = 1;
            if (g_PlayRecord.bgmHeard[this->playingTune])
            {
                g_SpriteEngine->RenderText(&this->commentSprites[4], 0xe0e0ff, 0x302080, this->tunes[this->playingTune].comment[4]);
            }
            else
            {
                g_SpriteEngine->RenderText(&this->commentSprites[4], 0xe0e0ff, 0x302080, g_MusicNotHeardComment[4]);
            }
        }
        else if (this->inputLockout == 12)
        {
            this->commentSprites[5].pendingInterrupt = 1;
            if (g_PlayRecord.bgmHeard[this->playingTune])
            {
                g_SpriteEngine->RenderText(&this->commentSprites[5], 0xe0e0ff, 0x302080, this->tunes[this->playingTune].comment[5]);
            }
            else
            {
                g_SpriteEngine->RenderText(&this->commentSprites[5], 0xe0e0ff, 0x302080, g_MusicNotHeardComment[5]);
            }
        }
        else if (this->inputLockout == 14)
        {
            this->commentSprites[6].pendingInterrupt = 1;
            if (g_PlayRecord.bgmHeard[this->playingTune])
            {
                g_SpriteEngine->RenderText(&this->commentSprites[6], 0xe0e0ff, 0x302080, this->tunes[this->playingTune].comment[6]);
            }
            else
            {
                g_SpriteEngine->RenderText(&this->commentSprites[6], 0xe0e0ff, 0x302080, g_MusicNotHeardComment[6]);
            }
        }
        else if (this->inputLockout == 16)
        {
            this->commentSprites[7].pendingInterrupt = 1;
            if (g_PlayRecord.bgmHeard[this->playingTune])
            {
                g_SpriteEngine->RenderText(&this->commentSprites[7], 0xe0e0ff, 0x302080, this->tunes[this->playingTune].comment[7]);
            }
            else
            {
                g_SpriteEngine->RenderText(&this->commentSprites[7], 0xe0e0ff, 0x302080, g_MusicNotHeardComment[7]);
            }
        }
        if (this->MoveMenuCursor(this->tuneCount) != 0)
        {
            if (this->cursor < this->listTop)
            {
                this->listTop = this->cursor;
            }
            else if (this->cursor >= this->listTop + 10)
            {
                this->listTop = this->cursor - 9;
                if (this->listTop < 0)
                {
                    this->listTop = 0;
                }
            }
            this->SelectEntryInterrupts(this->cursor, 0x9f, this->tuneCount);
            for (i = 0; i < this->listTop; i++)
            {
                this->sprites[i + 0x9f].flags &= ~2;
            }
            end = this->listTop + 10 < this->tuneCount ? this->listTop + 10 : this->tuneCount;
            for (i = this->listTop; i < end; i++)
            {
                this->sprites[i + 0x9f].flags |= 2;
                this->sprites[i + 0x9f].pos.y = (i - this->listTop + 1) * 18 + 104.0f - 20.0f;
            }
            for (i = end; i <= 29; i++)
            {
                this->sprites[i + 0x9f].flags &= ~2;
            }
        }
        if (g_PlayerInputs[2].unk_6 & 4)
        {
            g_App.FadeOutMusic(20.0f);
        }
        if (g_PlayerInputs[2].unk_6 & 0x200)
        {
            if (this->musicPaused == 0)
            {
                g_SoundPlayer.QueueSoundCommand(6, 0, "Pause");
            }
            else
            {
                g_SoundPlayer.QueueSoundCommand(7, 0, "UnPause");
            }
            this->musicPaused = 1 - this->musicPaused;
        }
        if (g_PlayerInputs[2].unk_6 & 0x4000)
        {
            if (this->musicPaused != 0)
            {
                g_SoundPlayer.QueueSoundCommand(7, 0, "UnPause");
            }
            this->musicPaused = 0;
            g_App.PlayMusic(this->tunes[this->playingTune].path, 0);
        }
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            if (this->musicPaused != 0)
            {
                g_SoundPlayer.QueueSoundCommand(7, 0, "UnPause");
            }
            this->playingTune = this->cursor;
            this->musicPaused = 0;
            g_App.PlayMusic(this->tunes[this->playingTune].path, 0);
            this->inputLockout = 0;
            for (i = 0; i < 8; i++)
            {
                g_App.textAnm->StartScript(&this->commentSprites[i], i + 6);
            }
        }
        else if (g_PlayerInputs[2].unk_6 & 0xa)
        {
            this->PlaySoundNow(11, 0);
            this->stateFrames = 0;
            this->SetState(1);
            this->cursor = 5;
            return 1;
        }
        break;
    case 0:
        if (this->inputLockout == 0)
        {
            if (g_SpriteEngine->LoadSurface(0, "title/music00.png") != 0)
            {
                return 0;
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 16);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            this->cursor = 0;
            this->listTop = 0;
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
            data = rp = (char *)LoadFileData("sprt/musiccmt.txt", &fileSize, 0);
            if (rp == NULL)
            {
                return -1;
            }
            i = -1;
            while ((DWORD)(rp - data) < (DWORD)fileSize)
            {
                if (*rp == '@')
                {
                    rp++;
                    i++;
                    k = 0;
                    while (*rp != '\n' && *rp != '\r')
                    {
                        this->tunes[i].path[k] = *rp;
                        rp++;
                        k++;
                        if ((DWORD)(rp - data) >= (DWORD)fileSize)
                        {
                            goto done;
                        }
                    }
                    while (*rp == '\n' || *rp == '\r')
                    {
                        rp++;
                        if ((DWORD)(rp - data) >= (DWORD)fileSize)
                        {
                            goto done;
                        }
                    }
                    k = 0;
                    while (*rp != '\n' && *rp != '\r')
                    {
                        this->tunes[i].title[k] = *rp;
                        rp++;
                        k++;
                        if ((DWORD)(rp - data) >= (DWORD)fileSize)
                        {
                            goto done;
                        }
                    }
                    while (*rp == '\n' || *rp == '\r')
                    {
                        rp++;
                        if ((DWORD)(rp - data) >= (DWORD)fileSize)
                        {
                            goto done;
                        }
                    }
                    for (row = 0; row < 7; row++)
                    {
                        if (*rp == '@')
                        {
                            break;
                        }
                        memset(this->tunes[i].comment[row], 0, sizeof(this->tunes[i].comment[row]));
                        k = 0;
                        while (*rp != '\n' && *rp != '\r')
                        {
                            this->tunes[i].comment[row][k] = *rp;
                            rp++;
                            k++;
                            if ((DWORD)(rp - data) >= (DWORD)fileSize)
                            {
                                goto done;
                            }
                        }
                        while (*rp == '\n' || *rp == '\r')
                        {
                            rp++;
                            if ((DWORD)(rp - data) >= (DWORD)fileSize)
                            {
                                goto done;
                            }
                        }
                    }
                }
                else
                {
                    rp++;
                }
            }
        done:
            free(data);
            this->tuneCount = i + 1;
            for (i = 0; i < this->tuneCount; i++)
            {
                this->anm->StartScript(&this->sprites[i + 0x9f], i + 0x9f);
                if (g_PlayRecord.bgmHeard[i])
                {
                    g_SpriteEngine->RenderText(&this->sprites[i + 0x9f], 0xd0e0ff, 0x302080, this->tunes[i].title);
                }
                else
                {
                    char buf[6];

                    memcpy(buf, this->tunes[i].title, 5);
                    buf[5] = '\0';
                    g_SpriteEngine->RenderText(&this->sprites[i + 0x9f], 0x80a0a0, 0x100040, "%5s ？？？？？？？？", buf);
                }
                this->sprites[i + 0x9f].pos.x = 93.0f;
                this->sprites[i + 0x9f].pos.y = (i + 1) * 18 + 104.0f - 20.0f;
                this->sprites[i + 0x9f].pos.z = 0.0f;
                this->sprites[i + 0x9f].flags |= 0x1800;
            }
            this->SelectEntryInterrupts(this->cursor, 0x9f, this->tuneCount);
            for (i = 0; i < this->listTop; i++)
            {
                this->sprites[i + 0x9f].flags &= ~2;
            }
            end = this->listTop + 10 < this->tuneCount ? this->listTop + 10 : this->tuneCount;
            for (i = this->listTop; i < end; i++)
            {
                this->sprites[i + 0x9f].flags |= 2;
                this->sprites[i + 0x9f].pos.y = (i - this->listTop + 1) * 18 + 104.0f - 20.0f;
            }
            for (i = end; i <= 29; i++)
            {
                this->sprites[i + 0x9f].flags &= ~2;
            }
            this->playingTune = this->cursor;
            for (i = 0; i < 8; i++)
            {
                g_App.textAnm->StartScript(&this->commentSprites[i], i + 6);
            }
            this->musicPaused = 0;
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
            this->inputLockout = 0;
        }
        break;
    }
    for (i = 0; i < 8; i++)
    {
        g_SpriteEngine->Step(&this->commentSprites[i]);
    }
    this->stateFrames++;
    this->subMenuIdleFrames++;
    this->inputLockout++;
    return 1;
}

void MainMenu::SelectCursorEntry()
{
    this->SelectEntry(this->cursor, this->entryFirst, this->entryCount);
}

int MainMenu::UpdateState9()
{
    int i;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 3);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            this->subState = 0;
            this->stateFrames = 0;
            this->prevCursor = -1;
            this->entryCount = this->entryFirst = 9;
            this->SelectCursorEntry();
            this->unk_c918 = 1;
        }
        this->subState = 1;
        for (i = 0; i < ARRAY_SIZE(g_OptionsHelpTexts); i++)
        {
            g_SpriteEngine->RenderTextCentered(&this->helpSprites[i], 0xfff0e0, 0x300000, g_OptionsHelpTexts[i]);
        }
    case 1:
        if (g_App.config.bgmVolume >= 100)
        {
            this->anm->SetSprite(&this->sprites[26], this->sprites[26].baseSpriteIndex);
            this->sprites[26].alpha = 0xff;
            this->sprites[26].flags |= 2;
        }
        else
        {
            this->sprites[26].alpha = 0;
            this->sprites[26].flags &= ~2;
        }
        if (g_App.config.bgmVolume >= 10)
        {
            this->anm->SetSprite(&this->sprites[27],
                                 this->sprites[27].baseSpriteIndex + g_App.config.bgmVolume / 10 % 10 * 2);
            this->sprites[27].alpha = 0xff;
            this->sprites[27].flags |= 2;
        }
        else
        {
            this->sprites[27].alpha = 0;
            this->sprites[27].flags &= ~2;
        }
        this->anm->SetSprite(&this->sprites[28], this->sprites[28].baseSpriteIndex + g_App.config.bgmVolume % 10 * 2);
        this->anm->SetSprite(&this->sprites[29], this->sprites[29].baseSpriteIndex);
        if (g_App.config.seVolume >= 100)
        {
            this->anm->SetSprite(&this->sprites[30], this->sprites[30].baseSpriteIndex);
            this->sprites[30].alpha = 0xff;
            this->sprites[30].flags |= 2;
        }
        else
        {
            this->sprites[30].alpha = 0;
            this->sprites[30].flags &= ~2;
        }
        if (g_App.config.seVolume >= 10)
        {
            this->anm->SetSprite(&this->sprites[31],
                                 this->sprites[31].baseSpriteIndex + g_App.config.seVolume / 10 % 10 * 2);
            this->sprites[31].alpha = 0xff;
            this->sprites[31].flags |= 2;
        }
        else
        {
            this->sprites[31].alpha = 0;
            this->sprites[31].flags &= ~2;
        }
        if (this->MoveMenuCursor(9) != 0)
        {
            this->SelectCursorEntry();
        }
        if (this->prevCursor != this->cursor)
        {
            this->helpSprite = &this->helpSprites[this->cursor];
            this->helpSprite->SetPendingInterrupt(1);
        }
        this->prevCursor = this->cursor;
        if (this->inputLockout < 4)
        {
            break;
        }
        if (g_PlayerInputs[2].IsPressed(0x40))
        {
            switch (this->cursor)
            {
            case 0:
                g_SoundPlayer.QueueSound(12, 0);
                if (g_App.config.unk_ac == 0)
                {
                    g_App.config.unk_ac = 2;
                }
                else
                {
                    g_App.config.unk_ac--;
                }
                this->SelectEntry(g_App.config.unk_ac, 0x12, 3);
                break;
            case 1:
                g_SoundPlayer.QueueSound(12, 0);
                if (g_App.config.colorMode16Bits == 0)
                {
                    g_App.config.colorMode16Bits = 1;
                }
                else
                {
                    g_App.config.colorMode16Bits--;
                }
                this->SelectEntry(g_App.config.colorMode16Bits, 0x15, 2);
                break;
            case 2:
                g_SoundPlayer.QueueSound(12, 0);
                g_App.StopMusic();
                if (g_App.config.musicMode == 0)
                {
                    g_App.config.musicMode = 1;
                }
                else
                {
                    g_App.config.musicMode--;
                }
                g_App.LoadBgm(0);
                g_App.PlayMusicById(0, 0);
                this->SelectEntry(g_App.config.musicMode, 0x17, 3);
                break;
            case 5:
                g_SoundPlayer.QueueSound(12, 0);
                if (g_App.config.windowed == 0)
                {
                    g_App.config.windowed = 1;
                }
                else
                {
                    g_App.config.windowed--;
                }
                this->SelectEntry(g_App.config.windowed, 0x22, 2);
                break;
            default:
                goto volume;
            }
            this->PlaySoundNow(12, 0);
        }
    volume:
        if (g_PlayerInputs[2].unk_6 & 0x40)
        {
            switch (this->cursor)
            {
            case 3:
                g_App.config.bgmVolume -= 4;
                if (g_App.config.bgmVolume < 0)
                {
                    g_App.config.bgmVolume = 0;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            case 4:
                g_App.config.seVolume -= 4;
                if (g_App.config.seVolume < 0)
                {
                    g_App.config.seVolume = 0;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            }
        }
        if (g_PlayerInputs[2].unk_6 & 0x80)
        {
            switch (this->cursor)
            {
            case 3:
                g_App.config.bgmVolume += 4;
                if (g_App.config.bgmVolume > 100)
                {
                    g_App.config.bgmVolume = 100;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            case 4:
                g_App.config.seVolume += 4;
                if (g_App.config.seVolume > 100)
                {
                    g_App.config.seVolume = 100;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            }
        }
        if (g_PlayerInputs[2].unk_6 & 0x40)
        {
            switch (this->cursor)
            {
            case 3:
                if (g_App.config.bgmVolume > 0)
                {
                    g_App.config.bgmVolume--;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            case 4:
                if (g_App.config.seVolume > 0)
                {
                    g_App.config.seVolume--;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            }
        }
        if (g_PlayerInputs[2].unk_6 & 0x80)
        {
            switch (this->cursor)
            {
            case 3:
                if (g_App.config.bgmVolume < 100)
                {
                    g_App.config.bgmVolume++;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            case 4:
                if (g_App.config.seVolume < 100)
                {
                    g_App.config.seVolume++;
                }
                g_SoundPlayer.QueueSoundCommand(8, 0, "SetVol");
                break;
            }
        }
        g_SoundPlayer.bgmVolume = g_App.config.bgmVolume;
        g_SoundPlayer.seVolume = g_App.config.seVolume;
        if (g_PlayerInputs[2].IsPressed(0x80))
        {
            switch (this->cursor)
            {
            case 0:
                g_SoundPlayer.QueueSound(12, 0);
                if (g_App.config.unk_ac >= 2)
                {
                    g_App.config.unk_ac = 0;
                }
                else
                {
                    g_App.config.unk_ac++;
                }
                this->SelectEntry(g_App.config.unk_ac, 0x12, 3);
                break;
            case 1:
                g_SoundPlayer.QueueSound(12, 0);
                if (g_App.config.colorMode16Bits >= 1)
                {
                    g_App.config.colorMode16Bits = 0;
                }
                else
                {
                    g_App.config.colorMode16Bits++;
                }
                this->SelectEntry(g_App.config.colorMode16Bits, 0x15, 2);
                break;
            case 2:
                g_SoundPlayer.QueueSound(12, 0);
                g_App.StopMusic();
                if (g_App.config.musicMode >= 1)
                {
                    g_App.config.musicMode = 0;
                }
                else
                {
                    g_App.config.musicMode++;
                }
                g_App.LoadBgm(0);
                g_App.PlayMusicById(0, 0);
                this->SelectEntry(g_App.config.musicMode, 0x17, 3);
                break;
            case 5:
                g_SoundPlayer.QueueSound(12, 0);
                if (g_App.config.windowed >= 1)
                {
                    g_App.config.windowed = 0;
                }
                else
                {
                    g_App.config.windowed++;
                }
                this->SelectEntry(g_App.config.windowed, 0x22, 2);
                break;
            default:
                goto idle;
            }
            this->PlaySoundNow(12, 0);
        }
    idle:
        if (this->cursor >= 3 && this->cursor <= 4 && this->inputLockout % 50 == 0)
        {
            g_SoundPlayer.QueueSound(0x1d, 0);
        }
        this->subMenuIdleFrames = 0;
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            switch (this->cursor)
            {
            case 6:
                g_App.config.unk_ac = 0;
                g_App.config.musicMode = 1;
                g_App.config.playSounds = 1;
                this->PlaySoundNow(10, 0);
                this->SelectEntry(g_App.config.unk_ac, 0x12, 3);
                this->SelectEntry(g_App.config.colorMode16Bits, 0x15, 2);
                this->SelectEntry(g_App.config.musicMode, 0x17, 3);
                this->SelectEntry(g_App.config.windowed, 0x22, 2);
                break;
            case 7:
                this->cursor = 0;
                this->SetState(10);
                this->PlaySoundNow(10, 0);
                return 1;
            case 8:
                goto leave;
            }
        }
        if (g_PlayerInputs[2].unk_6 & 0xa)
        {
            if (this->cursor == 8)
            {
            leave:
                this->cursor = 6;
                this->SetState(1);
                this->UpdateState1();
                this->PlaySoundNow(11, 0);
                if (this->config.colorMode16Bits != g_App.config.colorMode16Bits ||
                    this->config.windowed != g_App.config.windowed)
                {
                    return 5;
                }
                return 1;
            }
            this->anm->SetSprite(&this->sprites[this->cursor + 9], this->sprites[this->cursor + 9].baseSpriteIndex + 1);
            this->cursor = 8;
            this->anm->SetSprite(&this->sprites[this->cursor + 9], this->sprites[this->cursor + 9].baseSpriteIndex);
            this->PlaySoundNow(11, 0);
        }
    }
    this->subMenuIdleFrames++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateState10()
{
    int i;
    int delta;
    Sprite *digit;
    BYTE *pad;
    short key;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 4);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            this->entryFirst = 0x25;
            this->entryCount = 10;
            this->SelectCursorEntry();
            this->subState = 0;
            this->stateFrames = 0;
            this->unk_c918 = 1;
            memcpy(this->mappings, g_App.config.mappings, sizeof(this->mappings));
            this->ShowEntryFrame(this->unk_c920, 0x2f);
            this->ShowEntryFrame(g_App.config.pads[this->unk_c920], 0x30);
            this->ShowEntryFrame(g_App.config.unk_b4[this->unk_c920], 0x31);
            digit = &this->sprites[50];
            this->ShowButtonDigits(digit, this->mappings[this->unk_c920][0].shootButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->mappings[this->unk_c920][0].bombButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->mappings[this->unk_c920][0].focusButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->mappings[this->unk_c920][0].skipButton);
            digit += 2;
            this->ShowButtonDigits(digit, this->mappings[this->unk_c920][0].menuButton);
            this->prevCursor = -1;
        }
        this->subState = 1;
        for (i = 0; i < ARRAY_SIZE(g_KeyConfigHelpTexts); i++)
        {
            g_SpriteEngine->RenderTextCentered(&this->helpSprites[i], 0xfff0e0, 0x300000, g_KeyConfigHelpTexts[i]);
        }
    case 1:
        if ((delta = this->MoveMenuCursor(10)) != 0)
        {
            if (g_App.config.pads[this->unk_c920] >= 2)
            {
                while (this->cursor >= 3 && this->cursor <= 7)
                {
                    this->cursor += delta;
                }
            }
            this->SelectCursorEntry();
        }
        if (this->prevCursor != this->cursor)
        {
            this->helpSprite = &this->helpSprites[this->cursor];
            this->helpSprite->SetPendingInterrupt(1);
        }
        this->prevCursor = this->cursor;
        digit = &this->sprites[50];
        this->ShowButtonDigits(digit, this->mappings[this->unk_c920][0].shootButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->mappings[this->unk_c920][0].bombButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->mappings[this->unk_c920][0].focusButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->mappings[this->unk_c920][0].skipButton);
        digit += 2;
        this->ShowButtonDigits(digit, this->mappings[this->unk_c920][0].menuButton);
        if (g_App.config.pads[this->unk_c920] < 2)
        {
            pad = GetPadButtons(g_App.config.pads[this->unk_c920]);
            for (key = 0; key < 0x20; key++)
            {
                if (pad[key] & 0x80)
                {
                    break;
                }
            }
            if (key < 0x20 && g_LastPressedKey != key)
            {
                switch (this->cursor)
                {
                case 3:
                    this->ReassignButton(&this->mappings[this->unk_c920][0], key,
                                         this->mappings[this->unk_c920][0].shootButton, 1);
                    this->mappings[this->unk_c920][0].shootButton = key;
                    break;
                case 4:
                    this->ReassignButton(&this->mappings[this->unk_c920][0], key,
                                         this->mappings[this->unk_c920][0].bombButton, 0);
                    this->mappings[this->unk_c920][0].bombButton = key;
                    break;
                case 5:
                    this->ReassignButton(&this->mappings[this->unk_c920][0], key,
                                         this->mappings[this->unk_c920][0].focusButton, 1);
                    this->mappings[this->unk_c920][0].focusButton = key;
                    break;
                case 6:
                    this->ReassignButton(&this->mappings[this->unk_c920][0], key,
                                         this->mappings[this->unk_c920][0].skipButton, 0);
                    this->mappings[this->unk_c920][0].skipButton = key;
                    break;
                case 7:
                    this->ReassignButton(&this->mappings[this->unk_c920][0], key,
                                         this->mappings[this->unk_c920][0].menuButton, 0);
                    this->mappings[this->unk_c920][0].menuButton = key;
                    break;
                default:
                    goto assigned;
                }
                this->PlaySoundNow(10, 0);
            }
        assigned:
            g_LastPressedKey = key;
        }
        if (g_PlayerInputs[2].IsPressed(0x40))
        {
            switch (this->cursor)
            {
            case 0:
                this->unk_c920 = 1 - this->unk_c920;
                this->ShowEntryFrame(this->unk_c920, 0x2f);
                this->ShowEntryFrame(g_App.config.unk_b4[this->unk_c920], 0x31);
                this->ShowEntryFrame(g_App.config.pads[this->unk_c920], 0x30);
                this->ShowEntryFrame(g_App.config.unk_b4[this->unk_c920], 0x2a);
                if (g_App.config.pads[this->unk_c920] >= 2)
                {
                    for (i = 0; i < 5; i++)
                    {
                        this->sprites[i + 40].color = 0xff606060;
                    }
                    for (i = 0; i < 10; i++)
                    {
                        this->sprites[i + 50].color = 0xff606060;
                    }
                }
                else
                {
                    for (i = 0; i < 5; i++)
                    {
                        this->sprites[i + 40].color = 0xffffffff;
                    }
                    for (i = 0; i < 10; i++)
                    {
                        this->sprites[i + 50].color = 0xffffffff;
                    }
                }
                g_SoundPlayer.QueueSound(12, 0);
                break;
            case 1:
                do
                {
                    if (g_App.config.pads[this->unk_c920] <= 0)
                    {
                        g_App.config.pads[this->unk_c920] += 5;
                    }
                    g_App.config.pads[this->unk_c920]--;
                } while (g_App.config.pads[1 - this->unk_c920] == 1 && g_App.config.pads[this->unk_c920] == 1 ||
                       g_App.config.pads[1 - this->unk_c920] == 0 && g_App.config.pads[this->unk_c920] == 0 ||
                       g_App.config.pads[1 - this->unk_c920] == 2 &&
                           (g_App.config.pads[this->unk_c920] == 2 || g_App.config.pads[this->unk_c920] == 3 ||
                            g_App.config.pads[this->unk_c920] == 4) ||
                       g_App.config.pads[1 - this->unk_c920] == 3 &&
                           (g_App.config.pads[this->unk_c920] == 2 || g_App.config.pads[this->unk_c920] == 3) ||
                       g_App.config.pads[1 - this->unk_c920] == 4 &&
                           (g_App.config.pads[this->unk_c920] == 2 || g_App.config.pads[this->unk_c920] == 4));
                this->ShowEntryFrame(g_App.config.pads[this->unk_c920], 0x30);
                if (g_App.config.pads[this->unk_c920] >= 2)
                {
                    for (i = 0; i < 5; i++)
                    {
                        this->sprites[i + 40].color = 0xff606060;
                    }
                    for (i = 0; i < 10; i++)
                    {
                        this->sprites[i + 50].color = 0xff606060;
                    }
                }
                else
                {
                    for (i = 0; i < 5; i++)
                    {
                        this->sprites[i + 40].color = 0xffffffff;
                    }
                    for (i = 0; i < 10; i++)
                    {
                        this->sprites[i + 50].color = 0xffffffff;
                    }
                }
                g_SoundPlayer.QueueSound(12, 0);
                break;
            case 2:
                if (g_App.config.unk_b4[this->unk_c920] <= 0)
                {
                    g_App.config.unk_b4[this->unk_c920] += 2;
                }
                g_App.config.unk_b4[this->unk_c920]--;
                this->ShowEntryFrame(g_App.config.unk_b4[this->unk_c920], 0x31);
                this->ShowEntryFrame(g_App.config.unk_b4[this->unk_c920], 0x2a);
                g_SoundPlayer.QueueSound(12, 0);
                break;
            default:
                break;
            }
        }
        if (g_PlayerInputs[2].IsPressed(0x80))
        {
            switch (this->cursor)
            {
            case 0:
                this->unk_c920 = 1 - this->unk_c920;
                this->ShowEntryFrame(this->unk_c920, 0x2f);
                this->ShowEntryFrame(g_App.config.unk_b4[this->unk_c920], 0x31);
                this->ShowEntryFrame(g_App.config.pads[this->unk_c920], 0x30);
                this->ShowEntryFrame(g_App.config.unk_b4[this->unk_c920], 0x2a);
                if (g_App.config.pads[this->unk_c920] >= 2)
                {
                    for (i = 0; i < 5; i++)
                    {
                        this->sprites[i + 40].color = 0xff606060;
                    }
                    for (i = 0; i < 10; i++)
                    {
                        this->sprites[i + 50].color = 0xff606060;
                    }
                }
                else
                {
                    for (i = 0; i < 5; i++)
                    {
                        this->sprites[i + 40].color = 0xffffffff;
                    }
                    for (i = 0; i < 10; i++)
                    {
                        this->sprites[i + 50].color = 0xffffffff;
                    }
                }
                g_SoundPlayer.QueueSound(12, 0);
                break;
            case 1:
                do
                {
                    g_App.config.pads[this->unk_c920]++;
                    if (g_App.config.pads[this->unk_c920] >= 5)
                    {
                        g_App.config.pads[this->unk_c920] -= 5;
                    }
                } while (g_App.config.pads[1 - this->unk_c920] == 1 && g_App.config.pads[this->unk_c920] == 1 ||
                       g_App.config.pads[1 - this->unk_c920] == 0 && g_App.config.pads[this->unk_c920] == 0 ||
                       g_App.config.pads[1 - this->unk_c920] == 2 &&
                           (g_App.config.pads[this->unk_c920] == 2 || g_App.config.pads[this->unk_c920] == 3 ||
                            g_App.config.pads[this->unk_c920] == 4) ||
                       g_App.config.pads[1 - this->unk_c920] == 3 &&
                           (g_App.config.pads[this->unk_c920] == 2 || g_App.config.pads[this->unk_c920] == 3) ||
                       g_App.config.pads[1 - this->unk_c920] == 4 &&
                           (g_App.config.pads[this->unk_c920] == 2 || g_App.config.pads[this->unk_c920] == 4));
                this->ShowEntryFrame(g_App.config.pads[this->unk_c920], 0x30);
                if (g_App.config.pads[this->unk_c920] >= 2)
                {
                    for (i = 0; i < 5; i++)
                    {
                        this->sprites[i + 40].color = 0xff606060;
                    }
                    for (i = 0; i < 10; i++)
                    {
                        this->sprites[i + 50].color = 0xff606060;
                    }
                }
                else
                {
                    for (i = 0; i < 5; i++)
                    {
                        this->sprites[i + 40].color = 0xffffffff;
                    }
                    for (i = 0; i < 10; i++)
                    {
                        this->sprites[i + 50].color = 0xffffffff;
                    }
                }
                g_SoundPlayer.QueueSound(12, 0);
                break;
            case 2:
                g_App.config.unk_b4[this->unk_c920]++;
                if (g_App.config.unk_b4[this->unk_c920] >= 2)
                {
                    g_App.config.unk_b4[this->unk_c920] -= 2;
                }
                this->ShowEntryFrame(g_App.config.unk_b4[this->unk_c920], 0x31);
                this->ShowEntryFrame(g_App.config.unk_b4[this->unk_c920], 0x2a);
                g_SoundPlayer.QueueSound(12, 0);
                break;
            default:
                break;
            }
        }
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            switch (this->cursor)
            {
            case 8:
                this->PlaySoundNow(10, 0);
                memcpy(this->mappings, g_App.config.mappings, sizeof(this->mappings));
                g_App.config.unk_b4[this->unk_c920] = 0;
                g_App.config.pads[this->unk_c920] = this->unk_c920;
                this->ShowEntryFrame(this->unk_c920, 0x2f);
                this->ShowEntryFrame(g_App.config.unk_b4[this->unk_c920], 0x31);
                this->ShowEntryFrame(g_App.config.pads[this->unk_c920], 0x30);
                this->ShowEntryFrame(g_App.config.unk_b4[this->unk_c920], 0x2a);
                if (g_App.config.pads[this->unk_c920] >= 2)
                {
                    for (i = 0; i < 5; i++)
                    {
                        this->sprites[i + 40].color = 0xff606060;
                    }
                    for (i = 0; i < 10; i++)
                    {
                        this->sprites[i + 50].color = 0xff606060;
                    }
                }
                else
                {
                    for (i = 0; i < 5; i++)
                    {
                        this->sprites[i + 40].color = 0xffffffff;
                    }
                    for (i = 0; i < 10; i++)
                    {
                        this->sprites[i + 50].color = 0xffffffff;
                    }
                }
                break;
            case 9:
                this->PlaySoundNow(11, 0);
                this->SetState(9);
                memcpy(g_App.config.mappings, this->mappings, sizeof(this->mappings));
                g_PlayerInputs[0].mappings[0] = g_App.config.mappings[0][0];
                g_PlayerInputs[1].mappings[0] = g_App.config.mappings[1][0];
                this->cursor = 7;
                return 1;
            }
        }
    }
    this->subMenuIdleFrames++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateState8()
{
    int i;
    int n;
    char characters[2];

    switch (this->subState)
    {
    case 1:
        if (g_GameManager.unk_114 == 4 && g_Network->unk_a8 == 0)
        {
            this->PlaySoundNow(11, 0);
            this->stateFrames = 0;
            if (this->state == 3)
            {
                this->SetState(2);
                return 1;
            }
            if (this->state == 5)
            {
                this->SetState(4);
                return 1;
            }
            this->SetState(7);
            return 1;
        }
        if (this->cursors[2] == 0)
        {
            if (!(g_PlayerInputs[0].unk_0 & 4))
            {
                if ((i = this->MovePlayerCursor(0, 16)) != 0)
                {
                    this->SkipClosedEntries(0, i, g_VsCharacters, 16);
                    characters[0] = g_VsCharacters[this->cursors[0]];
                    characters[1] = g_VsCharacters[this->cursors[1]];
                    if (i > 0)
                    {
                        this->SelectEntryStep(characters[0], 0x5c, 16, 2);
                        this->SelectEntryStep(characters[0], 0x3c, 16, 2);
                    }
                    else
                    {
                        this->SelectEntry2(characters[0], 0x5c, 16, 2);
                        this->SelectEntry2(characters[0], 0x3c, 16, 2);
                    }
                    this->MoveCharacterCursor(0, characters[0], characters[1], g_VsCharacters);
                }
            }
            else
            {
                if (g_PlayerInputs[0].IsPressed(0x40))
                {
                    if (this->unk_11b84 > 1)
                    {
                        this->unk_11b84--;
                    }
                    this->FlashHalves(this->unk_11b84, this->unk_11b88);
                }
                if (g_PlayerInputs[0].IsPressed(0x80))
                {
                    if (this->unk_11b84 < 10)
                    {
                        this->unk_11b84++;
                    }
                    this->FlashHalves(this->unk_11b84, this->unk_11b88);
                }
            }
            if (g_PlayerInputs[0].unk_0 & 0x100)
            {
                this->cursors[0] += g_Rng.GetRandomU32() & 0xf;
                if (this->cursors[0] >= 16)
                {
                    this->cursors[0] -= 16;
                }
                this->SkipClosedEntries(0, 1, g_VsCharacters, 16);
                characters[0] = g_VsCharacters[this->cursors[0]];
                characters[1] = g_VsCharacters[this->cursors[1]];
                this->SelectEntryStep(characters[0], 0x5c, 16, 2);
                this->SelectEntryStep(characters[0], 0x3c, 16, 2);
                this->MoveCharacterCursor(0, characters[0], characters[1], g_VsCharacters);
            }
        }
        if (this->cursors[3] == 0)
        {
            if (!(g_PlayerInputs[1].unk_0 & 4))
            {
                if ((i = this->MovePlayerCursor(1, 16)) != 0)
                {
                    this->SkipClosedEntries(1, i, g_VsCharacters, 16);
                    characters[0] = g_VsCharacters[this->cursors[0]];
                    characters[1] = g_VsCharacters[this->cursors[1]];
                    if (i < 0)
                    {
                        this->SelectEntryStep(characters[1], 0x5d, 16, 2);
                        this->SelectEntryStep(characters[1], 0x3d, 16, 2);
                    }
                    else
                    {
                        this->SelectEntry2(characters[1], 0x5d, 16, 2);
                        this->SelectEntry2(characters[1], 0x3d, 16, 2);
                    }
                    this->MoveCharacterCursor(1, characters[1], characters[0], g_VsCharacters);
                }
            }
            else
            {
                if (g_PlayerInputs[1].IsPressed(0x40))
                {
                    if (this->unk_11b88 > 1)
                    {
                        this->unk_11b88--;
                    }
                    this->FlashHalves(this->unk_11b84, this->unk_11b88);
                }
                if (g_PlayerInputs[1].IsPressed(0x80))
                {
                    if (this->unk_11b88 < 10)
                    {
                        this->unk_11b88++;
                    }
                    this->FlashHalves(this->unk_11b84, this->unk_11b88);
                }
            }
            if (g_PlayerInputs[1].unk_0 & 0x100)
            {
                this->cursors[1] += g_Rng.GetRandomU32() & 0xf;
                if (this->cursors[1] >= 16)
                {
                    this->cursors[1] -= 16;
                }
                this->SkipClosedEntries(1, 1, g_VsCharacters, 16);
                characters[0] = g_VsCharacters[this->cursors[0]];
                characters[1] = g_VsCharacters[this->cursors[1]];
                this->SelectEntryStep(characters[1], 0x5d, 16, 2);
                this->SelectEntryStep(characters[1], 0x3d, 16, 2);
                this->MoveCharacterCursor(1, characters[1], characters[0], g_VsCharacters);
            }
        }
        characters[0] = g_VsCharacters[this->cursors[0]];
        characters[1] = g_VsCharacters[this->cursors[1]];
        if ((g_PlayerInputs[0].unk_6 & 0x1001) && this->cursors[2] == 0)
        {
            this->PlaySoundNow(10, 0);
            this->cursors[2] = 1;
            i = characters[0];
            this->sprites[92 + i * 2].colorRgb[2] = 0x80;
            this->sprites[92 + i * 2].colorRgb[1] = 0x80;
            this->sprites[92 + i * 2].colorRgb[0] = 0x80;
            this->sprites[60 + i * 2].colorRgb[2] = 0x80;
            this->sprites[60 + i * 2].colorRgb[1] = 0x80;
            this->sprites[60 + i * 2].colorRgb[0] = 0x80;
            g_GameManager.sides[0].unk_30 = g_PlayerInputs[0].unk_0 >> 2 & 1;
            if (this->cursors[3] != 0 && characters[0] == characters[1])
            {
                g_GameManager.sides[0].unk_30 = 1 - g_GameManager.sides[1].unk_30;
            }
        }
        if ((g_PlayerInputs[1].unk_6 & 0x1001) && this->cursors[3] == 0)
        {
            this->PlaySoundNow(10, 0);
            this->cursors[3] = 1;
            this->sprites[characters[1] * 2 + 93].colorRgb[2] = 0x80;
            this->sprites[characters[1] * 2 + 93].colorRgb[1] = 0x80;
            this->sprites[characters[1] * 2 + 93].colorRgb[0] = 0x80;
            this->sprites[characters[1] * 2 + 61].colorRgb[2] = 0x80;
            this->sprites[characters[1] * 2 + 61].colorRgb[1] = 0x80;
            this->sprites[characters[1] * 2 + 61].colorRgb[0] = 0x80;
            g_GameManager.sides[1].unk_30 = g_PlayerInputs[1].unk_0 >> 2 & 1;
            if (this->cursors[2] != 0 && characters[0] == characters[1])
            {
                g_GameManager.sides[1].unk_30 = 1 - g_GameManager.sides[0].unk_30;
            }
        }
        if (this->cursors[2] != 0 && this->cursors[3] != 0)
        {
            g_GameManager.sides[0].character = characters[0];
            g_GameManager.sides[1].character = characters[1];
            g_GameManager.initialLives[0] = this->unk_11b84;
            g_GameManager.initialLives[1] = this->unk_11b88;
            this->PlaySoundNow(10, 0);
            this->SetState(16);
            this->UpdateState16();
            return 1;
        }
        if ((g_PlayerInputs[0].unk_6 & 0xa) && this->cursors[2] == 0 ||
            (g_PlayerInputs[1].unk_6 & 0xa) && this->cursors[3] == 0)
        {
            g_GameManager.difficulty = this->cursor;
            this->PlaySoundNow(11, 0);
            this->stateFrames = 0;
            g_GameManager.sides[0].character = characters[0];
            g_GameManager.sides[1].character = characters[1];
            if (this->state == 3)
            {
                this->SetState(2);
                return 1;
            }
            if (this->state == 5)
            {
                this->SetState(4);
                return 1;
            }
            this->SetState(7);
            return 1;
        }
        if ((g_PlayerInputs[0].unk_6 & 0xa) && this->cursors[2] == 1)
        {
            this->cursors[2] = 0;
            this->PlaySoundNow(11, 0);
            this->sprites[characters[0] * 2 + 92].colorRgb[2] = 0xff;
            this->sprites[characters[0] * 2 + 92].colorRgb[1] = 0xff;
            this->sprites[characters[0] * 2 + 92].colorRgb[0] = 0xff;
            this->sprites[characters[0] * 2 + 60].colorRgb[2] = 0xff;
            this->sprites[characters[0] * 2 + 60].colorRgb[1] = 0xff;
            this->sprites[characters[0] * 2 + 60].colorRgb[0] = 0xff;
        }
        if ((g_PlayerInputs[1].unk_6 & 0xa) && this->cursors[3] == 1)
        {
            this->cursors[3] = 0;
            this->PlaySoundNow(11, 0);
            this->sprites[characters[1] * 2 + 93].colorRgb[2] = 0xff;
            this->sprites[characters[1] * 2 + 93].colorRgb[1] = 0xff;
            this->sprites[characters[1] * 2 + 93].colorRgb[0] = 0xff;
            this->sprites[characters[1] * 2 + 61].colorRgb[2] = 0xff;
            this->sprites[characters[1] * 2 + 61].colorRgb[1] = 0xff;
            this->sprites[characters[1] * 2 + 61].colorRgb[0] = 0xff;
        }
        break;
    case 0:
        if (this->inputLockout == 0)
        {
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 13);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            n = 0;
            for (i = 0; i < 16; i++)
            {
                if (g_PlayRecord.openEntries[g_VsCharacters[i]] || g_Network->unk_a8)
                {
                    this->anm->SetSprite(&this->sprites[n + 191], g_VsCharacters[i] + 0xeb);
                    this->sprites[n + 191].flags |= 2;
                    n++;
                }
            }
            for (; n < 16; n++)
            {
                this->sprites[n + 191].flags &= ~2;
            }
            this->sprites[189].flags |= 2;
            this->sprites[190].flags |= 2;
            for (this->cursors[0] = 0; g_VsCharacters[this->cursors[0]] != g_GameManager.sides[0].character;
                 this->cursors[0]++)
            {
            }
            for (this->cursors[1] = 0; g_VsCharacters[this->cursors[1]] != g_GameManager.sides[1].character;
                 this->cursors[1]++)
            {
            }
            this->SkipClosedEntries(0, 1, g_VsCharacters, 16);
            this->SkipClosedEntries(1, 1, g_VsCharacters, 16);
            characters[0] = g_VsCharacters[this->cursors[0]];
            characters[1] = g_VsCharacters[this->cursors[1]];
            this->SelectEntryStep(g_VsCharacters[this->cursors[0]], 0x5c, 16, 2);
            this->SelectEntryStep(g_VsCharacters[this->cursors[1]], 0x5d, 16, 2);
            this->SelectEntryStep(g_VsCharacters[this->cursors[0]], 0x3c, 16, 2);
            this->SelectEntryStep(g_VsCharacters[this->cursors[1]], 0x3d, 16, 2);
            this->unk_c918 = 1;
            this->cursors[3] = 0;
            this->cursors[2] = 0;
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
            this->unk_11b84 = g_GameManager.initialLives[0];
            this->unk_11b88 = g_GameManager.initialLives[1];
            this->FlashHalves(this->unk_11b84, this->unk_11b88);
            this->MoveCharacterCursor(0, characters[0], characters[1], g_VsCharacters);
            this->MoveCharacterCursor(1, characters[1], characters[0], g_VsCharacters);
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    }
    this->stateFrames++;
    this->subMenuIdleFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateState14()
{
    char c;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            g_App.StopMusic();
            if (g_SpriteEngine->LoadSurface(0, "title/result00.png") != 0)
            {
                return 0;
            }
            if (this->spriteCount == 0)
            {
                this->spriteCount = 0xdf;
                this->sprites = new Sprite[this->spriteCount];
                this->anm->StartSpriteSeries(this->sprites, 0, this->spriteCount);
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 16);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            if (g_GameManager.mode == 2)
            {
                this->stateFrames = 0;
                this->SetState(15);
                return 1;
            }
            this->scoreCharacter = g_GameManager.sides[0].character;
            this->replayNameCursor = 0;
            this->SelectEntry3(this->scoreCharacter, 0x3c, 0x10, 2);
            this->cursor = 0x5f;
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
            this->scoreRank = InsertHighScore(&g_Hscr);
            if (this->scoreRank >= 5)
            {
                this->cursor = 0x5f;
            }
            else
            {
                strcpy(g_HighScores[this->scoreCharacter][g_GameManager.difficulty][this->scoreRank].name,
                       g_Lsnm.name);
            }
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
        if (this->scoreRank < 5)
        {
            if (g_PlayerInputs[2].IsPressed(0x10))
            {
                g_SoundPlayer.QueueSound(12, 0);
                this->cursor -= 16;
                if (this->cursor < 0)
                {
                    this->cursor += 0x60;
                }
                if (this->cursor == 0x5d)
                {
                    this->cursor = 0x4d;
                }
                this->stateFrames = 0;
            }
            if (g_PlayerInputs[2].IsPressed(0x20))
            {
                g_SoundPlayer.QueueSound(12, 0);
                this->cursor += 16;
                if (this->cursor >= 0x60)
                {
                    this->cursor -= 0x60;
                }
                if (this->cursor == 0x5d)
                {
                    this->cursor = 0xd;
                }
                this->stateFrames = 0;
            }
            if (g_PlayerInputs[2].IsPressed(0x40))
            {
                g_SoundPlayer.QueueSound(12, 0);
                if (this->cursor % 16 == 0)
                {
                    this->cursor += 15;
                }
                else
                {
                    this->cursor--;
                    if (this->cursor == 0x5d)
                    {
                        this->cursor = 0x5c;
                    }
                }
                this->stateFrames = 0;
            }
            if (g_PlayerInputs[2].IsPressed(0x80))
            {
                g_SoundPlayer.QueueSound(12, 0);
                if (this->cursor % 16 == 15)
                {
                    this->cursor -= 15;
                }
                else
                {
                    this->cursor++;
                    if (this->cursor == 0x5d)
                    {
                        this->cursor = 0x5e;
                    }
                }
                this->stateFrames = 0;
            }
        }
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            if (this->cursor == 0x5f)
            {
                SaveScoreDat();
                this->PlaySoundNow(11, 0);
                this->stateFrames = 0;
                this->SetState(15);
                if (this->scoreRank < 5)
                {
                    strcpy(g_Lsnm.name,
                           g_HighScores[this->scoreCharacter][g_GameManager.difficulty][this->scoreRank].name);
                }
                return 1;
            }
            if (this->cursor == 0x5e)
            {
                c = ' ';
            }
            else
            {
                c = g_NameAlphabet[this->cursor];
            }
            g_HighScores[this->scoreCharacter][g_GameManager.difficulty][this->scoreRank]
                .name[this->replayNameCursor] = c;
            this->PlaySoundNow(10, 0);
            if (this->replayNameCursor < 7)
            {
                this->replayNameCursor++;
            }
            else
            {
                this->cursor = 0x5f;
            }
            this->stateFrames = 0;
        }
        if (g_PlayerInputs[2].unk_6 & 10)
        {
            this->PlaySoundNow(11, 0);
            if (this->replayNameCursor == 7 &&
                g_HighScores[this->scoreCharacter][g_GameManager.difficulty][this->scoreRank].name[7] != ' ')
            {
                g_HighScores[this->scoreCharacter][g_GameManager.difficulty][this->scoreRank].name[7] = ' ';
            }
            else if (this->replayNameCursor > 0)
            {
                this->replayNameCursor--;
                g_HighScores[this->scoreCharacter][g_GameManager.difficulty][this->scoreRank]
                    .name[this->replayNameCursor] = ' ';
            }
            this->stateFrames = 0;
        }
        break;
    }
    this->stateFrames++;
    this->subMenuIdleFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateState15()
{
    int i;
    char path[0x100];
    char slotPath[0x40];
    int size;
    void *data;
    ReplayHeader *replay;

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            if (g_GameManager.stateFlags & 0x2000)
            {
                this->stateFrames = 0;
                this->SetState(1);
                this->cursor = 0;
                return 1;
            }
            if (g_Network->unk_a8 != 0)
            {
                for (i = 0; i < 1000; i++)
                {
                    sprintf(path, "./replay/th9_udn%.3d.rpy", i + 1);
                    if (!FileExists(path))
                    {
                        break;
                    }
                }
                if (i >= 1000)
                {
                    i = 999;
                }
                sprintf(path, "./replay/th9_udn%.3d.rpy", i + 1);
                ReplayManager::SaveReplay(g_GameManager.replayManager, path, g_Lsnm.name);
                this->stateFrames = 0;
                this->SetState(1);
                this->cursor = 0;
                this->unk_1b23c = 2;
                return 1;
            }
            if (g_SpriteEngine->LoadSurface(0, "title/replay00.png") != 0)
            {
                return 0;
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 0x13);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            this->cursor = 0;
            strcpy(this->replayName, g_Lsnm.name);
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
            if (g_GameManager.mode == 2)
            {
            listSlots:
                memset(this->replays, 0, sizeof(this->replays));
                for (i = 0; i < 25; i++)
                {
                    sprintf(slotPath, "./replay/th9_%.2d.rpy", i + 1);
                    data = LoadFileData(slotPath, &size, 1);
                    if (data != NULL && (replay = ValidateReplay(data, size)) != NULL)
                    {
                        this->replays[i] = *replay;
                        strcpy(this->replayPaths[i], slotPath);
                        free(replay);
                    }
                }
                this->subState = 2;
                this->inputLockout = 0;
                this->cursor = 0;
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 0x16);
                g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
                break;
            }
            this->entryFirst = 0x7d;
            this->entryCount = 2;
            this->SelectCursorEntry();
            _mkdir("replay");
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
        if (this->MoveMenuCursorLR(2) != 0)
        {
            this->SelectCursorEntry();
        }
        if (g_PlayerInputs[2].unk_6 & 0xa)
        {
            if (this->cursor == 0)
            {
                this->PlaySoundNow(11, 0);
                this->cursor = 1;
                this->SelectCursorEntry();
                break;
            }
            goto quit;
        }
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            if (this->cursor == 1)
            {
            quit:
                this->PlaySoundNow(11, 0);
                this->stateFrames = 0;
                this->SetState(1);
                this->cursor = 0;
                strcpy(g_Lsnm.name, g_HighScores[this->scoreCharacter][g_GameManager.difficulty][this->scoreRank].name);
                return 1;
            }
            goto listSlots;
        }
        break;
    case 2:
        if (this->inputLockout < 6)
        {
            break;
        }
        if (this->MoveMenuCursor(25) != 0)
        {
            this->SelectCursorEntry();
        }
        if (g_PlayerInputs[2].unk_6 & 0xa)
        {
            this->PlaySoundNow(11, 0);
            this->cursor = 0;
            this->inputLockout = 0;
            this->subState = 1;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 0x13);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            this->entryFirst = 0x7d;
            this->entryCount = 2;
            this->SelectCursorEntry();
            break;
        }
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            this->PlaySoundNow(10, 0);
            this->replaySlot = this->cursor;
            this->cursor = 0;
            if (*(DWORD *)this->replays[this->replaySlot].magic != 0)
            {
                this->subState = 3;
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 0x15);
                g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
                this->entryFirst = 0x81;
                this->entryCount = 2;
                this->SelectCursorEntry();
            }
            else
            {
                this->cursor = 0x5f;
                this->subState = 4;
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 0x17);
                g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            }
            this->inputLockout = 0;
            strcpy(this->replayName, g_Lsnm.name);
        }
        break;
    case 3:
        if (this->inputLockout < 10)
        {
            break;
        }
        if (this->MoveMenuCursorLR(2) != 0)
        {
            this->SelectCursorEntry();
        }
        if (g_PlayerInputs[2].unk_6 & 0xa)
        {
            goto back;
        }
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            if (this->cursor == 1)
            {
            back:
                this->PlaySoundNow(11, 0);
                this->subState = 2;
                this->cursor = 0;
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 0x16);
                g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            }
            else
            {
                this->PlaySoundNow(10, 0);
                this->cursor = 0x5f;
                this->subState = 4;
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 0x17);
                g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            }
        }
        break;
    case 4:
        if (this->inputLockout < 10)
        {
            break;
        }
        if (g_PlayerInputs[2].IsPressed(0x10))
        {
            g_SoundPlayer.QueueSound(12, 0);
            this->cursor -= 16;
            if (this->cursor < 0)
            {
                this->cursor += 0x60;
            }
            if (this->cursor == 0x5d)
            {
                this->cursor = 0x4d;
            }
            this->stateFrames = 0;
        }
        if (g_PlayerInputs[2].IsPressed(0x20))
        {
            g_SoundPlayer.QueueSound(12, 0);
            this->cursor += 16;
            if (this->cursor >= 0x60)
            {
                this->cursor -= 0x60;
            }
            if (this->cursor == 0x5d)
            {
                this->cursor = 0xd;
            }
            this->stateFrames = 0;
        }
        if (g_PlayerInputs[2].IsPressed(0x40))
        {
            g_SoundPlayer.QueueSound(12, 0);
            if (this->cursor % 16 == 0)
            {
                this->cursor += 15;
            }
            else
            {
                this->cursor--;
                if (this->cursor == 0x5d)
                {
                    this->cursor = 0x5c;
                }
            }
            this->stateFrames = 0;
        }
        if (g_PlayerInputs[2].IsPressed(0x80))
        {
            g_SoundPlayer.QueueSound(12, 0);
            if (this->cursor % 16 == 15)
            {
                this->cursor -= 15;
            }
            else
            {
                this->cursor++;
                if (this->cursor == 0x5d)
                {
                    this->cursor = 0x5e;
                }
            }
            this->stateFrames = 0;
        }
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            if (this->cursor == 0x5f)
            {
                this->PlaySoundNow(10, 0);
                sprintf(path, "replay/th9_%.2d.rpy", this->replaySlot + 1);
                ReplayManager::SaveReplay(g_GameManager.replayManager, path, this->replayName);
                this->stateFrames = 0;
                this->SetState(1);
                this->cursor = 0;
                strcpy(g_Lsnm.name, this->replayName);
                return 1;
            }
            this->replayName[this->replayNameCursor] = this->cursor == 0x5e ? ' ' : g_NameAlphabet[this->cursor];
            this->PlaySoundNow(10, 0);
            if (this->replayNameCursor < 7)
            {
                this->replayNameCursor++;
            }
            else
            {
                this->cursor = 0x5f;
            }
            this->stateFrames = 0;
        }
        if (g_PlayerInputs[2].unk_6 & 0xa)
        {
            this->PlaySoundNow(11, 0);
            if (this->replayNameCursor == 7 && this->replayName[7] != ' ')
            {
                this->replayName[7] = ' ';
            }
            else if (this->replayNameCursor > 0)
            {
                this->replayNameCursor--;
                this->replayName[this->replayNameCursor] = ' ';
            }
            else
            {
                this->cursor = 0;
                this->subState = 2;
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 0x16);
                g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
                this->inputLockout = 0;
            }
            this->stateFrames = 0;
        }
        break;
    }
    this->stateFrames++;
    this->subMenuIdleFrames++;
    this->inputLockout++;
    return 1;
}

MainMenu::MainMenu()
{
    memset(this, 0, sizeof(MainMenu));
}

int MainMenu::UpdateState1()
{
    int delta;
#ifdef TH09_NATIVE_NETPLAY
    int netResult = thnet::TitleUpdate(this);
    if (netResult >= 0) return netResult;
#endif

    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            if (this->prevState == 0 || this->prevState == 14 || this->prevState == 12 || this->prevState == 15)
            {
                g_App.PlayMusicById(0, 0);
            }
            if ((this->prevState == 0 || this->prevState == 13 || this->prevState == 14 || this->prevState == 11 ||
                 this->prevState == 15 || this->prevState == 12 || this->prevState == 2 || this->prevState == 4 ||
                 this->prevState == 7 || this->prevState == 6) &&
                g_SpriteEngine->LoadSurface(0, "title/title00.png") != 0)
            {
                return 0;
            }
            ReplayManager::Release(g_GameManager.replayManager);
            g_GameManager.replayManager = NULL;
            if (this->spriteCount == 0)
            {
                this->spriteCount = 0xdf;
                this->sprites = new Sprite[this->spriteCount];
                this->anm->StartSpriteSeries(this->sprites, 0, this->spriteCount);
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 2);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            if (!g_PlayRecord.openEntries3[12])
            {
                this->sprites[2].color = 0x30908040;
            }
            if (this->unk_1b23c != 0)
            {
                g_GameManager.stateFlags &= ~8;
                this->cursor = g_GameManager.unk_114;
                this->SetState(6);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                return 1;
            }
            this->entryFirst = 1;
            this->entryCount = 8;
            this->SelectCursorEntry();
            if (g_GameManager.stateFlags & 8)
            {
                this->SetState(11);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                return 1;
            }
            g_GameManager.stateFlags &= ~2;
        }
        if (this->inputLockout < ARRAY_SIZE(g_TitleHelpTexts))
        {
            g_SpriteEngine->RenderTextCentered(&this->helpSprites[this->inputLockout], 0xfff0e0, 0x300000,
                                               g_TitleHelpTexts[this->inputLockout]);
            this->inputLockout++;
            return 1;
        }
        this->prevCursor = -1;
        this->inputLockout = 0;
        this->stateFrames = 0;
        this->subState = 1;
        this->idleFrames = 0;
    case 1:
        delta = this->MoveMenuCursor(8);
        if (delta != 0)
        {
            if (!g_PlayRecord.openEntries3[12] && this->cursor == 1)
            {
                this->cursor += delta;
            }
            this->SelectCursorEntry();
        }
        if (this->prevCursor != this->cursor)
        {
            this->helpSprite = &this->helpSprites[this->cursor];
            this->helpSprite->SetPendingInterrupt(1);
        }
        this->prevCursor = this->cursor;
        if (!g_PlayRecord.openEntries3[12])
        {
            this->sprites[2].color = 0xb0b0b040;
        }
        if (this->inputLockout < 10)
        {
            break;
        }
        if (g_PlayerInputs[2].unk_0 != 0)
        {
            this->idleFrames = 0;
        }
        this->idleFrames++;
        if (this->idleFrames > 1500)
        {
            g_GameManager.stateFlags |= 2;
            strcpy(g_GameManager.replayFile, g_DemoReplayPaths[g_GameManager.demoIndex]);
            g_GameManager.demoIndex++;
            g_GameManager.demoIndex %= 3;
            g_GameManager.stateFlags |= 8;
            g_GameManager.unk_fc = 9;
            g_GameManager.difficulty = 3;
            g_GameManager.sides[0].unk_28 = 0;
            g_GameManager.sides[1].unk_28 = 1;
            g_App.scene = 2;
            g_GameManager.mode = 2;
            return 0;
        }
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            switch (this->cursor)
            {
            case 0:
                g_GameManager.stateFlags &= ~8;
                this->PlaySoundNow(10, 0);
                if (this->cursor >= 4)
                {
                    this->cursor = 2;
                }
                this->SetState(2);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                g_GameManager.initialLives[0] = 10;
                g_GameManager.initialLives[1] = 10;
                return 1;
            case 1:
                if (g_PlayRecord.openEntries3[12])
                {
                    g_GameManager.stateFlags &= ~8;
                    this->PlaySoundNow(10, 0);
                    if (this->cursor >= 4)
                    {
                        this->cursor = 2;
                    }
                    this->SetState(4);
                    g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                    g_GameManager.initialLives[0] = 10;
                    g_GameManager.initialLives[1] = 10;
                    return 1;
                }
                this->PlaySoundNow(0x27, 0);
                return 1;
            case 2:
                g_GameManager.stateFlags &= ~8;
                this->PlaySoundNow(10, 0);
                this->cursor = g_GameManager.unk_114;
                this->SetState(6);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                g_GameManager.initialLives[0] = 10;
                g_GameManager.initialLives[1] = 10;
                return 1;
            case 3:
                this->PlaySoundNow(10, 0);
                this->SetState(11);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                this->helpSprite->SetPendingInterrupt(2);
                return 1;
            case 5:
                this->PlaySoundNow(10, 0);
                this->SetState(12);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                this->helpSprite->SetPendingInterrupt(2);
                return 1;
            case 4:
                this->PlaySoundNow(10, 0);
                this->SetState(13);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                return 1;
            case 6:
                this->PlaySoundNow(10, 0);
                this->subState = 0;
                this->cursor = 0;
                this->inputLockout = 0;
                this->stateFrames = 0;
                this->UpdateState9();
                this->SelectEntry(g_App.config.unk_ac, 0x12, 5);
                this->SelectEntry(g_App.config.colorMode16Bits, 0x15, 2);
                this->SelectEntry(g_App.config.musicMode, 0x17, 3);
                this->SelectEntry(g_App.config.windowed, 0x22, 2);
                this->subState = 3;
                this->stateFrames = 0;
                this->unk_c920 = 0;
                this->cursor = 0;
                break;
            case 7:
                this->subState = 2;
                this->stateFrames = 0;
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 1);
                if (g_App.config.musicMode == 2)
                {
                    g_App.midi->Restart(30);
                }
                break;
            }
        }
        if (g_PlayerInputs[2].unk_6 & 10)
        {
            this->anm->SetSprite(&this->sprites[this->cursor + 1], this->sprites[this->cursor + 1].baseSpriteIndex + 1);
            this->cursor = 7;
            this->anm->SetSprite(&this->sprites[this->cursor + 1], this->sprites[this->cursor + 1].baseSpriteIndex);
            this->PlaySoundNow(11, 0);
        }
        break;
    case 2:
        if (this->stateFrames >= 60)
        {
            delete[] this->sprites;
            this->sprites = NULL;
            this->spriteCount = 0;
            this->inputLockout = 0;
            g_App.scene = -1;
            return 0;
        }
        break;
    case 3:
        if (this->stateFrames >= 30)
        {
            this->SetState(9);
            this->cursor = 0;
            this->config = g_App.config;
            return 1;
        }
    }
    this->subMenuIdleFrames++;
    this->stateFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateState2()
{
    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            if (g_SpriteEngine->LoadSurface(0, "title/select00.png") != 0)
            {
                return 0;
            }
            if (this->state == 4)
            {
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 25);
            }
            else
            {
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 12);
            }
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            if (this->state == 7)
            {
                this->sprites[0x83 + g_GameManager.unk_114].pendingInterrupt = 9;
                if (this->unk_1b23c == 1)
                {
                    this->SetState(8);
                    switch (g_GameManager.unk_114)
                    {
                    case 0:
                        this->UpdateState8();
                        break;
                    case 1:
                        this->UpdateState8b();
                        break;
                    case 2:
                        this->UpdateState8b();
                        break;
                    case 3:
                        this->UpdateState8b();
                        break;
                    }
                    this->sprites[0x88 + g_App.config.difficulty].pendingInterrupt = 10;
                    this->unk_1b23c = 0;
                    return 1;
                }
                if (this->unk_1b23c == 2)
                {
                    this->UpdateState8();
                    this->sprites[0x88 + g_App.config.difficulty].pendingInterrupt = 10;
                    this->unk_1b23c = 0;
                    this->SetState(8);
                    return 1;
                }
            }
            this->cursor = g_App.config.difficulty;
            if (this->state == 4)
            {
                this->cursor = 4;
            }
            else if (this->cursor >= 4)
            {
                this->cursor = 1;
            }
            this->entryFirst = 0x88;
            this->entryCount = 4;
            this->SelectCursorEntry();
            this->unk_c918 = 1;
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
        if (this->state != 4)
        {
            if (this->MoveMenuCursor(4) != 0)
            {
                this->SelectCursorEntry();
            }
        }
        else if (g_PlayerInputs[2].unk_6 & 0x30)
        {
            this->PlaySoundNow(12, 0);
        }
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            g_App.config.difficulty = this->cursor;
            this->PlaySoundNow(10, 0);
            if (this->state == 7)
            {
                this->SetState(8);
                switch (g_GameManager.unk_114)
                {
                case 0:
                    this->UpdateState8();
                    break;
                case 1:
                    this->UpdateState8b();
                    break;
                case 2:
                    this->UpdateState8b();
                    break;
                case 3:
                    this->UpdateState8b();
                    break;
                case 4:
                    this->UpdateState8();
                    break;
                }
                this->sprites[0x88 + g_App.config.difficulty].pendingInterrupt = 10;
                return 1;
            }
            if (this->state == 2)
            {
                this->SetState(3);
            }
            else
            {
                this->SetState(5);
            }
            this->UpdateState3();
            this->sprites[0x88 + g_App.config.difficulty].pendingInterrupt = 9;
            return 1;
        }
        if (g_GameManager.unk_114 == 4 && g_Network->unk_a8 == 0)
        {
            this->PlaySoundNow(11, 0);
            this->stateFrames = 0;
            if (this->state == 7)
            {
                this->SetState(6);
                return 1;
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 6);
            this->subState = 3;
        }
        if (g_Network->unk_a8 == 0 && (g_PlayerInputs[2].unk_6 & 0xa))
        {
            g_App.config.difficulty = this->cursor;
            this->PlaySoundNow(11, 0);
            this->stateFrames = 0;
            if (this->state == 7)
            {
                this->SetState(6);
                return 1;
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 6);
            this->subState = 3;
        }
        break;
    case 3:
        if (this->stateFrames >= 20)
        {
            int state = this->state;
            this->SetState(1);
            this->cursor = state != 2;
            return 1;
        }
        break;
    }
    this->stateFrames++;
    this->subMenuIdleFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::UpdateState6()
{
    switch (this->subState)
    {
    case 0:
        if (this->inputLockout == 0)
        {
            if (this->prevState != 7 && g_SpriteEngine->LoadSurface(0, "title/select00.png") != 0)
            {
                return 0;
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 11);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            if (this->unk_1b23c != 0)
            {
                this->SetState(7);
                this->UpdateState2();
                this->sprites[0x83 + g_GameManager.unk_114].pendingInterrupt = 9;
                return 1;
            }
            this->cursor = g_GameManager.unk_114;
            this->entryFirst = 0x83;
            this->entryCount = 5;
            this->SelectCursorEntry();
            this->unk_c918 = 1;
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
        }
        if (this->inputLockout == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
        if (this->MoveMenuCursor(5) != 0)
        {
            this->SelectCursorEntry();
        }
        if (g_PlayerInputs[2].unk_6 & 0x1001)
        {
            g_GameManager.unk_114 = this->cursor;
            if (g_GameManager.unk_114 == 4)
            {
#ifdef TH09_NATIVE_NETPLAY
                thnet::OpenLobby(this);
                return 1;
#else
                g_Network->Init();
                g_App.ResetRun();
                g_App.config.difficulty = 1;
                g_GameManager.sides[0].character = 0;
                g_GameManager.sides[1].character = 1;
#endif
            }
            this->PlaySoundNow(10, 0);
            this->SetState(7);
            this->UpdateState2();
            this->sprites[0x83 + g_GameManager.unk_114].pendingInterrupt = 9;
            return 1;
        }
        if (g_PlayerInputs[2].unk_6 & 0xa)
        {
            g_GameManager.unk_114 = this->cursor;
            this->PlaySoundNow(11, 0);
            this->subState = 3;
            this->stateFrames = 0;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 6);
        }
        break;
    case 3:
        if (this->stateFrames >= 20)
        {
            this->SetState(1);
            this->cursor = 2;
            return 1;
        }
        break;
    }
    this->stateFrames++;
    this->subMenuIdleFrames++;
    this->inputLockout++;
    return 1;
}

int MainMenu::OnUpdate(MainMenu *menu)
{
    int ret;

    ret = 1;
    if (menu->loadingState != 0)
    {
        return menu->loadingState == 2 ? 4 : 1;
    }
    switch (menu->state)
    {
#ifdef TH09_NATIVE_NETPLAY
    case 17:
        ret = thnet::LobbyUpdate(menu);
        break;
    case 18:
        ret = coop::UpdateSetup(menu);
        break;
#endif
    case 1:
        ret = menu->UpdateState1();
        break;
    case 6:
        ret = menu->UpdateState6();
        break;
    case 2:
    case 4:
    case 7:
        ret = menu->UpdateState2();
        break;
    case 3:
    case 5:
        ret = menu->UpdateState3();
        break;
    case 8:
        switch (g_GameManager.unk_114)
        {
        case 0:
            ret = menu->UpdateState8();
            break;
        case 1:
            ret = menu->UpdateState8b();
            break;
        case 2:
            ret = menu->UpdateState8b();
            break;
        case 3:
            ret = menu->UpdateState8b();
            break;
        case 4:
            ret = menu->UpdateState8();
            break;
        }
        break;
    case 16:
        ret = menu->UpdateState16();
        break;
    case 9:
        ret = menu->UpdateState9();
        break;
    case 10:
        ret = menu->UpdateState10();
        break;
    case 11:
        ret = menu->UpdateState11();
        break;
    case 15:
        ret = menu->UpdateState15();
        break;
    case 12:
        ret = menu->UpdateState12();
        break;
    case 14:
        ret = menu->UpdateState14();
        break;
    case 13:
        ret = menu->UpdateState13();
        break;
    }
    g_SpriteEngine->StepSprites(menu->sprites, menu->spriteCount);
    if (menu->helpSprite != NULL)
    {
        g_SpriteEngine->Step(menu->helpSprite);
    }
    return ret;
}

int SetUpTitleScene(int arg)
{
    MainMenu *menu;

    menu = new MainMenu;
    g_MainMenu = menu;
    memset(menu, 0, sizeof(MainMenu));
    g_GameManager.isPaused = 0;
    menu->unk_1b22c = arg;
    menu->calcElem = g_Chain.CreateElem((ChainCallback)MainMenu::OnUpdate);
    menu->calcElem->arg = menu;
    menu->calcElem->addedCallback = (ChainCallback)MainMenu::AddedCallback;
    menu->calcElem->deletedCallback = (ChainCallback)MainMenu::DeletedCallback;
    if (g_Chain.AddToCalcChain(menu->calcElem, 4) != 0)
    {
        return -1;
    }
    menu->drawElem = g_Chain.CreateElem((ChainCallback)MainMenu::OnDraw);
    menu->drawElem->arg = menu;
    g_Chain.AddToDrawChain(menu->drawElem, 3);
    return 0;
}
