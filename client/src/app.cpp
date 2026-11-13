#ifdef TH09_NATIVE_NETPLAY
#include "UiText.h"
#endif
#include "archive.h"
#include "ascii.h"
#include "bullet.h"
#include "chain.h"
#include "ending.h"
#include "gamemanager.h"
#include "gdisurface.h"
#include "gui.h"
#include "mainmenu.h"
#include "midi.h"
#include "replay.h"
#include "scoredat.h"
#include "screeneffect.h"
#include "sound.h"
#include "sprite.h"
#include "timer.h"
#include "unsorted.h"
#ifdef TH09_NATIVE_NETPLAY
#include "NetGame.h"
#endif

int Network::GetOtherUnknown1c()
{
    return this->unk_1c[1 - this->unk_ac];
}

void Network::SetUnknownA8()
{
    this->unk_a8 = 1;
}

void Network::ClearUnknownA8()
{
    this->unk_a8 = 0;
}

void ReadInputs()
{
    ReadButtons(0);
    ReadButtons(1);
    ReadButtons(2);
}

void App::AddRecord(int side, int value, WORD unknown, WORD seed)
{
    int i;
    int j;

    this->lastRecords[side].value = value;
    this->lastRecords[side].unk_0 = unknown;
    this->lastRecords[side].seed = seed;
    for (i = 0; i < 10; i++)
    {
        if (this->records[side][i].value == 0 || this->records[side][i].value > value)
        {
            break;
        }
    }
    if (this->records[side][i].value != 0 && i < 9)
    {
        for (j = 9; j > i; j--)
        {
            this->records[side][j].unk_0 = this->records[side][j - 1].unk_0;
            this->records[side][j].seed = this->records[side][j - 1].seed;
            this->records[side][j].value = this->records[side][j - 1].value;
            this->records[side][j].unk_8 = this->records[side][j - 1].unk_8;
        }
    }
    this->records[side][i].unk_0 = unknown;
    this->records[side][i].value = value;
    this->records[side][i].seed = seed;
    this->records[side][i].unk_8 = 0;
}

void App::InsertRecord(int side, int value, WORD seed)
{
    int i;
    int j;

    for (i = 0; i < 10; i++)
    {
        if (this->records[side][i].value == 0 || this->records[side][i].value > value)
        {
            break;
        }
    }
    if (this->records[side][i].value != 0 && i < 9)
    {
        for (j = 9; j > i; j--)
        {
            this->records[side][j].unk_0 = this->records[side][j - 1].unk_0;
            this->records[side][j].seed = this->records[side][j - 1].seed;
            this->records[side][j].value = this->records[side][j - 1].value;
            this->records[side][j].unk_8 = this->records[side][j - 1].unk_8;
        }
    }
    this->records[side][i].unk_0 = this->lastRecords[side].unk_0;
    this->records[side][i].value = value;
    this->records[side][i].seed = seed;
    this->records[side][i].unk_8 = 1;
}

WORD App::PopRecord(int side, WORD *seed, int *unk_8)
{
    WORD unknown;
    int i;

    if (this->records[side][0].unk_8 == 0)
    {
        unknown = this->records[side][0].unk_0;
    }
    else
    {
        unknown = g_PlayerInputs[side].unk_0;
    }
    *seed = this->records[side][0].seed;
    *unk_8 = this->records[side][0].unk_8;
    for (i = 0; i < 9; i++)
    {
        this->records[side][i].unk_0 = this->records[side][i + 1].unk_0;
        this->records[side][i].seed = this->records[side][i + 1].seed;
        this->records[side][i].value = this->records[side][i + 1].value;
        this->records[side][i].unk_8 = this->records[side][i + 1].unk_8;
    }
    return unknown;
}

void App::ResetRun()
{
    this->unk_458 = 0;
    this->unk_45c = 0;
    this->unk_460 = 0;
    DWORD time = timeGetTime();
    this->unk_464 = time;
    this->unk_468 = time;
    this->unk_474 = time;
    this->unk_478 = 0;
    memset(this->records, 0, sizeof(this->records));
    memset(this->lastRecords, 0, sizeof(this->lastRecords));
    g_App.AddRecord(0, -9999, 0, g_Rng.GetSeed());
    g_App.AddRecord(1, -9999, 0, g_Rng.GetSeed());
    g_Network->SetUnknownA8();
    g_Network->unk_b8 = 0;
}

BOOL App::RecordsMeet(int side)
{
    if (this->records[side][0].value != 0 && this->records[1][0].value == this->records[0][0].value)
    {
        return TRUE;
    }
    return FALSE;
}

int App::GetPreloadBgm()
{
    return this->config.flags.preloadBgm;
}

int App::GetNoVsync()
{
    return this->config.flags.noVsync;
}

int App::GetNoTextRenderDetect()
{
    return this->config.flags.noTextRenderDetect;
}

int App::OnDrawLoading(App *app)
{
    DWORD i;

    for (i = 0; i < 3; i++)
    {
        g_LoadingScreenSprites[i].pos += g_LoadingScreenSprites[i].posOffset;
        g_SpriteEngine->Draw(&g_LoadingScreenSprites[i]);
        g_LoadingScreenSprites[i].pos -= g_LoadingScreenSprites[i].posOffset;
    }
    return 1;
}

BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext)
{
    HRESULT hr;

    if (g_App.joystick == NULL)
    {
        g_App.dinput->CreateDevice(pdidInstance->guidInstance, &g_App.joystick, NULL);
        return DIENUM_CONTINUE;
    }
    if (g_App.joystick2 == NULL)
    {
        hr = g_App.dinput->CreateDevice(pdidInstance->guidInstance, &g_App.joystick2, NULL);
        return FAILED(hr);
    }
    return DIENUM_CONTINUE;
}

BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext)
{
    DIPROPRANGE diprg;

    if (pdidoi->dwType & DIDFT_AXIS)
    {
        diprg.diph.dwSize = sizeof(DIPROPRANGE);
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType;
        diprg.lMin = -1000;
        diprg.lMax = +1000;

        if (FAILED(g_App.joystick->SetProperty(DIPROP_RANGE, &diprg.diph)))
        {
            return DIENUM_STOP;
        }
    }
    return DIENUM_CONTINUE;
}

BOOL CALLBACK EnumAxesCallback2(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext)
{
    DIPROPRANGE diprg;

    if (pdidoi->dwType & DIDFT_AXIS)
    {
        diprg.diph.dwSize = sizeof(DIPROPRANGE);
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType;
        diprg.lMin = -1000;
        diprg.lMax = +1000;

        if (FAILED(g_App.joystick2->SetProperty(DIPROP_RANGE, &diprg.diph)))
        {
            return DIENUM_STOP;
        }
    }
    return DIENUM_CONTINUE;
}




