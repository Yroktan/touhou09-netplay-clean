#ifdef TH09_NATIVE_NETPLAY
#include "UiText.h"
#endif


#include <direct.h>


#define _WIN32_DCOM

#include <windows.h>
#define NO_SHDOCVW_GUIDS
#include <shlguid.h>
#include <shobjidl.h>
#ifndef TH09_MODERN
#include <winnls32.h>
#else

static BOOL WINAPI ModernEnableIME(HWND window, BOOL enabled)
{
    typedef BOOL (WINAPI *EnableIME)(HWND, BOOL);
    EnableIME legacy = (EnableIME)GetProcAddress(GetModuleHandleA("user32.dll"), "WINNLSEnableIME");
    return legacy ? legacy(window, enabled) : TRUE;
}
#define WINNLSEnableIME ModernEnableIME
#endif

#include "chain.h"
#include "gamemanager.h"
#include "midi.h"
#include "network.h"
#include "screeneffect.h"
#include "sound.h"
#include "sprite.h"
#include "stage.h"
#include "unsorted.h"

Network::Network()
{
    memset(this, 0, sizeof(Network));
    this->syncFrames = 60;
}

double GameWindow::GetTime()
{
    if (g_GameWindow.perfFrequency[0] != 0)
    {

        DWORD count[2];

        QueryPerformanceCounter((LARGE_INTEGER *)count);
        return (double)count[0] / (double)g_GameWindow.perfFrequency[0];
    }
    else
    {
        double time;

        timeBeginPeriod(1);
        time = timeGetTime() * 0.001f;
        timeEndPeriod(1);
        return time;
    }
}

int CreateD3d()
{
    g_App.d3d = Direct3DCreate8(220);
    if (g_App.d3d == NULL)
    {
        LogError(&g_Logger, "Direct3D オブジェクトは何故か作成出来なかった\r\n");
        return 1;
    }
    return 0;
}



static char *AppendCapsFlagText(char *name, DWORD caps, DWORD flag, char *out)
{
    out += sprintf(out, name);
    if ((caps & flag) == 0)
    {
        out += sprintf(out, "不可\r\n");
    }
    else
    {
        out += sprintf(out, "可\r\n");
    }
    return out;
}


