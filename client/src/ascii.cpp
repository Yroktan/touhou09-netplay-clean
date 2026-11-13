

#include <stdio.h>
#include <string.h>

#include "ascii.h"
#include "gamemanager.h"
#include "player.h"
#include "sound.h"
#include "unsorted.h"
#ifdef TH09_NATIVE_NETPLAY
#include "NetGame.h"
#include "CoopGame.h"
#include "UiText.h"
#endif

int AsciiText::DeletedCallback(AsciiText *text)
{
#ifdef TH09_NATIVE_NETPLAY
    ui::ReleaseFonts();
#endif
    g_SpriteEngine->ReleaseAnm(1);
    g_SpriteEngine->ReleaseAnm(3);
    return 0;
}

void AsciiText::RemoveChain()
{
    g_Chain.Remove(&g_AsciiCalcElem);
    g_Chain.Remove(&g_AsciiDrawElem);
}

void AsciiText::AddString(D3DXVECTOR3 *pos, char *text)
{
    AsciiString *str;

    if (this->stringCount >= 0x100)
    {
        return;
    }
    str = &this->strings[this->stringCount];
    this->stringCount++;
    strcpy(str->text, text);
    str->pos = *pos;
    str->color = this->color;
    str->scaleY = this->scaleY;
    str->scaleX = this->scaleX;
    str->inArena = this->inArena;
    str->highlight = 0;
}

void AsciiText::AddFormat(D3DXVECTOR3 *pos, char *fmt, ...)
{
    char buf[0x200];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    this->AddString(pos, buf);
    va_end(args);
}

void AsciiText::Initialize()
{
    memset(&this->digit, 0, sizeof(this->digit));
    memset(&this->digit2, 0, sizeof(this->digit2));
    memset(&this->glyph, 0, sizeof(this->glyph));
    memset(this->strings, 0, sizeof(this->strings));
    memset(&this->pauseMenu, 0, sizeof(this->pauseMenu));
    memset(&this->continueMenu, 0, sizeof(this->continueMenu));
    memset(this->popups, 0, sizeof(this->popups[0]));
    this->stringCount = 0;
    this->inArena = 0;
    this->highlight = 0;
    this->nextPopup = 0;
    this->nextPopup2 = 0;
    this->unk_829c = 0;
    this->color = 0xffffffff;
    this->scaleY = 1.0f;
    this->scaleX = 1.0f;
    this->digit.flags |= 0x1800;
    this->digit2.flags |= 0x1800;
    this->anm->InitSprite(&this->digit, 0x61);
    this->anm->InitSprite(&this->glyph, 0x20);
    this->digit.pos.z = 0.1f;
    this->highlight = 0;
    this->SetGlyphWidth(9);
}

int AsciiText::AddedCallback(AsciiText *text)
{
    memset(text, 0, sizeof(AsciiText));
    text->anm = g_SpriteEngine->PreloadAnm(1, "ascii.anm");
    if (text->anm == NULL)
    {
        return -1;
    }
    text->anm2 = g_SpriteEngine->PreloadAnm(3, "capture.anm");
    if (text->anm2 == NULL)
    {
        return -1;
    }
    text->Initialize();
    return 0;
}

void AsciiText::DrawStrings()
{
    float advance;
    int idx;
    AsciiString *string;
    BYTE *c;
    int clipped = 1;

    string = this->strings;
    this->glyph.flags |= 1;
    this->glyph.flags |= 0x1800;
    for (idx = 0; idx < this->stringCount; idx++, string++)
    {
        this->glyph.pos = string->pos;
        c = (BYTE *)string->text;
        this->glyph.scaleX = string->scaleY;
        this->glyph.scaleY = string->scaleX;
        advance = this->glyphWidth * string->scaleY;
        if (clipped != string->inArena)
        {
            clipped = string->inArena;
            g_SpriteEngine->FlushQuadBatch();
            if (clipped != 0)
            {
                g_App.SetView(0);
            }
            else
            {
                g_App.SetView(2);
            }
        }
        while (*c != 0)
        {
            if (*c == '\n')
            {
                this->glyph.pos.y += 14.0f * string->scaleX;
                this->glyph.pos.x = string->pos.x;
            }
            else if (*c != ' ')
            {
                this->glyph.spriteData = this->anm->GetSprite(*c - 0x20);
                this->glyph.color = string->color;
                g_SpriteEngine->DrawUnrotated(&this->glyph);
                this->glyph.pos.x += advance;
            }
            else
            {
                this->glyph.pos.x += advance;
            }
            c++;
        }
    }
    if (clipped != 0)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.SetView(2);
    }
}