static int InitDirectInput(App *app)
{
    HINSTANCE hInst = (HINSTANCE)GetWindowLongA(app->hwnd, GWL_HINSTANCE);

    if (app->GetNoDirectInput())
    {
        return -1;
    }
    if (FAILED(DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID **)&app->dinput, NULL)))
    {
        app->dinput = NULL;
        LogPrintf(&g_Logger, "DirectInput が使用できません\r\n");
        return -1;
    }
    if (FAILED(app->dinput->CreateDevice(GUID_SysKeyboard, &app->keyboard, NULL)))
    {
        if (app->dinput)
        {
            app->dinput->Release();
            app->dinput = NULL;
        }
        LogPrintf(&g_Logger, "DirectInput が使用できません\r\n");
        return -1;
    }
    if (FAILED(app->keyboard->SetDataFormat(&c_dfDIKeyboard)))
    {
        if (app->keyboard)
        {
            app->keyboard->Release();
            app->keyboard = NULL;
        }
        if (app->dinput)
        {
            app->dinput->Release();
            app->dinput = NULL;
        }
        LogPrintf(&g_Logger, "DirectInput SetDataFormat が使用できません\r\n");
        return -1;
    }
    if (FAILED(app->keyboard->SetCooperativeLevel(app->hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND | DISCL_NOWINKEY)))
    {
        if (app->keyboard)
        {
            app->keyboard->Release();
            app->keyboard = NULL;
        }
        if (app->dinput)
        {
            app->dinput->Release();
            app->dinput = NULL;
        }
        LogPrintf(&g_Logger, "DirectInput SetCooperativeLevel が使用できません\r\n");
        return -1;
    }
    app->keyboard->Acquire();
    LogPrintf(&g_Logger, "DirectInput は正常に初期化されました\r\n");
    app->dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, NULL, DIEDFL_ATTACHEDONLY);
    if (app->joystick)
    {
        app->joystick->SetDataFormat(&c_dfDIJoystick2);
        app->joystick->SetCooperativeLevel(app->hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
        g_App.joystickCaps.dwSize = sizeof(DIDEVCAPS);
        app->joystick->GetCapabilities(&g_App.joystickCaps);
        app->joystick->EnumObjects(EnumAxesCallback, NULL, DIDFT_ALL);
        LogPrintf(&g_Logger, "有効なパッドを発見しました\r\n");
    }
    if (app->joystick2)
    {
        app->joystick2->SetDataFormat(&c_dfDIJoystick2);
        app->joystick2->SetCooperativeLevel(app->hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
        g_App.joystickCaps2.dwSize = sizeof(DIDEVCAPS);
        app->joystick2->GetCapabilities(&g_App.joystickCaps2);
        app->joystick2->EnumObjects(EnumAxesCallback2, NULL, DIDFT_ALL);
        LogPrintf(&g_Logger, "もひとつ有効なパッドを発見しました\r\n");
    }
    return 0;
}

int OpenGameArchive()
{
    char path[0x80];
    int size;

    if (g_Archive.Open("th09.dat"))
    {
        sprintf(path, "th09_%.4x%c.ver", 0x150, 'a');
        g_App.verFile = (char *)LoadFileData(path, &size, 0);
        g_App.verFileSize = size;
        if (g_App.verFile == NULL)
        {
            LogError(&g_Logger, "error : データのバージョンが違います\r\n");
            return -1;
        }
    }
    else
    {
        LogError(&g_Logger, "error : データファイルが存在しません\r\n");
        return -1;
    }
    return 0;
}

int MeasureRefreshRate()
{
    int count;
    int before;
    int ticks;
    float values[30];
    int validCount;
    float fpsRate;
    int interval;
    int nowTime;
    float sum;
    int k;

    count = 0;
    ticks = 0;
    validCount = 0;
    timeBeginPeriod(1);
    before = timeGetTime();
    timeEndPeriod(1);
    while (count < 600 && validCount < 4)
    {
        g_App.d3dDevice->BeginScene();
        g_SpriteEngine->CopySurfaceToBackBuffer(8, 0, 0, 0, 0);
        g_App.d3dDevice->EndScene();
        if (FAILED(g_App.d3dDevice->Present(NULL, NULL, NULL, NULL)))
        {
            #ifdef TH09_NATIVE_NETPLAY
            ui::ReleaseFonts();
            #endif
            g_App.d3dDevice->Reset(&g_App.presentParams);
        }
        count++;
        timeBeginPeriod(1);
        nowTime = timeGetTime();
        timeEndPeriod(1);
        ticks++;
        interval = nowTime - before;
        if (interval >= 700)
        {
            before = nowTime;
            ticks = 0;
        }
        else if (interval >= 500)
        {
            fpsRate = ticks * 1000.0f / interval;
            if (fpsRate >= 57.0f)
            {
                values[validCount] = fpsRate;
                validCount++;
            }
            before = nowTime;
            ticks = 0;
        }
    }
    if (!g_App.GetNoVsync())
    {
        sum = 0.0f;
        if (validCount >= 2)
        {
            for (k = 0; k < validCount; k++)
            {
                sum += values[k];
            }
            sum /= k;
        }
        else
        {
            sum = 1000.0f;
        }
        if (sum > 160.0f)
        {
            LogPrintf(&g_Logger, "垂直同期が取れてないか、リフレッシュレートが高すぎます\r\n");
            LogPrintf(&g_Logger, "強制６０フレームモードで動作します\r\n");
            g_App.noVsync = 1;
            return -2;
        }
        else if (sum >= 65.0f)
        {
            LogPrintf(&g_Logger, "垂直同期が取れてないか、リフレッシュレートが高すぎます。\r\n");
            LogPrintf(&g_Logger, "強制６０フレームモードで動作します\r\n");
            g_App.noVsync = 1;
            return -2;
        }
    }
    return 0;
}

void SpriteEngine::ReleaseVertexBuffer()
{
    if (this->vertexBuffer != NULL)
    {
        this->vertexBuffer->Release();
        this->vertexBuffer = NULL;
    }
}

int SpriteEngine::SetPendingSurfaceCopy(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight,
                                        int dstLeft, int dstTop, int dstWidth, int dstHeight)
{
    if (this->pendingSurfaceCopySurface >= 0)
    {
        return -1;
    }
    this->pendingSurfaceCopySurface = texture;
    this->pendingSurfaceCopySrcLeft = srcLeft;
    this->pendingSurfaceCopySrcTop = srcTop;
    this->pendingSurfaceCopySrcWidth = srcWidth;
    this->pendingSurfaceCopySrcHeight = srcHeight;
    this->pendingSurfaceCopyDstLeft = dstLeft;
    this->pendingSurfaceCopyDstTop = dstTop;
    this->pendingSurfaceCopyDstWidth = dstWidth;
    this->pendingSurfaceCopyDstHeight = dstHeight;
    return 0;
}

void Timer::Add(float frames)
{
    this->previous = this->current;
    if (g_App.gameSpeed > 0.99f)
    {
        this->currentF += frames;
    }
    else
    {
        this->currentF += g_App.gameSpeed * frames;
    }
    this->current = this->currentF;
    if (g_App.stateFlags & 0x20)
    {
        this->previous = -999999;
        this->currentF += this->current;
    }
}

void App::TickTimer(int *frames, float *subFrame)
{
    if (this->gameSpeed <= 0.99f)
    {
        *subFrame += this->gameSpeed;
        *frames = *subFrame;
    }
    else
    {
        (*frames)++;
        *subFrame += 1.0f;
    }
}

int App::SaveScreenshot(char *path)
{
    BITMAPFILEHEADER fileHdr;
    BYTE *bits;
    BITMAPINFO *bmi;
    IDirect3DSurface8 *buffer;

    bmi = NULL;
    bits = NULL;
    buffer = NULL;
    this->d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &buffer);
    memset(&fileHdr, 0, sizeof(fileHdr));
    fileHdr.bfType = *(WORD *)"BM";
    fileHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHdr.bfSize = fileHdr.bfOffBits;
    switch (this->presentParams.BackBufferFormat)
    {
    case D3DFMT_R5G6B5:
        LogPrintf(&g_Logger, "16bit は取り込めない\r\n");
        break;
    case D3DFMT_X8R8G8B8:
    {
        int widthBytes;
        int x;
        int y;
        BYTE *dst;
        BYTE *read;
        D3DLOCKED_RECT lockRect;
        int outRow;
        BYTE *pBits;
        int pitch;
        HANDLE file;

        bmi = (BITMAPINFO *)g_MemoryTracker.Alloc(sizeof(BITMAPINFO), GLOBAL_H);
        if (bmi == NULL)
        {
            LogPrintf(&g_Logger, "snapShotScreen : 確保しくり\r\n");
            break;
        }
        memset(bmi, 0, sizeof(BITMAPINFO));
        widthBytes = 640 * 3;
        bits = (BYTE *)g_MemoryTracker.Alloc(widthBytes * 480, GLOBAL_H);
        if (bits == NULL)
        {
            LogPrintf(&g_Logger, "snapShotScreen : 確保しくり\r\n");
            goto done;
        }
        fileHdr.bfSize += widthBytes * 480;
        bmi->bmiHeader.biBitCount = 24;
        bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi->bmiHeader.biWidth = 640;
        bmi->bmiHeader.biHeight = 480;
        bmi->bmiHeader.biPlanes = 1;
        bmi->bmiHeader.biCompression = BI_RGB;
        buffer->LockRect(&lockRect, NULL, 0);

        pBits = (BYTE *)lockRect.pBits;
        pitch = lockRect.Pitch;
        outRow = 0;
        for (y = 479; y > -1; y--, outRow++)
        {
            dst = bits + widthBytes * outRow;
            read = pBits + pitch * y;
            for (x = 0; x < 640; x++)
            {
                *dst = *read;
                read++;
                dst++;
                *dst = *read;
                read++;
                dst++;
                *dst = *read;
                read += 2;
                dst++;
            }
        }
        buffer->UnlockRect();
        file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE)
        {
            break;
        }

        WriteFile(file, &fileHdr, sizeof(BITMAPFILEHEADER), (DWORD *)&path, NULL);
        WriteFile(file, bmi, sizeof(BITMAPINFOHEADER), (DWORD *)&path, NULL);
        WriteFile(file, bits, widthBytes * 480, (DWORD *)&path, NULL);
        CloseHandle(file);
        break;
    }
    default:
        LogPrintf(&g_Logger, "error ? .\\system\\mother.cpp\r\n");
        return 1;
    }