static void FormatCapsReport(D3DCAPS8 *caps, char *buffer)
{
    char *p = buffer;

    p += sprintf(p, "現在のビデオカード、及びドライバの能力詳細\r\n");
    p = AppendCapsFlagText("　走査線取得能力 : ", caps->Caps, 0x20000, p);
    p = AppendCapsFlagText("　ウィンドウモードのレンダリング : ", caps->Caps2, 0x80000, p);
    p = AppendCapsFlagText("　プレゼンテーション間隔（直接）: ", caps->PresentationIntervals, 0x80000000, p);
    p = AppendCapsFlagText("　プレゼンテーション間隔（垂直同期）: ", caps->PresentationIntervals, 0x1, p);
    p += sprintf(p, "　-- デバイス能力 ------------------------------\r\n");
    p = AppendCapsFlagText("　System -> 非ローカルVRAMブリット : ", caps->DevCaps, 0x20000, p);
    p = AppendCapsFlagText("　ハードウェア T&L : ", caps->DevCaps, 0x10000, p);
    p = AppendCapsFlagText("　非ローカルVRAMからテクスチャ取得 : ", caps->DevCaps, 0x1000, p);
    p = AppendCapsFlagText("　システムメモリからテクスチャ取得 : ", caps->DevCaps, 0x100, p);
    p = AppendCapsFlagText("　VRAM からテクスチャ取得 : ", caps->DevCaps, 0x200, p);
    p = AppendCapsFlagText("　頂点バッファにシステムメモリを使用 : ", caps->DevCaps, 0x40, p);
    p = AppendCapsFlagText("　頂点バッファにビデオメモリを使用 : ", caps->DevCaps, 0x80, p);
    p += sprintf(p, "　-- プリミティブ能力 ---------------------------\r\n");
    p = AppendCapsFlagText("　半透明処理 : ", caps->PrimitiveMiscCaps, 0x800, p);
    p = AppendCapsFlagText("　ポイントのクリッピング処理 : ", caps->PrimitiveMiscCaps, 0x100, p);
    p = AppendCapsFlagText("　プリミティブのクリッピング処理 : ", caps->PrimitiveMiscCaps, 0x200, p);
    p = AppendCapsFlagText("　法線クリップ（反時計周り） : ", caps->PrimitiveMiscCaps, 0x40, p);
    p = AppendCapsFlagText("　法線クリップ（時計周り） : ", caps->PrimitiveMiscCaps, 0x20, p);
    p = AppendCapsFlagText("　法線クリップ無し : ", caps->PrimitiveMiscCaps, 0x10, p);
    p = AppendCapsFlagText("　デプステストON/OFF切り替え : ", caps->PrimitiveMiscCaps, 0x2, p);
    p += sprintf(p, "　-- ラスタ能力 --------------------------------\r\n");
    p = AppendCapsFlagText("　異方性フィルタリング : ", caps->RasterCaps, 0x20000, p);
    p = AppendCapsFlagText("　アンチエイリアシング : ", caps->RasterCaps, 0x1000, p);
    p = AppendCapsFlagText("　ディザ処理 : ", caps->RasterCaps, 0x1, p);
    p = AppendCapsFlagText("　範囲ベースのフォグ : ", caps->RasterCaps, 0x10000, p);
    p = AppendCapsFlagText("　Zベースのフォグ : ", caps->RasterCaps, 0x200000, p);
    p = AppendCapsFlagText("　テーブルフォグ : ", caps->RasterCaps, 0x100, p);
    p = AppendCapsFlagText("　頂点フォグ : ", caps->RasterCaps, 0x80, p);
    p = AppendCapsFlagText("　デプステスト : ", caps->RasterCaps, 0x10, p);
    p += sprintf(p, "　-- シェーディング能力 -----------------------\r\n");
    p = AppendCapsFlagText("　グーローシェーディング : ", caps->ShadeCaps, 0x8, p);
    p = AppendCapsFlagText("　α成分のグーローシェーディング : ", caps->ShadeCaps, 0x4000, p);
    p = AppendCapsFlagText("　グーローシェーディングでフォグ : ", caps->ShadeCaps, 0x80000, p);
    p += sprintf(p, "　-- テクスチャ能力 ---------------------------\r\n");
    p += sprintf(p, "　最大テクスチャサイズ : (%d, %d)\r\n", caps->MaxTextureWidth, caps->MaxTextureHeight);
    p = AppendCapsFlagText("　α付きテクスチャ : ", caps->TextureCaps, 0x4, p);
    p = AppendCapsFlagText("　テクスチャトランスフォーム : ", caps->TextureCaps, 0x400, p);
    p = AppendCapsFlagText("　バイリニア補間（拡大） : ", caps->TextureFilterCaps, 0x2000000, p);
    p = AppendCapsFlagText("　バイリニア補間（縮小） : ", caps->TextureFilterCaps, 0x200, p);
    p += sprintf(p, "--------------------------------------------\r\n");
}

void __stdcall ForceForegroundWindow(HWND hwnd)
{
#ifdef TH09_MODERN


    ShowWindowAsync(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
#else
    DWORD activeThread;
    DWORD thread;
    DWORD lockTime;

    activeThread = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    thread = GetWindowThreadProcessId(hwnd, NULL);
    AttachThreadInput(thread, activeThread, TRUE);
    SystemParametersInfoA(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &lockTime, 0);
    SystemParametersInfoA(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, NULL, 0);
    SetActiveWindow(hwnd);
    SystemParametersInfoA(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &lockTime, 0);
    AttachThreadInput(thread, activeThread, FALSE);
#endif
}

int ResolveShortcut(char *path, char *out, int size)
{
    HRESULT res;
    int ok;
    IShellLinkA *shellLink;
    IPersistFile *storage;
    WCHAR *buf;
    WIN32_FIND_DATAA findData;

    if (out == NULL)
    {
        return 0;
    }
    ok = 0;
    CoInitialize(NULL);
    res = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (void **)&shellLink);
    if (SUCCEEDED(res))
    {
        res = shellLink->QueryInterface(IID_IPersistFile, (void **)&storage);
        if (SUCCEEDED(res))
        {
            buf = new WCHAR[size];
            if (SUCCEEDED(res))
            {
                MultiByteToWideChar(CP_ACP, 0, path, -1, buf, size);
                res = storage->Load(buf, STGM_READ);
                if (SUCCEEDED(res))
                {
                    res = shellLink->GetPath(out, size, &findData, 0);
                    if (SUCCEEDED(res))
                    {
                        ok = 1;
                    }
                }
            }
            delete buf;
            storage->Release();
        }
        shellLink->Release();
    }
    CoUninitialize();
    return ok;
}