void AsciiText::AddPopup(int side, D3DXVECTOR3 *pos, int value, D3DCOLOR color)
{
    AsciiPopup *popup;
    int n;

    if (this->nextPopup >= 100)
    {
        this->nextPopup = 0;
    }
    popup = &this->popups[side][this->nextPopup];
    popup->active = 1;
    n = 0;
    if (value >= 0)
    {
        while (value != 0)
        {
            popup->digits[n] = value % 10;
            n++;
            value /= 10;
        }
    }
    else
    {
        popup->digits[n] = 10;
        n++;
    }
    if (n == 0)
    {
        popup->digits[n] = 0;
        n++;
    }
    popup->digitCount = n;
    popup->color = color;
    popup->timer = 0;
    popup->pos = *pos;
    this->nextPopup++;
}

int PauseMenu::UpdatePauseMenu()
{
    DWORD i;
    int state;

    if (g_PlayerInputs[2].IsJustPressed(8) && this->state != 3)
    {
        g_SoundPlayer.QueueSound(10, 0);
        this->state = 3;
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].IsShown())
            {
                this->sprites[i].pendingInterrupt = 2;
            }
        }
        this->stateFrames = 0;
        this->background.pendingInterrupt = 1;
    }
    if (g_PlayerInputs[2].IsJustPressed(0x200) && this->state != 8)
    {
        g_SoundPlayer.QueueSound(10, 0);
        this->state = 8;
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].IsShown())
            {
                this->sprites[i].pendingInterrupt = 2;
            }
        }
        this->stateFrames = 0;
    }
    switch (state = this->state)
    {
    case 0:
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            g_AsciiText.anm->StartScript(&this->sprites[i], i + 1);
        }
        for (i = 0; i < 3; i++)
        {
            this->sprites[i].pendingInterrupt = 1;
        }
        g_AsciiText.anm->SetSprite(&this->sprites[6], g_GameManager.difficulty + 0x7d);
        this->state++;
        this->stateFrames = 0;
    case 1:
        this->sprites[1].color = 0xffffffff;
        this->sprites[2].color = 0xff505050;
        this->sprites[1].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->sprites[2].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (!g_GameManager.IsReplay())
            {
                if (g_PlayerInputs[2].IsJustPressed(0x10))
                {
                    this->state = 2;
                    g_SoundPlayer.QueueSound(0, 0);
                }
            }
            else if (g_PlayerInputs[2].IsJustPressed(0x10))
            {
                this->state = 2;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_PlayerInputs[2].IsJustPressed(0x20))
            {
                this->state = 2;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_PlayerInputs[2].IsJustPressed(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 3; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                this->state = 3;
                this->stateFrames = 0;
                this->background.pendingInterrupt = 1;
            }
        }
        break;
    case 2:
        this->sprites[1].color = 0xff505050;
        this->sprites[2].color = 0xffffffff;
        this->sprites[1].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->sprites[2].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (g_PlayerInputs[2].IsJustPressed(0x10))
            {
                this->state = 1;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_GameManager.IsReplay())
            {
                if (g_PlayerInputs[2].IsJustPressed(0x20))
                {
                    this->state = 1;
                    g_SoundPlayer.QueueSound(0, 0);
                }
            }
            else if (g_PlayerInputs[2].IsJustPressed(0x20))
            {
                this->state = 1;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_PlayerInputs[2].IsJustPressed(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 3; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                for (; i < 6; i++)
                {
                    this->sprites[i].pendingInterrupt = 1;
                }
                this->state = 5;
                this->stateFrames = 0;
            }
        }
        break;
    case 3:
        if (this->stateFrames >= 20)
        {
            this->state = 0;
            g_GameManager.isPaused = 0;
            for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
            {
                this->sprites[i].ClearShown();
            }
            g_SoundPlayer.UnPause();
            g_App.lastTotalPlayTimeTick = timeGetTime();
        }
        break;
    case 4:
    case 6:
        this->sprites[4].color = 0xffff8080;
        this->sprites[5].color = 0xff505050;
        this->sprites[4].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->sprites[5].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (g_PlayerInputs[2].IsJustPressed(0x10) || g_PlayerInputs[2].IsJustPressed(0x20))
            {
                if (state == 4)
                {
                    this->state = 5;
                }
                else
                {
                    this->state = 7;
                }
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_PlayerInputs[2].IsJustPressed(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 3; i < 6; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                if (this->state == 4)
                {
                    this->state = 8;
                }
                else
                {
                    this->state = 9;
                }
                this->stateFrames = 0;
            }
        }
        break;
    case 5:
    case 7:
        this->sprites[4].color = 0xff505050;
        this->sprites[5].color = 0xffff8080;
        this->sprites[4].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->sprites[5].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (g_PlayerInputs[2].IsJustPressed(0x10) || g_PlayerInputs[2].IsJustPressed(0x20))
            {
                if (state == 5)
                {
                    this->state = 4;
                }
                else
                {
                    this->state = 6;
                }
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_PlayerInputs[2].IsJustPressed(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 3; i++)
                {
                    this->sprites[i].pendingInterrupt = 1;
                }
                for (; i < 6; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                if (this->state == 5)
                {
                    this->state = 2;
                }
                this->stateFrames = 0;
            }
        }
        break;
    case 8:
        if (this->stateFrames >= 20)
        {
            this->state = 0;
            g_App.scene = 1;
            g_GameManager.isPaused = 0;
            g_App.lastTotalPlayTimeTick = timeGetTime();
        }
        break;
    case 9:
        if (this->stateFrames >= 20)
        {
            if (!g_GameManager.IsInPractice() && g_GameManager.difficulty != 4)
            {
                this->state = 0;
                g_App.scene = 10;
                g_GameManager.isPaused = 0;
                g_App.lastTotalPlayTimeTick = timeGetTime();
            }
            else
            {
                g_App.StopMusic();
                g_App.scene = 11;
                g_GameManager.isPaused = 0;
                g_App.lastTotalPlayTimeTick = timeGetTime();
                return 0;
            }
        }
    }
    for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
    {
        g_SpriteEngine->Step(&this->sprites[i]);
    }
    if ((g_App.stateFlags >> 1) & 1)
    {
        g_SpriteEngine->Step(&this->background);
    }
    this->stateFrames++;
    return 0;
}