done:
    if (buffer != NULL)
    {
        buffer->Release();
        buffer = NULL;
    }
    g_MemoryTracker.Free(bmi);
    g_MemoryTracker.Free(bits);
    return 0;
}

void SpriteEngine::ResetFrameState()
{
    this->ClearVertexFormat();
    this->ClearSpriteData();
    this->ClearTexture();
    this->ClearColorOp();
    this->ClearBlendMode();
    this->ClearZWrite();
    this->ClearDrawStats();
    this->ClearLastCameraSetupFlag();
    this->ResetColorModulation();
    this->shakeY = 0.0f;
    this->shakeX = 0.0f;
}

int App::LoadConfig(char *path)
{
    GameConfig *file;
    int size;

    g_App.config.SetDefaults();
    file = (GameConfig *)LoadFileData(path, &size, 1);
    if (file == NULL)
    {
        LogPrintf(&g_Logger, "コンフィグデータが見つからないので初期化しました\r\n");
    }
    else
    {
        g_App.config = *file;
        g_MemoryTracker.Free(file);
        if (g_App.config.unk_ac < 3 && g_App.config.colorMode16Bits < 2 && g_App.config.musicMode < 3 &&
            g_App.config.difficulty < 6 && g_App.config.playSounds < 2 && g_App.config.windowed < 2 &&
            g_App.config.frameSkip < 3 && g_App.config.unk_b3 < 3 && g_App.config.version == 0x90003 &&
            size == sizeof(GameConfig))
        {
            g_PlayerInputs[0].mappings[0] = g_App.config.mappings[0][0];
            g_PlayerInputs[0].mappings[1] = g_App.config.mappings[0][1];
            g_PlayerInputs[0].mappings[2] = g_App.config.mappings[0][2];
            g_PlayerInputs[1].mappings[0] = g_App.config.mappings[1][0];
            g_PlayerInputs[1].mappings[1] = g_App.config.mappings[1][1];
            g_PlayerInputs[1].mappings[2] = g_App.config.mappings[1][2];
            goto done;
        }
        LogPrintf(&g_Logger, "コンフィグデータが異常でしたので再初期化しました\r\n");
    }
    g_App.config.SetDefaults();

    {
        HANDLE bgmFile;
        DWORD read;
        BgmDatHeader bgmHeader;

        bgmFile = CreateFileA("./thbgm.dat", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (bgmFile != INVALID_HANDLE_VALUE)
        {
            ReadFile(bgmFile, &bgmHeader, sizeof(BgmDatHeader), &read, NULL);
            CloseHandle(bgmFile);
            if (bgmHeader.magic != 'VAWZ' || bgmHeader.version != 1 || bgmHeader.gameVersion != 0x900)
            {
                LogError(&g_Logger, "BGM データのバージョンが違います\r\n");
                return -1;
            }
            g_App.config.musicMode = 1;
        }
        else
        {
            g_App.config.musicMode = 2;
        }
    }
done:
    this->noVsync = 0;
    if (this->GetNoFog())
    {
        LogPrintf(&g_Logger, "フォグの使用を抑制します\r\n");
    }
    if (this->GetForce16BitTextures())
    {
        LogPrintf(&g_Logger, "16Bit のテクスチャの使用を強制します\r\n");
    }
    if (this->GetWindowed())
    {
        LogPrintf(&g_Logger, "ウィンドウモードで起動します\r\n");
    }
    if (this->GetReferenceRasterizer())
    {
        LogPrintf(&g_Logger, "リファレンスラスタライザを強制します\r\n");
    }
    if (this->GetNoDirectInput())
    {
        LogPrintf(&g_Logger, "パッド、キーボードの入力に DirectInput を使用しません\r\n");
    }
    if (this->GetPreloadBgm())
    {
        LogPrintf(&g_Logger, "ＢＧＭをメモリに読み込みます\r\n");
    }
    if (this->GetNoVsync())
    {
        LogPrintf(&g_Logger, "垂直同期を取りません\r\n");
        g_App.noVsync = 1;
    }
    if (this->GetNoTextRenderDetect())
    {
        LogPrintf(&g_Logger, "文字描画の環境を自動検出しません\r\n");
    }
    if (WriteToFile(path, &g_App.config, sizeof(GameConfig)) != 0)
    {
        LogError(&g_Logger, "ファイルが書き出せません %s\r\n", path);
        LogError(&g_Logger, "フォルダが書込み禁止属性になっているか、ディスクがいっぱいいっぱいになってませんか？\r\n");
        return -1;
    }
    return 0;
}

void SoundPlayer::UpdateBgmFades()
{
    if (this->bgm != NULL)
    {
        this->bgm->UpdateFade();
        this->bgm->UpdateFadeIn();
        this->bgm->UpdateFadeDown();
        this->bgm->UpdateFadeUp();
    }
}

int App::LoadBgm(int id)
{
    char *dot;
    char wavFile[0x100];
    DWORD i;

    for (i = 0; i < ARRAY_SIZE(g_BgmInfos); i++)
    {
        if (g_BgmInfos[i].id == id)
        {
            break;
        }
    }
    if (i >= ARRAY_SIZE(g_BgmInfos))
    {
        return 1;
    }
    if (g_App.config.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->LoadFile(g_BgmInfos[i].id, g_BgmInfos[i].path);
        }
        return 0;
    }
    if (g_App.config.musicMode == 1)
    {
        strcpy(wavFile, g_BgmInfos[i].path);
        dot = strrchr(wavFile, '.');
        dot[1] = 'w';
        dot[2] = 'a';
        dot[3] = 'v';
        g_SoundPlayer.QueueSoundCommand(1, g_BgmInfos[i].id, wavFile);
    }
    return 1;
}