void Log::Reset()
{
    this->cursor = this->buffer;
    *this->cursor = '\0';
}

void Log::Flush()
{
    if (this->cursor != this->buffer)
    {
        LogPrintf(this, "---------------------------------------------------------- \r\n");
        if (this->hasError)
        {
            MessageBoxA(NULL, this->buffer, "log", MB_ICONERROR);
        }
        WriteToFile("./log.txt", this->buffer, strlen(this->buffer));
    }
}

GameWindow::GameWindow()
{
    memset(this, 0, sizeof(*this));
}

void SpriteEngine::ReleaseSurfaces()
{
    int i;

    for (i = 0; i < 0x20; i++)
    {
        if (this->surfaces[i] != NULL)
        {
            this->surfaces[i]->Release();
            this->surfaces[i] = NULL;
        }
    }
}

void SpriteEngine::DoPendingCopy()
{
    if (this->pendingTexCopyTexture >= 0)
    {
        this->CopyBackBufferToTexture(this->pendingTexCopyTexture, this->pendingTexCopySrcLeft,
                                      this->pendingTexCopySrcTop, this->pendingTexCopySrcWidth,
                                      this->pendingTexCopySrcHeight, this->pendingTexCopyDstLeft,
                                      this->pendingTexCopyDstTop, this->pendingTexCopyDstWidth,
                                      this->pendingTexCopyDstHeight);
        this->pendingTexCopyTexture = -1;
    }
    if (this->pendingSurfaceCopySurface >= 0)
    {
        this->CopyBackBufferToSurface(this->pendingSurfaceCopySurface, this->pendingSurfaceCopySrcLeft, this->pendingSurfaceCopySrcTop,
                                      this->pendingSurfaceCopySrcWidth, this->pendingSurfaceCopySrcHeight, this->pendingSurfaceCopyDstLeft,
                                      this->pendingSurfaceCopyDstTop, this->pendingSurfaceCopyDstWidth, this->pendingSurfaceCopyDstHeight);
        this->pendingSurfaceCopySurface = -1;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_ERASEBKGND:
        return 1;
    case WM_ACTIVATEAPP:
        g_GameWindow.isAppActive = wParam;
        g_GameWindow.isAppInactive = wParam == 0;
        break;
    case WM_CLOSE:
        g_App.stateFlags |= 0x80;
#ifdef TH09_MODERN
        g_GameWindow.isClosing = 1;
#endif
        return 1;
    case WM_SETCURSOR:
        if (!g_App.GetWindowed() && !g_GameWindow.isAppInactive)
        {
            ShowCursor(FALSE);
            SetCursor(NULL);
        }
        else
        {
            SetCursor(LoadCursorA(NULL, IDC_ARROW));
            ShowCursor(TRUE);
        }
        return 1;
    case MM_MOM_DONE:
        if (g_App.midi != NULL)
        {
            g_App.midi->ReleaseHeader((LPMIDIHDR)lParam);
        }
        break;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}



static int CreateGameWindow(HINSTANCE instance)
{
    WNDCLASSA wc;
    int width;
    int height;

    memset(&wc, 0, sizeof(wc));
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hInstance = instance;
    wc.lpfnWndProc = WindowProc;
    g_GameWindow.isAppActive = 1;
    g_GameWindow.isAppInactive = 0;
    wc.lpszClassName = "BASE";
    RegisterClassA(&wc);
    if (!g_App.GetWindowed())
    {
        width = 640;
        height = 480;
        g_GameWindow.hwnd =
            CreateWindowExA(0, "BASE", "東方花映塚　～ Phantasmagoria of Flower View. ver 1.50a", WS_OVERLAPPEDWINDOW,
                            0, 0, width, height, NULL, NULL, instance, NULL);
    }
    else
    {
        width = GetSystemMetrics(SM_CXDLGFRAME) * 2 + 640;
        height = GetSystemMetrics(SM_CYDLGFRAME) * 2 + 480 + GetSystemMetrics(SM_CYCAPTION);
        g_GameWindow.hwnd = CreateWindowExA(0, "BASE", "東方花映塚　～ Phantasmagoria of Flower View. ver 1.50a",
                                            WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT,
                                            width, height, NULL, NULL, instance, NULL);
    }
    g_App.hwnd = g_GameWindow.hwnd;
#ifdef TH09_NATIVE_NETPLAY
    SetWindowTextA(g_GameWindow.hwnd, "Touhou 09 - Co-op and Netplay");
#endif
    if (g_GameWindow.hwnd == NULL)
    {
        return 1;
    }
    ForceForegroundWindow(g_GameWindow.hwnd);
    return 0;
}

void SetDefaultRenderStates()
{
    float fogVal;

    g_App.d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    g_App.d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_App.d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    g_App.d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    g_App.d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    g_App.d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_App.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    g_App.d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    g_App.d3dDevice->SetRenderState(D3DRS_ALPHAREF, 4);
    g_App.d3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    if (g_App.GetNoFog() == 0)
    {
        g_App.d3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
    }
    else
    {
        g_App.d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    }
    fogVal = 1.0f;
    g_App.d3dDevice->SetRenderState(D3DRS_FOGDENSITY, *(DWORD *)&fogVal);
    g_App.d3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
    g_App.d3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
    g_App.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, 0xffa0a0a0);
    fogVal = 1000.0f;
    g_App.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fogVal);
    fogVal = 5000.0f;
    g_App.d3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)&fogVal);

    if (g_App.caps.RasterCaps | D3DPRASTERCAPS_ANTIALIASEDGES)
    {
        g_App.d3dDevice->SetRenderState(D3DRS_EDGEANTIALIAS, FALSE);
    }
    g_App.d3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
    if (g_SpriteEngine != NULL)
    {
        g_SpriteEngine->ClearBlendMode();
        g_SpriteEngine->ClearColorOp();
        g_SpriteEngine->ClearVertexFormat();
        g_SpriteEngine->ClearTexture();
        g_SpriteEngine->ClearLastCameraSetupFlag();
    }
    if (g_GameManager.sides[0].stage != NULL)
    {
        g_GameManager.sides[0].stage->fogColorChanged = 1;
    }
}