void PauseMenu::DrawPauseMenu()
{
    int i;

    if (g_GameManager.isPaused)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.SetView(2);
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].IsShown())
            {
                g_SpriteEngine->DrawUnrotated(&this->sprites[i]);
            }
        }
    }
}

int ContinueMenu::UpdateContinueMenu()
{
    DWORD i;

    if (g_GameManager.IsReplay())
    {
        g_GameManager.isInContinueMenu = 0;
        g_App.scene = 1;
        return 1;
    }
    if (g_GameManager.sides[0].stats->unk_14 >= 3)
    {
        g_GameManager.isInContinueMenu = 0;
        g_App.scene = 6;
        return 1;
    }
    switch (this->state)
    {
    case 0:
        if (!g_GameManager.IsMode1())
        {
            for (i = 0; i < 5; i++)
            {
                g_AsciiText.anm->StartScript(&this->sprites[i], i + 8);
            }
            for (i = 0; i < 5; i++)
            {
                this->sprites[i].pendingInterrupt = 1;
            }
            g_AsciiText.anm->SetSprite(&this->sprites[4], 0x78 - g_GameManager.sides[0].stats->unk_14);
        }
        else
        {
            for (i = 0; i < 3; i++)
            {
                g_AsciiText.anm->StartScript(&this->sprites[i], i + 0x11);
            }
            for (i = 0; i < 3; i++)
            {
                this->sprites[i].pendingInterrupt = 1;
            }
        }
        this->state++;
        this->stateFrames = 0;
    case 1:
        this->sprites[1].color = 0xff505050;
        this->sprites[2].color = 0xffff8080;
        this->sprites[1].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->sprites[2].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (g_PlayerInputs[2].IsJustPressed(0x10) || g_PlayerInputs[2].IsJustPressed(0x20))
            {
                this->state = 2;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_PlayerInputs[2].IsJustPressed(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 5; i++)
                {
                    this->sprites[i].pendingInterrupt = 1;
                }
                this->state = 4;
                this->stateFrames = 0;
            }
        }
        break;
    case 2:
        this->sprites[2].color = 0xff505050;
        this->sprites[1].color = 0xffff8080;
        this->sprites[2].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->sprites[1].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (g_PlayerInputs[2].IsJustPressed(0x10) || g_PlayerInputs[2].IsJustPressed(0x20))
            {
                this->state = 1;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_PlayerInputs[2].IsJustPressed(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 5; i++)
                {
                    this->sprites[i].pendingInterrupt = 1;
                }
                this->state = 3;
                this->stateFrames = 0;
            }
        }
        break;
    case 4:
        if (this->stateFrames >= 20)
        {
            this->state = 0;
            this->stateFrames = 0;
            g_GameManager.isInContinueMenu = 0;
            g_App.scene = 6;
            for (i = 0; i < 5; i++)
            {
                this->sprites[i].ClearShown();
            }
            g_App.lastTotalPlayTimeTick = timeGetTime();
            return 1;
        }
        break;
    case 3:
        if (this->stateFrames >= 20)
        {
            if (!g_GameManager.IsMode1())
            {
                this->state = 0;
                this->stateFrames = 0;
                g_GameManager.isInContinueMenu = 0;
                for (i = 0; i < 5; i++)
                {
                    this->sprites[i].ClearShown();
                }
                g_GameManager.sides[0].stats->unk_14++;
                g_GameManager.stateFlags |= 0x2000;
                g_GameManager.sides[0].stats->guiScore = 0;
                g_GameManager.sides[0].stats->score = 0;
                g_GameManager.sides[0].stats->scoreIncrement = 0;
                g_GameManager.sides[1].stats->guiScore = 0;
                g_GameManager.sides[1].stats->score = 0;
                g_GameManager.sides[1].stats->scoreIncrement = 0;
                g_App.redrawFrames = 8;
                g_App.lastTotalPlayTimeTick = timeGetTime();
                g_App.StopMusic();
                g_App.PlayBgm();
                GameManager::ResetRound();
                g_GameManager.unk_108[0] = 0;
                g_GameManager.unk_108[1] = 0;
                g_GameManager.sides[0].stats->lives = g_GameManager.config->unk_ac + 2.0f;
                g_GameManager.stateFlags |= 0x4000;
                return 1;
            }
            this->state = 0;
            g_App.scene = 13;
            this->stateFrames = 0;
            g_GameManager.unk_fc = 0;
            g_GameManager.isInContinueMenu = 0;
            g_App.lastTotalPlayTimeTick = timeGetTime();
            return 1;
        }
        break;
    }
    for (i = 0; i < 5; i++)
    {
        g_SpriteEngine->Step(&this->sprites[i]);
    }
    if ((g_App.stateFlags >> 1) & 1)
    {
        g_SpriteEngine->Step(&this->background);
    }
    this->stateFrames++;
    return 0;
}