int App::PlayBgm()
{
    if (g_App.config.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->Play();
        }
    }
    else if (g_App.config.musicMode == 1)
    {
        g_SoundPlayer.QueueSoundCommand(10, 0, "dummy");
    }
    return 0;
}

int App::StopMusic()
{
    if (g_App.config.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->Stop();
        }
    }
    else if (g_App.config.musicMode == 1)
    {
        if (g_App.GetPreloadBgm())
        {
            g_SoundPlayer.QueueSoundCommand(4, 0, "dummy");
        }
        else
        {
            g_SoundPlayer.QueueSoundCommand(3, 0, "dummy");
        }
    }
    else
    {
        return -1;
    }
    return 0;
}

int App::UnloadBgm(int id)
{
    DWORD i;
    int slot;

    for (i = 0; i < ARRAY_SIZE(g_BgmInfos); i++)
    {
        if (g_BgmInfos[i].id == id)
        {
            break;
        }
    }
    if (i >= ARRAY_SIZE(g_BgmInfos))
    {
        return 1;
    }
    slot = g_BgmInfos[i].id;
    if (g_App.config.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->UnloadFile(slot);
        }
    }
    else if (g_App.config.musicMode == 1)
    {
        g_SoundPlayer.QueueSoundCommand(9, slot, "dummy");
    }
    else
    {
        return -1;
    }
    return 0;
}

int App::FadeOutMusic(float seconds)
{
    float duration;

    if (g_App.config.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->FadeOut(1000.0f * seconds);
        }
    }
    else if (g_App.config.musicMode == 1)
    {
        if (this->gameSpeed == 0.0f)
        {
            duration = seconds;
        }
        else if (this->gameSpeed > 1.0f)
        {
            duration = seconds;
        }
        else
        {
            duration = seconds / this->gameSpeed;
        }
        g_SoundPlayer.QueueSoundCommand(5, duration, "");
    }
    else
    {
        return -1;
    }
    return 0;
}

int App::EnableFog()
{
    g_SpriteEngine->FlushQuadBatch();
    if (this->fogState != 1)
    {
        this->fogState = 1;
        return this->d3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
    }
    return 0;
}

int App::DisableFog()
{
    g_SpriteEngine->FlushQuadBatch();
    if (this->fogState != 0)
    {
        this->fogState = 0;
        return this->d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    }
    return 0;
}

int App::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
    g_SpriteEngine->FlushQuadBatch();
    return this->d3dDevice->SetRenderState(state, value);
}

void App::AddPlayTime()
{
    DWORD now = timeGetTime();
    DWORD delta;

    if (now < this->lastPlayTimeTick)
    {
        this->lastPlayTimeTick = now;
    }
    delta = now - this->lastPlayTimeTick;
    g_RunTime.hours += delta / 3600000;
    delta %= 3600000;
    g_RunTime.minutes += delta / 60000;
    delta %= 60000;
    g_RunTime.seconds += delta / 1000;
    delta %= 1000;
    g_RunTime.milliseconds += delta;
    if (g_RunTime.milliseconds >= 1000)
    {
        g_RunTime.seconds += g_RunTime.milliseconds / 1000;
        g_RunTime.milliseconds %= 1000;
    }
    if (g_RunTime.seconds >= 60)
    {
        g_RunTime.minutes += g_RunTime.seconds / 60;
        g_RunTime.seconds %= 60;
    }
    if (g_RunTime.minutes >= 60)
    {
        g_RunTime.hours += g_RunTime.minutes / 60;
        g_RunTime.minutes %= 60;
    }
    this->lastPlayTimeTick = now;
}

void App::AddTotalPlayTime()
{
    DWORD now = timeGetTime();
    DWORD delta;

    if (now < this->lastTotalPlayTimeTick)
    {
        this->lastTotalPlayTimeTick = 0;
    }
    delta = now - this->lastTotalPlayTimeTick;
    g_PlayTime.hours += delta / 3600000;
    delta %= 3600000;
    g_PlayTime.minutes += delta / 60000;
    delta %= 60000;
    g_PlayTime.seconds += delta / 1000;
    delta %= 1000;
    g_PlayTime.milliseconds += delta;
    if (g_PlayTime.milliseconds >= 1000)
    {
        g_PlayTime.seconds += g_PlayTime.milliseconds / 1000;
        g_PlayTime.milliseconds %= 1000;
    }
    if (g_PlayTime.seconds >= 60)
    {
        g_PlayTime.minutes += g_PlayTime.seconds / 60;
        g_PlayTime.seconds %= 60;
    }
    if (g_PlayTime.minutes >= 60)
    {
        g_PlayTime.hours += g_PlayTime.minutes / 60;
        g_PlayTime.minutes %= 60;
    }
    this->lastTotalPlayTimeTick = now;
}

int App::CheckVersionFile(char *version, DWORD size, DWORD checksum)
{
    DWORD size2;
    char *start;
    DWORD left;
    DWORD checksum2;
    char *p;

    if (this->verFile == NULL)
    {
        return 0;
    }
    p = this->verFile;
    left = this->verFileSize;
    if (strncmp(version, "debug", 5) == 0)
    {
        return 0;
    }
    if (strcmp("0150a", "debug") == 0)
    {
        return 0;
    }
    while (left > 0)
    {
        if (strncmp(version, p, 5) == 0)
        {
            p += 6;
            sscanf(p, "%d %d", &size2, &checksum2);
            if (size2 == size && checksum2 == checksum)
            {
                return 0;
            }
        }
        start = p;
        p = strchr(p, '\n');
        if (p == NULL)
        {
            break;
        }
        p++;
        left -= p - start;
    }
    return -1;
}

