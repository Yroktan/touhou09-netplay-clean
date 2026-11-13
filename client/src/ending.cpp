

#include <stdlib.h>
#include <string.h>

#include "ending.h"
#include "gamemanager.h"
#include "scoredat.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

int Ending::ReadInt()
{
    int value;

    value = atoi(this->cursor);
    while (*this->cursor != '\0')
    {
        this->cursor++;
    }
    while (*this->cursor == '\0')
    {
        this->cursor++;
    }
    return value;
}

void Ending::DrawFade()
{
    float rect[4];

    rect[0] = 0.0f;
    rect[1] = 0.0f;
    rect[2] = 640.0f;
    rect[3] = 480.0f;
    if (this->fadeColor & 0xff000000)
    {
        DrawColorRect(rect, this->fadeColor);
    }
}

void Ending::UpdateFade()
{
    int a;

    switch (this->fadeType)
    {
    case 1:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeType = 0;
            this->fadeColor = 0;
            break;
        }
        a = 0xff - this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24;
        this->fadeTimer++;
        break;
    case 2:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeColor = 0xff000000;
            break;
        }
        a = this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24;
        this->fadeTimer++;
        break;
    case 3:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeType = 0;
            this->fadeColor = 0;
            break;
        }
        a = 0xff - this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24 | 0xffffff;
        this->fadeTimer++;
        break;
    case 4:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeColor = 0xffffffff;
            break;
        }
        a = this->fadeTimer * 0xff / this->fadeDuration;
        this->fadeColor = a << 24 | 0xffffff;
        this->fadeTimer++;
        break;
    case 0:
        this->fadeColor = 0;
        break;
    }
}

int Ending::DeletedCallback(Ending *ending)
{
    g_SpriteEngine->ReleaseAnm(0x13);
    g_App.scene = 6;
    g_SpriteEngine->ReleaseSurface(0);
    g_MemoryTracker.Free(ending->file);
    g_Chain.Remove(ending->drawElem);
    ending->drawElem = NULL;
    delete ending;
    g_App.stateFlags &= ~0x400;
    g_App.hideFps = 0;
    return 0;
}

int Ending::OnDraw(Ending *ending)
{
    int i;

    g_SpriteEngine->CopySurfaceRectToBackBuffer(0, 0, 0, ending->scrollX, ending->scrollY, 640, 480);
    for (i = 0; i < 0xf; i++)
    {
        g_SpriteEngine->Draw(&ending->sprites[i]);
    }
    ending->DrawFade();
    return 1;
}

int Ending::Load(char *path)
{
    char *oldFile;

    oldFile = this->file;
    this->file = (char *)LoadFileData(path, NULL, 0);
    if (this->file == NULL)
    {
        this->file = oldFile;
        LogPrintf(&g_Logger, "error : エンディングファイルが読み込めない、ファイルが破壊されています\r\n");
        return -1;
    }
    this->cursor = this->file;
    this->lineDelay = 8;
    this->lineTimer = 0;
    this->timer = 0;
    if (oldFile != NULL)
    {
        g_MemoryTracker.Free(oldFile);
    }
    return 0;
}