void ContinueMenu::DrawContinueMenu()
{
    int i;

    if (g_GameManager.isInContinueMenu)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.SetView(2);
        if ((g_App.stateFlags >> 1) & 1 && (this->state != 0 || this->stateFrames > 2))
        {
            g_SpriteEngine->DrawUnrotated(&this->background);
        }
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].IsShown())
            {
                g_SpriteEngine->DrawUnrotated(&this->sprites[i]);
            }
        }
    }
}

void AsciiText::DrawPopups(int side)
{
    AsciiPopup *popup;
    int i;
    int j;
    BYTE *digit;
    float dx;
    float dy;
    int distSq;
    int alpha;

    popup = this->popups[side];
    if (!g_App.GetNoFog())
    {
        g_App.DisableFog();
    }
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    for (i = 0; i < 100; i++, popup++)
    {
        if (!popup->active)
        {
            continue;
        }
        this->digit.pos.x = g_GameManager.ArenaToScreenX(popup->pos.x - popup->digitCount * 4);
        this->digit.pos.y = g_GameManager.ArenaToScreenY(popup->pos.y);
        this->digit.color = popup->color;
        dx = g_GameManager.sides[side].player->pos.x - popup->pos.x;
        dy = g_GameManager.sides[side].player->pos.y - popup->pos.y;
        distSq = dx * dx + dy * dy;
        if (distSq > 0x1000)
        {
            alpha = 0xd0;
        }
        else if (distSq > 0x400)
        {
            alpha = (distSq - 0x400) * 0x80 / 0xc00 + 0x50;
        }
        else
        {
            alpha = 0x50;
        }
        digit = &popup->digits[popup->digitCount - 1];
        for (j = popup->digitCount; j > 0; j--, digit--)
        {
            if (popup->timer < 0x34)
            {
                this->digit.spriteData = this->anm->GetSprite(*digit + 0x61);
                this->digit.alpha = alpha;
            }
            else if (popup->timer < 0x38)
            {
                this->digit.spriteData = this->anm->GetSprite(*digit + 0x61);
                this->digit.alpha = alpha;
            }
            else
            {
                this->digit.spriteData = this->anm->GetSprite(*digit + 0x61);
                this->digit.alpha = alpha;
            }
            this->digit.spriteWidth = this->digit.spriteData->width;
            g_SpriteEngine->DrawUnrotated(&this->digit);
            this->digit.pos.x += 8.0f;
        }
    }
}