void App::CloseSubThread()
{
    if (this->subThread != NULL)
    {
        this->closeSubThread = 1;
        while (WaitForSingleObject(this->subThread, 1000) == WAIT_TIMEOUT)
        {
            Sleep(1);
        }
        CloseHandle(this->subThread);
        this->subThread = NULL;
        this->closeSubThread = 0;
    }
}

void App::StartLoadingSprites(D3DXVECTOR3 *pos)
{
    if (this->loadingState == 0)
    {
        this->loadingAnm->StartScriptAtOrigin(&g_LoadingScreenSprites[0], 0);
        this->loadingAnm->StartScriptAtOrigin(&g_LoadingScreenSprites[1], 1);
        this->loadingAnm->StartScriptAtOrigin(&g_LoadingScreenSprites[2], 2);
        this->loadingState = 1;
        g_LoadingScreenSprites[0].pos = *pos;
        g_LoadingScreenSprites[1].pos = *pos;
        g_LoadingScreenSprites[2].pos = *pos;
    }
}

void App::StopLoadingSprites()
{
    if (this->loadingState == 1)
    {
        g_LoadingScreenSprites[0].SetPendingInterrupt(1);
        g_LoadingScreenSprites[1].SetPendingInterrupt(1);
        g_LoadingScreenSprites[2].SetPendingInterrupt(1);
        this->loadingState = 0;
    }
    if (g_LoadingScreenEffect != NULL)
    {
        g_LoadingScreenEffect->Finish();
        g_LoadingScreenEffect = NULL;
    }
}

void App::StartLoadingSpritesOverScreen(D3DXVECTOR3 *pos)
{
    if (this->loadingState == 0)
    {
        this->loadingAnm->StartScriptAtOrigin(&g_LoadingScreenSprites[0], 0);
        this->loadingAnm->StartScriptAtOrigin(&g_LoadingScreenSprites[1], 1);
        this->loadingAnm->StartScriptAtOrigin(&g_LoadingScreenSprites[2], 2);
        this->loadingState = 1;
        g_LoadingScreenSprites[0].pos = *pos;
        g_LoadingScreenSprites[1].pos = *pos;
        g_LoadingScreenSprites[2].pos = *pos;
    }
    g_SpriteEngine->SetPendingSurfaceCopy(8, 0, 0, 640, 480, 0, 0, 640, 480);
}

void App::FinishLoadingSprites()
{
    if (this->loadingState == 1)
    {
        g_LoadingScreenSprites[0].SetPendingInterrupt(2);
        g_LoadingScreenSprites[1].SetPendingInterrupt(2);
        g_LoadingScreenSprites[2].SetPendingInterrupt(2);
        this->loadingState = 2;
    }
    if (g_LoadingScreenEffect != NULL)
    {
        g_LoadingScreenEffect->Finish();
        g_LoadingScreenEffect = NULL;
    }
}

void App::StartLoadingScreenEffect(int type)
{
    if (g_LoadingScreenEffect == NULL)
    {
        g_LoadingScreenEffect = ScreenEffect::StartEffect(type + 5, 60, 0, 0, 0, 1, 2);
    }
}

void App::InitCriticalSections()
{
    DWORD i;

    for (i = 0; i < ARRAY_SIZE(this->criticalSections); i++)
    {
        InitializeCriticalSection(&this->criticalSections[i]);
    }
}

void App::DeleteCriticalSections()
{
    DWORD i;

    for (i = 0; i < ARRAY_SIZE(this->criticalSections); i++)
    {
        DeleteCriticalSection(&this->criticalSections[i]);
    }
}

void App::SetupCamera(Camera *camera)
{
    if (g_SpriteEngine != NULL)
    {
        g_SpriteEngine->FlushQuadBatch();
    }
    float halfWidth = (float)camera->viewport.Width * g_Half;
    float halfHeight = (float)camera->viewport.Height * g_Half;
    float aspect = (float)camera->viewport.Width / (float)camera->viewport.Height;
    D3DXVECTOR3 eye(halfWidth, halfHeight, halfHeight / tan(0.15707964f));
    D3DXVECTOR3 target(halfWidth, halfHeight, 0.0f);
    D3DXVECTOR3 up(0.0f, -1.0f, 0.0f);
    D3DXMatrixLookAtLH(&camera->viewMatrix, &eye, &target, &up);
    D3DXMatrixPerspectiveFovLH(&camera->projectionMatrix, 0.31415927f, aspect, 1.0f, 10000.0f);
    g_App.d3dDevice->SetTransform(D3DTS_VIEW, &camera->viewMatrix);
    g_App.d3dDevice->SetTransform(D3DTS_PROJECTION, &camera->projectionMatrix);
    if (g_SpriteEngine != NULL)
    {
        g_SpriteEngine->shakeX = camera->shakeX;
        g_SpriteEngine->shakeY = camera->shakeY;
    }
}

void App::SetupCamera3D(Camera *camera)
{
    if (g_SpriteEngine != NULL)
    {
        g_SpriteEngine->FlushQuadBatch();
    }
    D3DXVECTOR3 at = camera->at + camera->eye;
    D3DXVECTOR3 eye = camera->offset + camera->eye;
    D3DXMatrixLookAtLH(&camera->viewMatrix, &eye, &at, &camera->up);
    D3DXMatrixPerspectiveFovLH(&camera->projectionMatrix, camera->fov,
                               (float)camera->viewport.Width / (float)camera->viewport.Height, 30.0f, 1800.0f);
    g_App.d3dDevice->SetTransform(D3DTS_VIEW, &camera->viewMatrix);
    g_App.d3dDevice->SetTransform(D3DTS_PROJECTION, &camera->projectionMatrix);
    D3DXVec3Cross(&camera->cameraRight, &camera->at, &camera->up);
    D3DXVec3Normalize(&camera->cameraRight, &camera->cameraRight);
    if (g_SpriteEngine != NULL)
    {
        g_SpriteEngine->shakeX = camera->shakeX;
        g_SpriteEngine->shakeY = camera->shakeY;
    }
}

void App::InitCameras()
{
    g_App.cameras[2].eye = D3DXVECTOR3(0.0f, 0.0f, 1000.0f);
    g_App.cameras[2].at = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    g_App.cameras[2].up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    g_App.cameras[2].fov = 0.5235988f;
    g_App.cameras[2].viewport.X = 0;
    g_App.cameras[2].viewport.Y = 0;
    g_App.cameras[2].viewport.Width = 640;
    g_App.cameras[2].viewport.Height = 480;
    g_App.cameras[2].viewport.MinZ = 0.0f;
    g_App.cameras[2].viewport.MaxZ = 1.0f;
    g_App.cameras[2].index = 2;
    g_App.cameras[2].offset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    g_App.cameras[0].eye = D3DXVECTOR3(0.0f, 0.0f, 1000.0f);
    g_App.cameras[0].at = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    g_App.cameras[0].up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    g_App.cameras[0].fov = 0.5235988f;
    g_App.cameras[0].viewport.X = 16;
    g_App.cameras[0].viewport.Y = 16;
    g_App.cameras[0].viewport.Width = 288;
    g_App.cameras[0].viewport.Height = 448;
    g_App.cameras[0].viewport.MinZ = 0.0f;
    g_App.cameras[0].viewport.MaxZ = 1.0f;
    g_App.cameras[0].index = 0;
    g_App.cameras[0].offset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    g_App.cameras[1].eye = D3DXVECTOR3(0.0f, 0.0f, 1000.0f);
    g_App.cameras[1].at = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    g_App.cameras[1].up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    g_App.cameras[1].fov = 0.5235988f;
    g_App.cameras[1].viewport.X = 336;
    g_App.cameras[1].viewport.Y = 16;
    g_App.cameras[1].viewport.Width = 288;
    g_App.cameras[1].viewport.Height = 448;
    g_App.cameras[1].viewport.MinZ = 0.0f;
    g_App.cameras[1].viewport.MaxZ = 1.0f;
    g_App.cameras[1].index = 1;
    g_App.cameras[1].offset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}