int Ending::AddedCallback(Ending *ending)
{
    int j;
    int i;
    int count;

    g_App.stateFlags |= 0x400;
    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->ClearSpriteData();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearVertexFormat();
    ClearScreen(0xffffffff);
    g_App.hideFps = 1;
    for (j = 0; j < 0xf; j++)
    {
        g_App.textAnm->StartScriptAtOrigin(&ending->sprites[j], j + 0xe);
        ending->sprites[j].pos = D3DXVECTOR3(64.0f, (float)j * 16.0f + 400.0f, 0.0f);
    }
    if (g_GameManager.IsMode0())
    {
        g_PlayRecord.AddClear(g_GameManager.sides[0].character, g_GameManager.difficulty);
    }
    else if (g_GameManager.IsMode1())
    {
        g_PlayRecord.AddClear(g_GameManager.sides[0].character, g_GameManager.difficulty);
    }
    count = 0;
    for (i = 0; i <= 4; i++)
    {
        if (g_PlayRecord.HasCleared(i))
        {
            count++;
        }
    }
    if (count >= 2)
    {
        if (g_PlayRecord.GetClears5(5))
        {
            g_PlayRecord.OpenEntry2(5);
        }
        if (g_PlayRecord.GetClears5(7))
        {
            g_PlayRecord.OpenEntry2(7);
        }
    }
    count = 0;
    for (i = 0; i <= 8; i++)
    {
        if (g_PlayRecord.HasCleared(i))
        {
            count++;
        }
    }
    if (count >= 4)
    {
        if (g_PlayRecord.GetClears5(6))
        {
            g_PlayRecord.OpenEntry2(6);
        }
        if (g_PlayRecord.GetClears5(8))
        {
            g_PlayRecord.OpenEntry2(8);
        }
    }
    count = 0;
    for (i = 0; i <= 8; i++)
    {
        if (g_PlayRecord.HasCleared(i))
        {
            count++;
        }
    }
    if (count >= 9)
    {
        if (g_PlayRecord.GetClears5(10))
        {
            g_PlayRecord.OpenEntry2(10);
        }
        if (g_PlayRecord.GetClears5(9))
        {
            g_PlayRecord.OpenEntry2(9);
        }
        if (g_PlayRecord.GetClears5(11))
        {
            g_PlayRecord.OpenEntry2(11);
        }
    }
    count = 0;
    for (i = 0; i <= 11; i++)
    {
        if (g_PlayRecord.HasCleared(i))
        {
            count++;
        }
    }
    if (count > 11)
    {
        g_PlayRecord.OpenEntry3(12);
    }
    if (g_PlayRecord.HasCleared(12))
    {
        g_PlayRecord.OpenEntry3(13);
    }
    if (g_PlayRecord.HasCleared(13))
    {
        for (i = 0; i <= 13; i++)
        {
            g_PlayRecord.OpenEntry2(i);
            g_PlayRecord.OpenEntry3(i);
        }
        g_PlayRecord.OpenEntry(14);
        g_PlayRecord.OpenEntry(15);
    }
    ending->canSkip = 1;
    if (ending->Load(g_EndingFiles[g_GameManager.sides[0].character]) != 0)
    {
        return -1;
    }
    if (g_Unknown4a7c80 != NULL)
    {
        g_Unknown4a7c80->Finish();
    }
    g_Unknown4a7c80 = NULL;
    return 0;
}

#define SKIP_LINE()                                                                                                    \
    while (*this->cursor != '\n' && *this->cursor != '\r')                                                             \
    {                                                                                                                  \
        this->cursor++;                                                                                                \
    }                                                                                                                  \
    while (*this->cursor == '\n' || *this->cursor == '\r')                                                             \
    {                                                                                                                  \
        this->cursor++;                                                                                                \
    }