int CheckSingleInstance()
{
    char exePath[0x105];
    char consoleTitle[0x105];
    char *extension;

    SetLastError(ERROR_SUCCESS);
#ifdef TH09_NATIVE_NETPLAY
    g_AppMutex = CreateMutexA(NULL, FALSE, NULL);
#else
    g_AppMutex = CreateMutexA(NULL, TRUE, "Touhou 08 App");
#endif
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        LogError(&g_Logger, "二つは起動できません\r\n");
        return -1;
    }
    STARTUPINFOA startup = {sizeof(STARTUPINFOA)};
    GetModuleFileNameA(NULL, exePath, 0x105);
    GetConsoleTitleA(consoleTitle, 0x105);
    GetStartupInfoA(&startup);
    if (startup.lpTitle != NULL)
    {
        extension = strrchr(startup.lpTitle, '.');
        if (FileExists(startup.lpTitle) && extension != NULL)
        {
            if (_stricmp(extension, ".lnk") == 0)
            {
                do
                {
                    ResolveShortcut(startup.lpTitle, consoleTitle, 0x104);
                    extension = strrchr(consoleTitle, '.');
                } while (_stricmp(extension, ".lnk") == 0);
            }
            else
            {
                strcpy(consoleTitle, startup.lpTitle);
            }
            if (strcmp(exePath, consoleTitle) != 0)
            {
                g_GameWindow.forceClockPacing = 1;
            }
        }
        g_App.stateFlags &= ~0x40;
    }
    else
    {
        g_App.stateFlags |= 0x40;
    }
    if (g_AppMutex == NULL)
    {
        return -1;
    }
    return 0;
}