int UnknownMenu::UpdateUnknownMenu()
{
#ifdef TH09_NATIVE_NETPLAY
    if(thnet::PostMatchUpdate())return 0;
#endif
    DWORD i;

    if (g_GameManager.IsReplay())
    {
        g_GameManager.unk_13e = 0;
        g_App.scene = 1;
        return 1;
    }
    switch (this->state)
    {
    case 0:
        for (i = 0; i < 3; i++)
        {
            g_AsciiText.anm->StartScript(&this->sprites[i], i + 0xd);
            this->sprites[i].pendingInterrupt = 1;
        }
        this->state++;
        this->stateFrames = 0;
    case 1:
        this->sprites[0].color = 0xffffffff;
        this->sprites[1].color = this->sprites[2].color = 0xff505050;
        this->sprites[0].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->sprites[1].posOffset = this->sprites[2].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (g_PlayerInputs[2].IsJustPressed(0x10))
            {
                this->state = 3;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_PlayerInputs[2].IsJustPressed(0x20))
            {
                this->state = 2;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_PlayerInputs[2].IsJustPressed(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 3; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                this->state = 4;
                this->stateFrames = 0;
            }
        }
        break;
    case 2:
        this->sprites[0].color = this->sprites[2].color = 0xff505050;
        this->sprites[1].color = 0xffffffff;
        this->sprites[0].posOffset = this->sprites[2].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->sprites[1].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (g_PlayerInputs[2].IsJustPressed(0x10))
            {
                this->state = 1;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_PlayerInputs[2].IsJustPressed(0x20))
            {
                this->state = 3;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_PlayerInputs[2].IsJustPressed(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 3; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                this->state = 5;
                this->stateFrames = 0;
            }
        }
        break;
    case 3:
        this->sprites[0].color = this->sprites[1].color = 0xff505050;
        this->sprites[2].color = 0xffffffff;
        this->sprites[0].posOffset = this->sprites[1].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->sprites[2].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (this->stateFrames >= 4)
        {
            if (g_PlayerInputs[2].IsJustPressed(0x10))
            {
                this->state = 2;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_PlayerInputs[2].IsJustPressed(0x20))
            {
                this->state = 1;
                g_SoundPlayer.QueueSound(0, 0);
            }
            if (g_PlayerInputs[2].IsJustPressed(0x1001))
            {
                g_SoundPlayer.QueueSound(10, 0);
                for (i = 0; i < 3; i++)
                {
                    this->sprites[i].pendingInterrupt = 2;
                }
                this->state = 6;
                this->stateFrames = 0;
            }
        }
        break;
    case 5:
        if (this->stateFrames >= 20)
        {
            this->state = 0;
            g_App.scene = 1;
            g_GameManager.unk_13e = 0;
            g_App.lastTotalPlayTimeTick = timeGetTime();
            return 1;
        }
        break;
    case 4:
        if (this->stateFrames >= 20)
        {
            this->state = 0;
            g_App.scene = 10;
            g_GameManager.unk_13e = 0;
            g_App.lastTotalPlayTimeTick = timeGetTime();
            return 1;
        }
        break;
    case 6:
        if (this->stateFrames >= 20)
        {
            this->state = 0;
            g_App.scene = 6;
            g_GameManager.unk_13e = 0;
            g_App.lastTotalPlayTimeTick = timeGetTime();
            return 1;
        }
        break;
    }
    for (i = 0; i < 3; i++)
    {
        g_SpriteEngine->Step(&this->sprites[i]);
    }
    this->stateFrames++;
    return 0;
}