int App::OnUpdate(App *app)
{
#ifdef TH09_NATIVE_NETPLAY
    thnet::BeginFrame();
#endif
    app->unk_478 = 0;
    if (g_Network->unk_a8 == 0)
    {
        ReadInputs();
    }
    else if (!app->HasSubThread())
    {
        for (;;)
        {
            switch (app->UpdateNetPlay())
            {
            case 1:
                continue;
            case 2:
                return 3;
            case 3:
                g_Network->Close();
                g_Network->ClearUnknownA8();
                g_Network->unk_b8 = 0;
                app->scene = 1;
                break;
            case 4:
                g_Network->Close();
                return 4;
            }
            break;
        }
    }
    if ((app->stateFlags >> 7 & 1) && !app->HasSubThread())
    {
        return 4;
    }
    g_App.fogState = 0xff;
    g_SpriteEngine->ResetFrameState();
    g_App.cameras[0].shakeX = 0.0f;
    g_App.cameras[0].shakeY = 0.0f;
    g_App.cameras[1].shakeX = 0.0f;
    g_App.cameras[1].shakeY = 0.0f;
    g_App.cameras[2].shakeX = 0.0f;
    g_App.cameras[2].shakeY = 0.0f;
    g_SpriteEngine->StepSprites(g_LoadingScreenSprites, 3);
    if (g_SpriteEngine->StepAnmLoading() != 0)
    {
        return 4;
    }
    if (app->loadingThreadState != 0)
    {
        if (app->loadingThreadState == 2)
        {
            return 4;
        }
        if (app->subThreadRunning != 0)
        {
            return 1;
        }
        app->loadingThreadState = 0;
    }
    g_SoundPlayer.UpdateBgmFades();
    if (app->curScene != app->scene)
    {
        app->prevScene = app->curScene;
        switch (app->curScene)
        {
        case 0:
        title:
            app->scene = 1;
            g_App.d3dDevice->ResourceManagerDiscardBytes(0);
            if (SetUpTitleScene(0) != 0)
            {
                return 4;
            }
            break;
        case 1:
            switch (app->scene)
            {
            case -1:
                return 4;
            case 2:
                if (GameManager::RegisterChain() != 0)
                {
                    return 4;
                }
                break;
            case 4:
                return 5;
            case 6:
                app->scene = 1;
                g_App.d3dDevice->ResourceManagerDiscardBytes(0);
                if (SetUpTitleScene(1) != 0)
                {
                    return 4;
                }
                break;
            case 9:
                if (Ending::RegisterChain() != 0)
                {
                    return 4;
                }
                break;
            }
            break;
        case 5:
        case 6:
        case 8:
            switch (app->scene)
            {
            case -1:
                return 4;
            case 1:
                app->scene = 0;
                goto title;
            }
            break;
        case 2:
            switch (app->scene)
            {
            case -1:
                return 4;
            case 1:
                RemoveGameManagerChain();
                app->scene = 0;
                goto title;
            case 6:
                RemoveGameManagerChain();
                app->scene = 1;
                g_App.d3dDevice->ResourceManagerDiscardBytes(0);
                if (SetUpTitleScene(1) != 0)
                {
                    return 4;
                }
                break;
            case 10:
            case 13:
                RemoveGameManagerChain();
                if (GameManager::RegisterChain() != 0)
                {
                    return 4;
                }
                app->scene = 2;
                break;
            case 11:
                g_App.unk_59c = 1;
                RemoveGameManagerChain();
                if (GameManager::RegisterChain() != 0)
                {
                    return 4;
                }
                app->scene = 2;
                break;
            case 12:
                g_App.scene = 3;
                RemoveGameManagerChain();
                if (GameManager::RegisterChain() != 0)
                {
                    return 4;
                }
                app->scene = 2;
                break;
            case 3:
                RemoveGameManagerChain();
                if (GameManager::RegisterChain() != 0)
                {
                    return 4;
                }
                app->scene = 2;
                break;
            case 7:
                RemoveGameManagerChain();
                app->scene = 0;
                app->scene = 1;
                g_App.d3dDevice->ResourceManagerDiscardBytes(0);
                if (SetUpTitleScene(1) != 0)
                {
                    return 4;
                }
                break;
            case 9:
                RemoveGameManagerChain();
                if (Ending::RegisterChain() != 0)
                {
                    return 4;
                }
                break;
            }
            break;
        case 9:
            switch (app->scene)
            {
            case -1:
                return 4;
            case 1:
                app->scene = 0;
                goto title;
            case 6:
                app->scene = 1;
                g_App.d3dDevice->ResourceManagerDiscardBytes(0);
                if (SetUpTitleScene(1) != 0)
                {
                    return 4;
                }
                break;
            }
            break;
        }
    }
    app->curScene = app->scene;
    app->frames++;
    if (app->frames % 4000 == 3999)
    {
        if (g_App.CheckVersionFile("0150a", g_App.exeSize, g_App.exeChecksum) != 0)
        {
            return 4;
        }
    }
    if (g_Unknown4ac884 != 0)
    {
        g_Unknown4ac884--;
    }
    return 1;
}

App::App()
{
    memset(this, 0, sizeof(*this));
    this->stateFlags |= 0x140;
}

int App::OnDrawLogo(App *app)
{
    g_SpriteEngine->BeginQuadBatch();
    if (app->loadingState >= 2 && ++app->loadingState >= 5)
    {
        D3DXVECTOR3 pos(288.0f, 454.0f, 0.0f);
        g_AsciiText.SetScale(0.5f, 0.5f);
        if (app->loadingState < 35)
        {
            g_AsciiText.SetAlpha(255 - (app->loadingState - 5) * 128 / 30);
        }
        else
        {
            g_AsciiText.SetAlpha(255 - (65 - app->loadingState) * 128 / 30);
        }
        g_AsciiText.AddFormat(&pos, "Press Shot Button");
        g_AsciiText.SetScale(1.0f, 1.0f);
        g_AsciiText.DrawStrings();
        g_AsciiText.ClearStrings();
        if (app->loadingState >= 65)
        {
            app->loadingState = 5;
        }
    }
    if (app->loadingState != 0)
    {
        g_SpriteEngine->CopySurfaceToBackBuffer(8, 0, 0, 0, 0);
    }
    else if (g_SpriteEngine->GetSurface(8) != NULL)
    {
        g_SpriteEngine->ReleaseSurface(8);
    }
    return 1;
}