int ChecksumExecutable()
{
    char exePath[0x105];
    int sum;
    char *name;
    int *data;
    int idx;
    int *words;
    int fileSize;

    if (GetModuleFileNameA(NULL, exePath, sizeof(exePath)) != 0)
    {
        sum = 0;
        name = strrchr(exePath, '\\');
        if (name == NULL)
        {
            name = strrchr(exePath, '/');
        }
        if (name == NULL)
        {
            name = exePath;
        }
        else
        {
            name++;
        }
        data = words = (int *)LoadFileData(name, &fileSize, 1);
        if (words == NULL)
        {
            return -1;
        }
        for (idx = 0; idx < fileSize / 4 - 1; idx++, words++)
        {
            sum += *words;
        }
        g_MemoryTracker.Free(data);
#if FEAT_ORIG_CHECKSUM

        g_App.exeSize = TH09_ORIGINAL_EXE_SIZE;
        g_App.exeChecksum = TH09_ORIGINAL_EXE_CHECKSUM;
#else
        g_App.exeSize = fileSize;
        g_App.exeChecksum = sum;
#endif
        return sum;
    }
    return -1;
}

void PresentFrame()
{
    int i;
    char path[0x100];

    if (g_App.d3dDevice->Present(NULL, NULL, NULL, NULL) < 0)
    {
        g_SpriteEngine->ReleaseSurfaces();
        #ifdef TH09_NATIVE_NETPLAY
        ui::ReleaseFonts();
        #endif
        g_App.d3dDevice->Reset(&g_App.presentParams);
        SetDefaultRenderStates();
        g_App.redrawFrames = 2;
    }
    g_SpriteEngine->DoPendingCopy();
    if (g_PlayerInputs[2].IsJustPressed(0x800) && !(g_App.stateFlags >> 10 & 1))
    {
        _mkdir("snapshot");
        for (i = 0; i < 1000; i++)
        {
            sprintf(path, "snapshot/th%.3d.bmp", i);
            if (!FileExists(path))
            {
                break;
            }
        }
        if (i < 1000)
        {
            g_App.SaveScreenshot(path);
        }
    }
    if (g_App.redrawFrames != 0 && !g_GameManager.isPaused)
    {
        g_App.redrawFrames--;
    }
}

int GameWindow::RunFrame()
{
    int res;

    this->now = this->GetTime();

    if (this->lastTime > this->now)
    {
        this->nextFrameTime = this->now;
    }
    this->lastTime = this->now;
    if (this->nextFrameTime < this->now)
    {
        while (this->nextFrameTime < this->now)
        {
            this->nextFrameTime += 1.0 / 60.0;
        }
        g_SpriteEngine->FlushQuadBatch();
        g_App.SetView2D(2);
        res = g_Chain.RunCalcChain();
        g_SoundPlayer.PlayQueuedSounds();
        if (res == 0)
        {
            g_App.CloseSubThread();
            return 1;
        }
        if (res == -1)
        {
            g_App.CloseSubThread();
            return 2;
        }
        this->curFrame++;
        if (g_App.config.frameSkip < this->curFrame)
        {
            g_App.d3dDevice->BeginScene();
            g_SpriteEngine->BeginQuadBatch();
            g_App.fogState = 0xff;
            g_App.DisableFog();
            g_Chain.RunDrawChain();
            g_SpriteEngine->FlushQuadBatch();
            g_App.d3dDevice->SetTexture(0, NULL);
            g_App.d3dDevice->EndScene();
            this->curFrame = 0;
        }
        this->now = this->GetTime();
        PresentFrame();
    }
    else
    {
        Sleep(0);
    }
    return 0;
}