void UnknownMenu::DrawUnknownMenu()
{
#ifdef TH09_NATIVE_NETPLAY
    if(thnet::Active() && !coop::Active())return;
#endif
    int i;

    if (g_GameManager.unk_13e)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.SetView(2);
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].IsShown())
            {
                g_SpriteEngine->DrawUnrotated(&this->sprites[i]);
            }
        }
    }
}

int AsciiText::OnUpdate(AsciiText *text)
{
    int i;
    int side;
    AsciiPopup *entry;

    if (!g_GameManager.isPaused && !g_GameManager.isInContinueMenu)
    {
        if (!(g_GameManager.stateFlags & 0x1800))
        {
            for (side = 0; side < 2; side++)
            {
                entry = text->popups[side];
                if (g_GameManager.sides[side].flags & 1)
                {
                    continue;
                }
                for (i = 0; i < 100; i++, entry++)
                {
                    if (!entry->active)
                    {
                        continue;
                    }
                    entry->pos.y -= 0.2f * g_App.gameSpeed;
                    entry->timer++;
                    if (entry->timer > 60)
                    {
                        entry->active = 0;
                    }
                }
            }
        }
    }
    else if (g_GameManager.isPaused)
    {
        text->pauseMenu.UpdatePauseMenu();
    }
    if (g_GameManager.isInContinueMenu)
    {
        if (text->continueMenu.UpdateContinueMenu())
        {
            return 3;
        }
    }
    if (g_GameManager.unk_13e)
    {
        if (text->unknownMenu.UpdateUnknownMenu())
        {
            return 3;
        }
    }
    text->frameCounter++;
    return 1;
}

int AsciiText::OnDraw(AsciiText *text)
{
    text->DrawStrings();
    text->ClearStrings();
    text->pauseMenu.DrawPauseMenu();
    text->continueMenu.DrawContinueMenu();
    text->unknownMenu.DrawUnknownMenu();
    if (text->demoIcon.scriptIndex)
    {
        g_SpriteEngine->DrawUnrotated(&text->demoIcon);
    }
#ifdef TH09_NATIVE_NETPLAY
    ui::Draw();
#endif
    return 1;
}

int AsciiText::OnDraw2(AsciiText *text)
{
    g_App.SetView(0);
    text->DrawPopups(0);
    g_App.SetView(1);
    text->DrawPopups(1);
    return 1;
}

int AsciiText::RegisterChain()
{
    AsciiText *text = &g_AsciiText;

    g_AsciiCalcElem.SetCallback((ChainCallback)AsciiText::OnUpdate);
    g_AsciiCalcElem.addedCallback = (ChainCallback)AsciiText::AddedCallback;
    g_AsciiCalcElem.deletedCallback = (ChainCallback)AsciiText::DeletedCallback;
    g_AsciiCalcElem.arg = text;
    if (g_Chain.AddToCalcChain(&g_AsciiCalcElem, 1) != 0)
    {
        return -1;
    }
    g_AsciiDrawElem.SetCallback((ChainCallback)AsciiText::OnDraw);
    g_AsciiDrawElem.arg = text;
    g_Chain.AddToDrawChain(&g_AsciiDrawElem, 0x22);
    g_AsciiDrawElem2.SetCallback((ChainCallback)AsciiText::OnDraw2);
    g_AsciiDrawElem2.arg = text;
    g_Chain.AddToDrawChain(&g_AsciiDrawElem2, 0x17);
    return 0;
}

AsciiText::AsciiText()
{
}


void AsciiText::StartScripts()
{
}