void DrawFps(int draw)
{
    static DWORD frames;
    static char fpsText[0x100];


    static DWORD lastCounter[2];
    static DWORD measures;
    static char speedText[0x100];
    float fps;
    float duration;
    DWORD now;

    frames += 1 + g_App.config.frameSkip;
    if (g_App.perfFrequency[0] == 0)
    {
        static DWORD lastTime = timeGetTime();
        now = timeGetTime();

        if (now < lastTime)
        {
            lastTime = now;
            frames = 0;
        }
        if (now - lastTime >= 500)
        {
            duration = (now - lastTime) * 0.001f;
            lastTime = now;
        measured:
            fps = frames / duration;
            frames = 0;
            sprintf(fpsText, "%.02ffps", fps);
            if ((g_GameManager.stateFlags & 4) && draw)
            {
                float target = 60.0f;

                g_App.expectedFrames += target;
                if (fps > target * 0.9f)
                {
                    g_App.achievedFrames += target;
                }
                else if (fps > target * 0.7f)
                {
                    g_App.achievedFrames += target * 0.8f;
                }
                else if (fps > target * 0.5f)
                {
                    g_App.achievedFrames += target * 0.6f;
                }
                else
                {
                    g_App.achievedFrames += target * 0.5f;
                }
                if (!g_GameManager.IsReplay())
                {
                    g_App.roundedFps = fps + 0.5f;
                }
                else
                {
                    sprintf(speedText, "%2d", g_App.roundedFps);
                }
            }
        }
    }
    else
    {
        LARGE_INTEGER counter;

        if (lastCounter[0] == 0)
        {
            QueryPerformanceCounter((LARGE_INTEGER *)lastCounter);
        }
        QueryPerformanceCounter(&counter);
        if (counter.LowPart < lastCounter[0])
        {
            *(LARGE_INTEGER *)lastCounter = counter;
            frames = 0;
        }
        if (counter.LowPart >= lastCounter[0] + (g_App.perfFrequency[0] >> 1))
        {
            duration = (float)(counter.LowPart - lastCounter[0]) / (float)g_App.perfFrequency[0];
            *(LARGE_INTEGER *)lastCounter = counter;
            measures++;
            goto measured;
        }
    }
    if (g_App.hideFps == 0 && draw && (g_GameManager.stateFlags & 4))
    {
        D3DXVECTOR3 pos(280.0f, 466.0f, 0.0f);

        g_AsciiText.AddString(&pos, fpsText);
    }
}

int App::PlayMusicById(int id, int arg)
{
    DWORD i;

    for (i = 0; i < ARRAY_SIZE(g_BgmInfos); i++)
    {
        if (g_BgmInfos[i].id == id)
        {
            break;
        }
    }
    if (i >= ARRAY_SIZE(g_BgmInfos))
    {
        return 1;
    }
    int heard = g_BgmInfos[i].unk_0c;
    int slot = g_BgmInfos[i].id;
    if (g_App.config.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->Restart(slot);
        }
        if (!g_GameManager.IsReplay() && !g_GameManager.IsDemo())
        {
            g_PlayRecord.bgmHeard[heard] = 1;
        }
        return 0;
    }
    if (g_App.config.musicMode == 1)
    {
        if (g_App.GetPreloadBgm())
        {
            g_SoundPlayer.QueueSoundCommand(4, 0, "dummy");
        }
        g_SoundPlayer.QueueSoundCommand(2, slot, "dummy");
        if (!g_GameManager.IsReplay() && !g_GameManager.IsDemo())
        {
            g_PlayRecord.bgmHeard[heard] = 1;
        }
    }
    g_BgmUnknown08 = g_BgmInfos[i].unk_08;
    return 0;
}

int App::PlayMusic(char *path, int heard)
{
    char *dot;
    char wavFile[0x100];

    if (g_App.config.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->PlayFile(path);
        }
        if (!g_GameManager.IsReplay() && !g_GameManager.IsDemo())
        {
            g_PlayRecord.bgmHeard[heard] = 1;
        }
    }
    else if (g_App.config.musicMode == 1)
    {
        strcpy(wavFile, path);
        dot = strrchr(wavFile, '.');
        dot[1] = 'w';
        dot[2] = 'a';
        dot[3] = 'v';
        g_SoundPlayer.QueueSoundCommand(2, -1, wavFile);
        if (!g_GameManager.IsReplay() && !g_GameManager.IsDemo())
        {
            g_PlayRecord.bgmHeard[heard] = 1;
        }
    }
    else
    {
        return -1;
    }
    return 0;
}

#ifdef TH09_MODERN
struct ModernThreadJob
{
    void (__fastcall *callback)(void *);
    void *argument;
};
static DWORD WINAPI ModernThreadEntry(void *data)
{
    ModernThreadJob job = *(ModernThreadJob *)data;
    delete (ModernThreadJob *)data;
    job.callback(job.argument);
    return 0;
}
#endif
int App::StartSubThread(LPTHREAD_START_ROUTINE proc, LPVOID arg)
{
    this->CloseSubThread();
#ifdef TH09_MODERN
    ModernThreadJob *job = new ModernThreadJob;
    job->callback = (void (__fastcall *)(void *))proc;
    job->argument = arg;
    this->subThread = CreateThread(NULL, 0, ModernThreadEntry, job, 0, &this->subThreadId);
    if (this->subThread == NULL) delete job;
#else
    this->subThread = CreateThread(NULL, 0, proc, arg, 0, &this->subThreadId);
#endif
    this->subThreadRunning = 1;
    return this->subThread != NULL ? 0 : -1;
}

int App::OnDraw(App *app)
{
    DrawFps(1);
    if (app->unk_478 != 0)
    {
        g_App.SetView(2);
        g_SpriteEngine->DrawUnrotated(g_App.unk_584);
    }
    return 1;
}

int App::DeletedCallback(App *app)
{
    g_App.CloseSubThread();
    if (g_App.verFile != NULL)
    {
        g_MemoryTracker.Free(g_App.verFile);
        g_App.verFile = NULL;
    }
    if (g_GameManager.sides[0].stats != NULL)
    {
        delete g_GameManager.sides[0].stats;
    }
    g_GameManager.sides[0].stats = NULL;
    if (g_GameManager.sides[1].stats != NULL)
    {
        delete g_GameManager.sides[1].stats;
    }
    g_GameManager.sides[1].stats = NULL;
    if (g_App.unk_584 != NULL)
    {
        delete g_App.unk_584;
        g_App.unk_584 = NULL;
    }
    g_SpriteEngine->ReleaseVertexBuffer();
    g_SpriteEngine->ReleaseAnm(0);
    g_SpriteEngine->ReleaseAnm(2);
    g_SpriteEngine->ReleaseSurface(8);
    AsciiText::RemoveChain();
    g_SoundPlayer.QueueSoundCommand(4, 0, "dummy");
    if (g_App.config.musicMode == 2 && g_App.midi != NULL)
    {
        g_App.midi->Restart(0x1e);
    }
    ReplayManager::Release(g_GameManager.replayManager);
    g_GameManager.replayManager = NULL;
    ReleaseTextSurface();
    if (app->keyboard)
    {
        app->keyboard->Unacquire();
    }
    if (app->keyboard)
    {
        app->keyboard->Release();
        app->keyboard = NULL;
    }
    if (app->joystick)
    {
        app->joystick->Unacquire();
    }
    if (app->joystick2)
    {
        app->joystick2->Unacquire();
    }
    if (app->joystick)
    {
        app->joystick->Release();
        app->joystick = NULL;
    }
    if (app->joystick2)
    {
        app->joystick2->Release();
        app->joystick2 = NULL;
    }
    if (app->dinput)
    {
        app->dinput->Release();
        app->dinput = NULL;
    }
    g_Archive.Release();
    if (g_App.timer != NULL)
    {
        g_App.timer->StopTimer();
        delete g_App.timer;
        g_App.timer = NULL;
    }
    SaveScoreDat();
    BulletManager::ReleaseAnm();
    Gui::ReleaseAnm();
    return 0;
}