int InitD3dDevice()
{
    char buffer[0x2000];
    D3DDISPLAYMODE displayMode;
    D3DPRESENT_PARAMETERS params;
    bool checkFormat = true;
    int retried;

    memset(&params, 0, sizeof(params));
    g_App.d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);
    if (!g_App.config.windowed)
    {
        if (g_App.GetForce16BitTextures() == 1)
        {
            params.BackBufferFormat = D3DFMT_R5G6B5;
            g_App.config.colorMode16Bits = 1;
        }
        else if (g_App.config.colorMode16Bits == 0xff)
        {
            params.BackBufferFormat = D3DFMT_X8R8G8B8;
            g_App.config.colorMode16Bits = 0;
            LogPrintf(&g_Logger, "初回起動、画面を 32Bits で初期化しました\r\n");
        }
        else if (g_App.config.colorMode16Bits == 0)
        {
            params.BackBufferFormat = D3DFMT_X8R8G8B8;
        }
        else
        {
            params.BackBufferFormat = D3DFMT_R5G6B5;
        }
        if (g_GameWindow.forceClockPacing)
        {
            g_App.noVsync = 1;
        }
        if (g_App.noVsync == 0)
        {
            params.FullScreen_RefreshRateInHz = 60;
            params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
            LogPrintf(&g_Logger, "リフレッシュレートを60Hzに変更を試みます\r\n");
            if (g_App.config.frameSkip == 0)
            {
                params.SwapEffect = D3DSWAPEFFECT_FLIP;
            }
            else
            {
                params.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
            }
        }
        else
        {
            params.FullScreen_RefreshRateInHz = 0;
            params.SwapEffect = D3DSWAPEFFECT_COPY;
            params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
            LogPrintf(&g_Logger, "VSync非同期可能かどうかを試みます\r\n");
        }
    }
    else
    {
        params.BackBufferFormat = displayMode.Format;
        params.SwapEffect = D3DSWAPEFFECT_COPY;
        params.Windowed = TRUE;
    }
    params.BackBufferWidth = 640;
    params.BackBufferHeight = 480;
    params.EnableAutoDepthStencil = TRUE;
    params.AutoDepthStencilFormat = D3DFMT_D16;
    params.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    g_App.stateFlags |= 2;
    g_App.wants60Hz = 1;
    retried = 0;