int Ending::RunScript()
{
    float secs4;
    int m;
    int bottom2;
    int num4;
    int spr2;
    int a2;
    int sprite2;
    int k2;
    int lineLen3 = 0;
    char text3[0x44];

    memset(text3, 0, sizeof(text3));
    this->UpdateFade();
    if (this->waitTimer > 0)
    {
        this->waitTimer--;
        if (this->waitMinFrames != 0)
        {
            this->waitMinFrames--;
        }
        else if (g_PlayerInputs[2].IsJustPressed(0x1001) ||
                 (this->canSkip != 0 && g_PlayerInputs[2].IsHeld(0x100)))
        {
            this->waitTimer = 0;
        }
        if (this->waitTimer <= 0)
        {
            for (k2 = 0; k2 < 0xf; k2++)
            {
                this->sprites[k2].SetPendingInterrupt(2);
            }
            this->line = 0;
        }
        else
        {
            goto done;
        }
    }
    if (this->lineTimer > 0)
    {
        this->lineTimer--;
        if (this->lineMinFrames != 0)
        {
            this->lineMinFrames--;
        }
        else if (g_PlayerInputs[2].IsJustPressed(0x1001) ||
                 (this->canSkip != 0 && g_PlayerInputs[2].IsHeld(0x100)))
        {
            this->lineTimer = 0;
        }
        goto done;
    }
    while (TRUE)
    {
        switch (*this->cursor)
        {
        case '@':
            this->cursor++;
            switch (*this->cursor)
            {
            case 'b':
                if (g_SpriteEngine->LoadSurface(0, this->cursor + 1) != 0)
                {
                    return -1;
                }
                break;
            case 'a':
                this->cursor++;
                a2 = this->ReadInt();
                sprite2 = this->ReadInt();
                spr2 = this->ReadInt();
                this->anm->StartScriptAtOrigin(&this->sprites[a2], sprite2);
                this->anm->SetSprite(&this->sprites[a2], spr2);
                break;
            case 'V':
                this->cursor++;
                num4 = this->ReadInt();
                bottom2 = this->ReadInt();
                this->scrollSpeed = (float)num4 / (float)bottom2;
                break;
            case 'v':
                this->cursor++;
                this->scrollY = this->ReadInt();
                break;
            case 'F':
                g_SpriteEngine->ReleaseAnm(0x13);
                this->anm = g_SpriteEngine->LoadAnm(0x13, "staff01.anm");
                if (this->Load(this->cursor + 1) != 0)
                {
                    return -1;
                }
                lineLen3 = 0;
            case 'R':
                for (m = 0; m < 0x10; m++)
                {
                    this->sprites[m].scriptIndex = 0;
                }
                break;
            case 'm':
                g_App.StopMusic();
                g_App.LoadBgm(this->cursor[1]);
                g_App.PlayMusicById(this->cursor[1], 0);
                break;
            case 'M':
                this->cursor++;
                secs4 = this->ReadInt();
                g_App.FadeOutMusic(secs4);
                break;
            case 's':
                this->cursor++;
                this->lineDelay = this->ReadInt();
                this->lineDelayFast = this->ReadInt();
                break;
            case 'c':
                this->cursor++;
                this->textColor = this->ReadInt();
                break;
            case 'r':
                this->cursor++;
                this->waitTimer = this->ReadInt();
                this->waitMinFrames = this->ReadInt();
                this->lineTimer = 0;
                this->lineMinFrames = 0;
                SKIP_LINE();
                goto done;
            case 'w':
                this->cursor++;
                this->lineTimer = this->ReadInt();
                this->lineMinFrames = this->ReadInt();
                SKIP_LINE();
                goto done;
            case '0':
                this->cursor++;
                this->fadeType = 1;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadInt() / 2;
                this->UpdateFade();
                break;
            case '1':
                this->cursor++;
                this->fadeType = 2;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadInt() / 2;
                this->UpdateFade();
                break;
            case '2':
                this->cursor++;
                this->fadeType = 3;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadInt() / 2;
                this->UpdateFade();
                break;
            case '3':
                this->cursor++;
                this->fadeType = 4;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadInt() / 2;
                this->UpdateFade();
                break;
            case 'z':
                return -1;
            }
            SKIP_LINE();
            break;
        case '\0':
        case '\n':
        case '\r':
            if (lineLen3 != 0)
            {
                g_SpriteEngine->RenderText(&this->sprites[this->line], this->textColor, 0, text3);
                this->sprites[this->line].SetPendingInterrupt(1);
            }
            while (*this->cursor == '\n' || *this->cursor == '\0' || *this->cursor == '\r')
            {
                this->cursor++;
            }
            if (g_PlayerInputs[2].IsHeld(0x1001))
            {
                this->lineTimer = this->lineDelayFast;
                this->lineMinFrames = this->lineDelayFast;
            }
            else
            {
                this->lineTimer = this->lineDelay;
                this->lineMinFrames = this->lineDelayFast;
            }
            this->line++;
            goto done;
        default:
            text3[lineLen3] = this->cursor[0];
            text3[lineLen3 + 1] = this->cursor[1];
            lineLen3 += 2;
            this->cursor += 2;
            break;
        }
    }
done:
    this->timer++;
    this->scrollY -= this->scrollSpeed;
    if (this->scrollY <= 0.0f)
    {
        this->scrollY = 0.0f;
        this->scrollSpeed = 0.0f;
    }
    return 0;
}

Ending::Ending()
{
    memset(this, 0, sizeof(Ending));
    this->lineDelay = 8;
    this->lineTimer = 0;
    this->timer = 0;
    this->scrollX = 0.0f;
    this->scrollY = 0.0f;
    this->scrollSpeed = 0.0f;
}

int Ending::OnUpdate(Ending *ending)
{
    int i;
    int extra = 0;

loop:
    if (ending->RunScript() != 0)
    {
        return 0;
    }
    for (i = 0; i < 0xf; i++)
    {
        g_SpriteEngine->Step(&ending->sprites[i]);
    }
    if (ending->canSkip != 0 && g_PlayerInputs[2].IsHeld(0x100) && extra < 8)
    {
        extra++;
        goto loop;
    }
    return 1;
}

int Ending::RegisterChain()
{
    Ending *ending = (Ending *)g_MemoryTracker.Register(new Ending(), sizeof(Ending), "EndingInf");

    ending->calcElem = g_Chain.CreateElem((ChainCallback)Ending::OnUpdate);
    ending->calcElem->arg = ending;
    ending->calcElem->addedCallback = (ChainCallback)Ending::AddedCallback;
    ending->calcElem->deletedCallback = (ChainCallback)Ending::DeletedCallback;
    if (g_Chain.AddToCalcChain(ending->calcElem, 5) != 0)
    {
        return -1;
    }
    ending->drawElem = g_Chain.CreateElem((ChainCallback)Ending::OnDraw);
    ending->drawElem->arg = ending;
    g_Chain.AddToDrawChain(ending->drawElem, 4);
    return 0;
}