void App::LoadingThread(App *app)
{
    float vol;

    g_App.hideFps = 0;
    g_App.redrawFrames = 0;
    g_App.lastPlayTimeTick = timeGetTime();
    InitDirectInput(&g_App);
    if (g_App.midi == NULL)
    {
        g_App.midi = (MidiPlayer *)g_MemoryTracker.Register(new MidiPlayer(), sizeof(MidiPlayer), "MidiSysInf");
    }
    if (g_App.midi != NULL)
    {
        g_App.midi->LoadFile(0x1e, "bgm/init.mid");
    }
    g_SoundPlayer.LoadSounds();
    g_App.textAnm = g_SpriteEngine->PreloadAnm(0, "text.anm");
    if (g_App.textAnm == NULL)
    {
        goto fail;
    }
    g_App.unk_584 = (Sprite *)g_MemoryTracker.Register(new Sprite(), sizeof(Sprite), "SprtInf");
    g_App.textAnm->InitSprite(g_App.unk_584, 0x1f);
    g_App.unk_584->pos.x = 320.0f;
    g_App.unk_584->pos.y = 240.0f;
    g_App.unk_584->pos.z = 0.0f;
    if (AsciiText::RegisterChain() != 0)
    {
        if (g_App.closeSubThread)
        {
            return;
        }
        LogPrintf(&g_Logger, "error : 文字の初期化に失敗しました\r\n");
        goto fail;
    }
    if (BulletManager::LoadAnm())
    {
        if (g_App.closeSubThread)
        {
            return;
        }
        LogPrintf(&g_Logger, "error : 敵弾の初期化に失敗しました\r\n");
        goto fail;
    }
    if (Gui::LoadAnm())
    {
        if (g_App.closeSubThread)
        {
            return;
        }
        LogPrintf(&g_Logger, "error : 表示物の初期化に失敗しました\r\n");
        goto fail;
    }
    if (g_SoundPlayer.LoadBgmFormat("bgm/thbgm.fmt") != 0)
    {
        if (g_App.closeSubThread)
        {
            return;
        }
        LogPrintf(&g_Logger, "error : BGM の初期化に失敗しました\r\n");
        goto fail;
    }
    g_SoundPlayer.bgmVolume = g_App.config.bgmVolume;
    g_SoundPlayer.seVolume = g_App.config.seVolume;
    vol = g_SoundPlayer.bgmVolume / 100.0f;
    if (g_SoundPlayer.seVolume != 0)
    {
        vol = (vol = (vol = 1.0f - vol) * vol) * vol;
        vol = 1.0f - vol;
        g_SoundPlayer.bgmBufferVolume = -5000 - (int)(vol * -5000.0f);
    }
    else
    {
        g_SoundPlayer.bgmBufferVolume = -10000;
    }
    if (g_SoundPlayer.bgmFileOffset == 0)
    {
        if (!g_App.GetPreloadBgm())
        {
            g_SoundPlayer.LoadBgm("thbgm.dat");
        }
        else
        {
            strcpy(g_SoundPlayer.bgmPath, "thbgm.dat");
        }
    }
    else
    {
        if (!g_App.GetPreloadBgm())
        {
            g_SoundPlayer.LoadBgm("th09.dat");
        }
        else
        {
            strcpy(g_SoundPlayer.bgmPath, "th09.dat");
        }
    }
    LoadScoreFile();
    if (g_App.stateFlags >> 6 & 1)
    {
        g_App.timer = (MidiTimer *)g_MemoryTracker.Register(new MidiTimer(), sizeof(MidiTimer), "DummyTimerSysInf");
        if (g_App.timer != NULL)
        {
            g_App.timer->StartTimer();
        }
    }
    g_App.ResetSubThread();
    g_App.loadingThreadState = 0;
    g_App.stateFlags &= ~0x100;
    g_GameManager.sides[0].character = 0;
    g_GameManager.sides[1].character = 1;
    return;

fail:
    g_App.ResetSubThread();
    g_App.loadingThreadState = 2;
    g_App.stateFlags |= 0x80;
}

int App::AddedCallback(App *app)
{
    g_App.gameSpeed = 1.0f;
    g_GameManager.Init();
    if (OpenGameArchive() != 0)
    {
        return -1;
    }
    g_SpriteEngine->LoadSurface(8, "title/th09logo.jpg");
    app->loadingAnm = g_SpriteEngine->LoadAnm(2, "nowloading.anm");
    if (app->loadingAnm == NULL)
    {
        g_SpriteEngine->ReleaseSurface(0);
        return -1;
    }
    g_App.hideFps = 1;
    if (g_App.noVsync == 0 && MeasureRefreshRate() != 0)
    {
        g_SpriteEngine->ReleaseSurface(0);
        return -2;
    }
    g_SpriteEngine->CreateQuad();
    CreateTextSurface();
    D3DXVECTOR3 position(500.0f, 440.0f, 0.0f);
    g_App.StartLoadingSprites(&position);
    g_App.loadingThreadState = 1;
    g_App.StartSubThread((LPTHREAD_START_ROUTINE)App::LoadingThread, app);
    return 0;
}

int App::RegisterChain()
{
    App *app = &g_App;
    ChainElem *elem;
    int ret;

    app->curScene = 0;
    app->scene = -1;
    app->frames = 0;
    elem = g_Chain.CreateElem((ChainCallback)App::OnUpdate);
    elem->arg = app;
    elem->addedCallback = (ChainCallback)App::AddedCallback;
    elem->deletedCallback = (ChainCallback)App::DeletedCallback;
    ret = g_Chain.AddToCalcChain(elem, 0);
    if (ret != 0)
    {
        return ret;
    }
    elem = g_Chain.CreateElem((ChainCallback)App::OnDraw);
    elem->arg = app;
    g_Chain.AddToDrawChain(elem, 0x1f);
    elem = g_Chain.CreateElem((ChainCallback)App::OnDrawLogo);
    elem->arg = app;
    g_Chain.AddToDrawChain(elem, 0);
    elem = g_Chain.CreateElem((ChainCallback)App::OnDrawLoading);
    elem->arg = app;
    g_Chain.AddToDrawChain(elem, 2);
    return 0;
}