retry:
    if (g_App.GetReferenceRasterizer())
    {
        goto reference;
    }
    if (g_App.d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_GameWindow.hwnd,
                                D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &g_App.d3dDevice) < 0)
    {
        if (retried)
        {
            LogPrintf(&g_Logger, "T&L HAL は使用できないようです\r\n");
        }
        if (g_App.d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_GameWindow.hwnd,
                                    D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &g_App.d3dDevice) < 0)
        {
            if (retried)
            {
                LogPrintf(&g_Logger, "HAL も使用できないようです\r\n");
            }
        reference:
            if (g_App.d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, g_GameWindow.hwnd,
                                        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &g_App.d3dDevice) < 0)
            {
                if (g_App.noVsync == 0)
                {
                    LogPrintf(&g_Logger, "リフレッシュレートが変更できません\r\n");
                    params.FullScreen_RefreshRateInHz = 0;
                    g_App.wants60Hz = 0;
                    retried = 1;
                    goto retry;
                }
                if (params.FullScreen_PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE)
                {
                    LogPrintf(&g_Logger, "非同期更新も行えません。一番汚いモードに変更します\r\n");
                    LogError(&g_Logger, "*** リフレッシュレートを60Hzに変更することを推奨します ***\r\n");
                    params.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
                    params.SwapEffect = D3DSWAPEFFECT_COPY;
                    goto retry;
                }
                LogError(&g_Logger, "Direct3D の初期化に失敗、これではゲームは出来ません\r\n");
                if (g_App.d3d != NULL)
                {
                    g_App.d3d->Release();
                    g_App.d3d = NULL;
                }
                return 1;
            }
            else
            {
                LogPrintf(&g_Logger, "REF で動作しますが、重すぎて恐らくゲームになりません...\r\n");
                g_App.stateFlags &= ~1;
                checkFormat = false;
            }
        }
        else
        {
            LogPrintf(&g_Logger, "HAL で動作します\r\n");
            g_App.stateFlags &= ~1;
        }
    }
    else
    {
        LogPrintf(&g_Logger, "T&L HAL で動作しま～す\r\n");
        g_App.stateFlags |= 1;
    }
    memcpy(&g_App.presentParams, &params, sizeof(params));
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
    g_App.SetView2D(2);
    g_App.d3dDevice->GetDeviceCaps(&g_App.caps);
    if (g_App.caps.MaxTextureWidth <= 256)
    {
        LogPrintf(&g_Logger, "512 以上のテクスチャをサポートしていません。殆どの絵がボケて表示されます。\r\n");
    }
    FormatCapsReport(&g_App.caps, buffer);
    LogPrintf(&g_Logger, buffer);
    if (g_App.GetForce16BitTextures() == 0 && checkFormat)
    {
        if (g_App.d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, params.BackBufferFormat, 0,
                                         D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8) == D3D_OK)
        {
            g_App.stateFlags |= 4;
        }
        else
        {
            g_App.stateFlags &= ~4;
            g_App.config.flags.force16BitTextures = 1;
            LogPrintf(&g_Logger, "D3DFMT_A8R8G8B8 をサポートしていません、減色モードで動作します\r\n");
        }
    }
    SetDefaultRenderStates();
    ClearViewport(0xff000000);
    g_GameWindow.isClosing = 0;
    g_App.clearedOnD3dInit = 0;
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int runResult = 0;
    MSG message;
    HRESULT testResult;
    int j;
    int i;


    if (GetModuleFileNameA(NULL, g_DataDir, 0x104) != 0)
    {
        for (i = strlen(g_DataDir) - 1; g_DataDir[i] != '\\' && g_DataDir[i] != '/'; i--)
        {
            if (i <= 0)
            {
                break;
            }
        }
        if (i <= 0)
        {
            strcpy(g_DataDir, "./");
        }
        else
        {
            g_DataDir[i] = '/';
            g_DataDir[i + 1] = '\0';
        }
    }
    else
    {
        strcpy(g_DataDir, "./");
    }
    g_App.hInstance = hInstance;
    SystemParametersInfoA(SPI_GETSCREENSAVEACTIVE, 0, &g_GameWindow.screenSaveActive, 0);
    SystemParametersInfoA(SPI_GETLOWPOWERACTIVE, 0, &g_GameWindow.lowPowerActive, 0);
    SystemParametersInfoA(SPI_GETPOWEROFFACTIVE, 0, &g_GameWindow.powerOffActive, 0);
    SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, 0, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETLOWPOWERACTIVE, 0, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETPOWEROFFACTIVE, 0, NULL, SPIF_SENDCHANGE);
    g_App.InitCriticalSections();
    LogPrintf(&g_Logger, "東方動作記録 --------------------------------------------- \r\n");
    if (CheckSingleInstance() == -1)
    {
        goto stop;
    }
    if (g_App.LoadConfig("th09.cfg") != 0)
    {
        goto stop;
    }
    ChecksumExecutable();
    QueryPerformanceFrequency((LARGE_INTEGER *)g_GameWindow.perfFrequency);
restart:
    if (CreateD3d() != 0)
    {
        goto stop;
    }
    if (CreateGameWindow(hInstance) != 0)
    {
        goto stop;
    }
    if (InitD3dDevice() != 0)
    {
        goto stop;
    }
    g_SoundPlayer.Initialize(g_GameWindow.hwnd);
    InitJoystick();
    ResetKeyboard();
    g_SpriteEngine =
        (SpriteEngine *)g_MemoryTracker.Register(new SpriteEngine(), sizeof(SpriteEngine), "SprtCtrlInf");
    g_Rng.SetSeed(timeGetTime());
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    g_Network = (Network *)g_MemoryTracker.Register(new Network(), sizeof(Network), "NetworkInf");
#ifndef TH09_NATIVE_NETPLAY
    g_Network->Init();
    if (g_Network->CreatePeer() != 0)
    {
        LogError(&g_Logger, "DirectPlay 初期化失敗\r\n");
    }
#endif
    if (!g_App.GetWindowed())
    {
        WINNLSEnableIME(NULL, FALSE);
        ShowCursor(FALSE);
        SetCursor(NULL);
    }
    runResult = App::RegisterChain();
    if (runResult != 0)
    {
        if (runResult == -1)
        {
            goto release;
        }
        runResult = 2;
        goto release;
    }
    runResult = 0;
    g_GameWindow.curFrame = -4;
    g_GameWindow.now = g_GameWindow.lastTime = g_GameWindow.nextFrameTime = 0;
    while (!g_GameWindow.isClosing)
    {
        if (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        else
        {
            testResult = g_App.d3dDevice->TestCooperativeLevel();
            if (testResult == D3D_OK)
            {
                runResult = g_GameWindow.RunFrame();
                if (runResult != 0)
                {
                    break;
                }
                g_App.stateFlags &= ~0x10;
            }
            else if (testResult == D3DERR_DEVICENOTRESET)
            {
                g_SpriteEngine->ReleaseSurfaces();
                #ifdef TH09_NATIVE_NETPLAY
                ui::ReleaseFonts();
                #endif
                if (g_App.d3dDevice->Reset(&g_App.presentParams) != 0)
                {
                    break;
                }
                SetDefaultRenderStates();
                g_App.redrawFrames = 3;
                g_App.stateFlags |= 0x10;
            }
        }
    }
release:
    g_Chain.Release();
    while (g_SoundPlayer.PlayQueuedSounds())
    {
    }
stop:
    Sleep(1000);
    g_SoundPlayer.Release();
    delete g_SpriteEngine;
    g_SpriteEngine = NULL;
    if (g_App.d3dDevice != NULL)
    {
        #ifdef TH09_NATIVE_NETPLAY
        ui::ReleaseFonts();
        #endif
        g_App.d3dDevice->Reset(&g_App.presentParams);
    }
    if (g_App.d3dDevice != NULL)
    {
        g_App.d3dDevice->Release();
        g_App.d3dDevice = NULL;
    }
    if (g_App.d3d != NULL)
    {
        g_App.d3d->Release();
        g_App.d3d = NULL;
    }
    if (g_Network != NULL)
    {
        g_Network->Close();
        g_Network->ReleaseInterfaces();
        delete g_Network;
        g_Network = NULL;
    }
    if (g_GameWindow.hwnd != NULL)
    {
        ShowWindow(g_GameWindow.hwnd, SW_HIDE);
        MoveWindow(g_GameWindow.hwnd, 0, 0, 0, 0, FALSE);
        DestroyWindow(g_GameWindow.hwnd);
        g_GameWindow.hwnd = NULL;
    }
    ShowCursor(TRUE);
    if (runResult == 2)
    {
        g_Logger.Reset();
        LogPrintf(&g_Logger, "再起動を要するオプションが変更されたので再起動します\r\n");
        if (!g_App.GetWindowed())
        {
            WINNLSEnableIME(NULL, TRUE);
        }
        j = 0;
        while (j < 60)
        {
            if (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }
            j++;
        }
        goto restart;
    }
    WriteToFile("th09.cfg", &g_App.config, sizeof(g_App.config));
    if (g_App.midi != NULL)
    {
        g_App.midi->Stop();
        delete g_App.midi;
        g_App.midi = NULL;
    }
    g_Logger.Flush();
    g_App.DeleteCriticalSections();
    CoUninitialize();
    SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, g_GameWindow.screenSaveActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETLOWPOWERACTIVE, g_GameWindow.lowPowerActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETPOWEROFFACTIVE, g_GameWindow.powerOffActive, NULL, SPIF_SENDCHANGE);
    WINNLSEnableIME(NULL, TRUE);
    return 0;
}
