#include "unsorted.h"

#include "archive.h"
#include "gamemanager.h"
#include "gui.h"
#include "player.h"
#include "chain.h"
#include "timer.h"

float Fmod(float x, float y)
{
    return (float)fmod((double)x, (double)y);
}

ChainElem::ChainElem()
{
    this->prev = NULL;
    this->next = NULL;
    this->callback = NULL;
    this->self = this;
    this->addedCallback = NULL;
    this->deletedCallback = NULL;
    this->priority = 0;
    this->allocated = 0;
}

ChainElem::~ChainElem()
{
    if (this->deletedCallback != NULL)
    {
        this->deletedCallback(this->arg);
    }
    this->prev = NULL;
    this->next = NULL;
    this->callback = NULL;
    this->addedCallback = NULL;
    this->deletedCallback = NULL;
}

Chain::Chain()
{
}

static int SetButtonFromDirectInput(WORD *buttons, short button, WORD bit, BYTE *rgbButtons)
{
    if (button < 0)
    {
        return 0;
    }
    *buttons |= (rgbButtons[button] & 0x80) ? bit : 0;
    return (rgbButtons[button] & 0x80) ? bit : 0;
}

static int SetButtonFromJoystick(WORD *buttons, short button, WORD bit, DWORD joyButtons)
{
    if (button < 0)
    {
        return 0;
    }
    DWORD mask = 1 << button;
    *buttons |= (joyButtons & mask) ? bit : 0;
    return (joyButtons & mask) ? bit : 0;
}

WORD UpdatePlayerInput(int player, WORD pressed)
{
    PlayerInput *input;
    int i;
    WORD *held;
    WORD bit;
    WORD buttons;

    input = &g_PlayerInputs[player];
    input->unk_2 = input->unk_0;
    input->unk_0 = (WORD)pressed;
    bit = 1;
    buttons = (WORD)pressed;
    input->unk_4 = 0;
    held = input->heldFrames;
    for (i = 0; i < 16; i++, held++, bit <<= 1)
    {
        if (buttons & 1)
        {
            (*held)++;
            if (*held >= 26)
            {
                input->unk_4 |= bit;
                *held -= 8;
            }
        }
        else
        {
            *held = 0;
        }
        buttons >>= 1;
    }
    input->unk_6 = (input->unk_0 ^ input->unk_2) & input->unk_0;
    input->unk_8 = ~input->unk_0 & (input->unk_0 ^ input->unk_2);
    return pressed;
}

void ResetKeyboard()
{
    BYTE keyState[0x100];
    int i;

    GetKeyboardState(keyState);
    for (i = 0; i < 0x100; i++)
    {
        keyState[i] &= 0x7f;
    }
    SetKeyboardState(keyState);
}

char *__stdcall GetDataPath(char *name)
{
    strcpy(g_PathBuffer, g_DataDir);
    strcat(g_PathBuffer, name);
    return g_PathBuffer;
}

WORD Rng::GetRandomU16()
{
    WORD a;

    a = (this->seed ^ 0x9630) - 0x6553;
    this->seed = (((a & 0xc000) >> 14) + a * 4) & 0xffff;
    this->generated++;
    return this->seed;
}

DWORD Rng::GetRandomU32()
{
    return (this->GetRandomU16() << 16) | this->GetRandomU16();
}

float Rng::GetRandomF32ZeroToOne()
{
    return this->GetRandomU32() / 4294967296.0f;
}

float Rng::GetRandomF32MinusOneToOne()
{
    return this->GetRandomU32() / 2147483648.0f - 1.0f;
}

float AddNormalizeAngle(float a, float b)
{
    int i = 0;

    a += b;
    while (a > 3.1415927f)
    {
        a -= 6.2831855f;
        if (i++ > 16)
        {
            break;
        }
    }
    while (a < -3.1415927f)
    {
        a += 6.2831855f;
        if (i++ > 16)
        {
            break;
        }
    }
    return a;
}

void Rotate2D(D3DXVECTOR3 *out, D3DXVECTOR3 *in, float angle)
{
    float s = sinf(angle);
    float c = cosf(angle);

    out->x = c * in->x - s * in->y;
    out->y = c * in->y + s * in->x;
}

void HermiteInterpolate(D3DXVECTOR3 *out, D3DXVECTOR3 *p1, D3DXVECTOR3 *p0, D3DXVECTOR3 *m1, D3DXVECTOR3 *m0,
                        float time, float duration)
{
    float t;
    float f00;
    float f01;
    float f10;
    float f11;

    t = time / duration;
    f00 = (2.0f * t + 1.0f) * (t - 1.0f) * (t - 1.0f);
    f01 = (3.0f - 2.0f * t) * t * t;
    f10 = (1.0f - t) * (1.0f - t) * t;
    f11 = (t - 1.0f) * t * t;
    out->x = f00 * p0->x + f01 * p1->x + f10 * m0->x + f11 * m1->x;
    out->y = f00 * p0->y + f01 * p1->y + f10 * m0->y + f11 * m1->y;
}

PlayerInput::PlayerInput()
{
    this->mappings[0].shootButton = 0;
    this->mappings[0].bombButton = 1;
    this->mappings[0].focusButton = 2;
    this->mappings[0].menuButton = 4;
    this->mappings[0].upButton = -1;
    this->mappings[0].downButton = -1;
    this->mappings[0].leftButton = -1;
    this->mappings[0].rightButton = -1;
    this->mappings[0].skipButton = 3;
    this->mappings[1].shootButton = 0x5a;
    this->mappings[1].bombButton = 0x58;
    this->mappings[1].focusButton = 0x10;
    this->mappings[1].menuButton = 0x1b;
    this->mappings[1].upButton = 0x26;
    this->mappings[1].downButton = 0x28;
    this->mappings[1].leftButton = 0x25;
    this->mappings[1].rightButton = 0x27;
    this->mappings[1].skipButton = 0x11;
    this->mappings[2].shootButton = 0x2c;
    this->mappings[2].bombButton = 0x2d;
    this->mappings[2].focusButton = 0x2a;
    this->mappings[2].menuButton = 1;
    this->mappings[2].upButton = 0xc8;
    this->mappings[2].downButton = 0xd0;
    this->mappings[2].leftButton = 0xcb;
    this->mappings[2].rightButton = 0xcd;
    this->mappings[2].skipButton = 0x1d;
}

MemoryManager::MemoryManager()
{
    this->freeBlocksOnExit = 0;
}

MemoryManager::~MemoryManager()
{
    int i;

    if (this->freeBlocksOnExit != 0)
    {
        for (i = 0; i < 0x1000; i++)
        {
            if (this->blocks[i] != NULL)
            {
                free(this->blocks[i]);
            }
        }
    }
}

Log::Log()
{
    this->cursor = this->buffer;
    *this->cursor = '\0';
    this->hasError = 0;
}

void App::EnterCriticalSection(int idx)
{
    ::EnterCriticalSection(&this->criticalSections[idx]);
    this->criticalSectionDepths[idx]++;
}

void App::LeaveCriticalSection(int idx)
{
    ::LeaveCriticalSection(&this->criticalSections[idx]);
    this->criticalSectionDepths[idx]--;
}

Chain::~Chain()
{
}

int Chain::AddToCalcChain(ChainElem *elem, int priority)
{
    ChainElem *cur = &this->calcChain;
    int result = 0;

    if (elem->addedCallback != NULL)
    {
        result = elem->addedCallback(elem->arg);
        elem->addedCallback = NULL;
    }
    g_App.EnterCriticalSection(0);
    elem->priority = priority;
    while (cur->next != NULL)
    {
        if (cur->priority > priority)
        {
            break;
        }
        cur = cur->next;
    }
    if (cur->priority > priority)
    {
        elem->next = cur;
        elem->prev = cur->prev;
        if (elem->prev != NULL)
        {
            elem->prev->next = elem;
        }
        cur->prev = elem;
    }
    else
    {
        elem->next = NULL;
        elem->prev = cur;
        cur->next = elem;
    }
    g_App.LeaveCriticalSection(0);
    return result;
}

int Chain::AddToDrawChain(ChainElem *elem, int priority)
{
    ChainElem *cur = &this->drawChain;
    int result = 0;

    if (elem->addedCallback != NULL)
    {
        result = elem->addedCallback(elem->arg);
        elem->addedCallback = NULL;
    }
    g_App.EnterCriticalSection(0);
    elem->priority = priority;
    while (cur->next != NULL)
    {
        if (cur->priority > priority)
        {
            break;
        }
        cur = cur->next;
    }
    if (cur->priority > priority)
    {
        elem->next = cur;
        elem->prev = cur->prev;
        if (elem->prev != NULL)
        {
            elem->prev->next = elem;
        }
        cur->prev = elem;
    }
    else
    {
        elem->next = NULL;
        elem->prev = cur;
        cur->next = elem;
    }
    g_App.LeaveCriticalSection(0);
    return result;
}

ChainElem *Chain::CreateElem(ChainCallback callback)
{
    ChainElem *elem = (ChainElem *)g_MemoryTracker.Register(new ChainElem(), sizeof(ChainElem), "funcChainInf");
    elem->SetCallback(callback);
    elem->allocated = 1;
    return elem;
}

void Chain::RemoveLocked(ChainElem *elem)
{
    ChainElem *cur;
    ChainCallback callback;

    if (elem == NULL)
    {
        return;
    }
    cur = &this->calcChain;
    while (cur != NULL)
    {
        if (cur == elem)
        {
            goto found;
        }
        cur = cur->next;
    }
    cur = &this->drawChain;
    while (cur != NULL)
    {
        if (cur == elem)
        {
            goto found;
        }
        cur = cur->next;
    }
    return;

found:
    if (elem->prev != NULL)
    {
        elem->callback = NULL;
        elem->prev->next = elem->next;
        if (elem->next != NULL)
        {
            elem->next->prev = elem->prev;
        }
        elem->prev = NULL;
        elem->next = NULL;
        if (elem->allocated)
        {
            g_App.LeaveCriticalSection(0);
            delete elem;
            elem = NULL;
            g_App.EnterCriticalSection(0);
        }
        else if (elem->deletedCallback != NULL)
        {
            callback = elem->deletedCallback;
            elem->deletedCallback = NULL;
            g_App.LeaveCriticalSection(0);
            callback(elem->arg);
            g_App.EnterCriticalSection(0);
        }
    }
}

WORD ReadJoystick(int input, int pad, WORD buttons)
{
    JOYINFOEX joyInfo;
    DWORD margin;
    HRESULT hr;
    DIJOYSTATE2 state;
    PlayerInput *playerInput;
    int retries;

    playerInput = &g_PlayerInputs[input];
    if (g_App.joystick == NULL || g_App.joystick2 == NULL)
    {
        memset(&joyInfo, 0, sizeof(joyInfo));
        joyInfo.dwSize = sizeof(joyInfo);
        joyInfo.dwFlags = JOY_RETURNALL;
        if (joyGetPosEx(pad != 0, &joyInfo) != JOYERR_NOERROR)
        {
            return buttons;
        }
        SetButtonFromJoystick(&buttons, playerInput->mappings[0].shootButton, 1, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, playerInput->mappings[0].bombButton, 2, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, playerInput->mappings[0].focusButton, 4, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, playerInput->mappings[0].menuButton, 8, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, playerInput->mappings[0].upButton, 0x10, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, playerInput->mappings[0].downButton, 0x20, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, playerInput->mappings[0].leftButton, 0x40, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, playerInput->mappings[0].rightButton, 0x80, joyInfo.dwButtons);
        SetButtonFromJoystick(&buttons, playerInput->mappings[0].skipButton, 0x100, joyInfo.dwButtons);
        margin = (g_JoyDeviceCaps[pad].wXmax - g_JoyDeviceCaps[pad].wXmin) / 2 / 2;
        buttons |= ((g_JoyDeviceCaps[pad].wXmin + g_JoyDeviceCaps[pad].wXmax) / 2 + margin < joyInfo.dwXpos) ? 0x80 : 0;
        buttons |= (joyInfo.dwXpos < (g_JoyDeviceCaps[pad].wXmin + g_JoyDeviceCaps[pad].wXmax) / 2 - margin) ? 0x40 : 0;
        margin = (g_JoyDeviceCaps[pad].wYmax - g_JoyDeviceCaps[pad].wYmin) / 2 / 2;
        buttons |= ((g_JoyDeviceCaps[pad].wYmin + g_JoyDeviceCaps[pad].wYmax) / 2 + margin < joyInfo.dwYpos) ? 0x20 : 0;
        buttons |= (joyInfo.dwYpos < (g_JoyDeviceCaps[pad].wYmin + g_JoyDeviceCaps[pad].wYmax) / 2 - margin) ? 0x10 : 0;
        return buttons;
    }
    hr = g_App.joysticks[pad]->Poll();
    if (FAILED(hr))
    {
        retries = 0;
        hr = g_App.joysticks[pad]->Acquire();
        while (hr == DIERR_INPUTLOST)
        {
            hr = g_App.joysticks[pad]->Acquire();
            retries++;
            if (retries >= 400)
            {
                return buttons;
            }
        }
        return buttons;
    }
    memset(&state, 0, sizeof(state));
    hr = g_App.joysticks[pad]->GetDeviceState(sizeof(DIJOYSTATE2), &state);
    if (FAILED(hr))
    {
        return buttons;
    }
    SetButtonFromDirectInput(&buttons, playerInput->mappings[0].shootButton, 1, state.rgbButtons);
    SetButtonFromDirectInput(&buttons, playerInput->mappings[0].bombButton, 2, state.rgbButtons);
    SetButtonFromDirectInput(&buttons, playerInput->mappings[0].focusButton, 4, state.rgbButtons);
    SetButtonFromDirectInput(&buttons, playerInput->mappings[0].menuButton, 8, state.rgbButtons);
    SetButtonFromDirectInput(&buttons, playerInput->mappings[0].upButton, 0x10, state.rgbButtons);
    SetButtonFromDirectInput(&buttons, playerInput->mappings[0].downButton, 0x20, state.rgbButtons);
    SetButtonFromDirectInput(&buttons, playerInput->mappings[0].leftButton, 0x40, state.rgbButtons);
    SetButtonFromDirectInput(&buttons, playerInput->mappings[0].rightButton, 0x80, state.rgbButtons);
    SetButtonFromDirectInput(&buttons, playerInput->mappings[0].skipButton, 0x100, state.rgbButtons);
    buttons |= (state.lX > g_App.config.padThresholdX) ? 0x80 : 0;
    buttons |= (state.lX < -g_App.config.padThresholdX) ? 0x40 : 0;
    buttons |= (state.lY > g_App.config.padThresholdY) ? 0x20 : 0;
    buttons |= (state.lY < -g_App.config.padThresholdY) ? 0x10 : 0;
    return buttons;
}

BYTE *GetPadButtons(int player)
{
    JOYINFOEX joyInfo;
    DWORD mask;
    DWORD button;
    HRESULT hr;
    DIJOYSTATE2 state;
    int retries;

    memset(g_PadButtons, 0, sizeof(g_PadButtons));
    if (g_App.joysticks[player] == NULL)
    {
        memset(&joyInfo, 0, sizeof(joyInfo));
        joyInfo.dwSize = sizeof(joyInfo);
        joyInfo.dwFlags = JOY_RETURNALL;
        if (joyGetPosEx(JOYSTICKID1, &joyInfo) != JOYERR_NOERROR)
        {
            return g_PadButtons;
        }
        mask = joyInfo.dwButtons;
        for (button = 0; button < 32; button++, mask >>= 1)
        {
            if (mask & 1)
            {
                g_PadButtons[button] = 0x80;
            }
        }
        return g_PadButtons;
    }
    hr = g_App.joysticks[player]->Poll();
    if (FAILED(hr))
    {
        retries = 0;
        hr = g_App.joysticks[player]->Acquire();
        while (hr == DIERR_INPUTLOST)
        {
            hr = g_App.joysticks[player]->Acquire();
            retries++;
            if (retries >= 400)
            {
                return g_PadButtons;
            }
        }
        return g_PadButtons;
    }
    g_App.joysticks[player]->GetDeviceState(sizeof(DIJOYSTATE2), &state);
    memcpy(g_PadButtons, state.rgbButtons, sizeof(g_PadButtons));
    return g_PadButtons;
}


#define KEY(key, bit) buttons |= ((keyState[key] & 0x80) ? (bit) : 0)

WORD ReadButtons(int player)
{
    WORD buttons = 0;
    BYTE keyState[0x100];
    HRESULT hr;
    PlayerInput *input;
    int i;
    WORD *held;
    WORD bit;
    WORD pressed;

    input = &g_PlayerInputs[player];
    if (g_GameWindow.isAppActive)
    {
        if (g_App.keyboard == NULL)
        {
            GetKeyboardState(keyState);
            if (player == 2)
            {
                goto layout2;
            }
            switch (g_App.config.pads[player])
            {
            case 2:
            layout2:
                KEY(VK_UP, 0x10);
                KEY(VK_DOWN, 0x20);
                KEY(VK_LEFT, 0x40);
                KEY(VK_RIGHT, 0x80);
                KEY(VK_NUMPAD8, 0x10);
                KEY(VK_NUMPAD2, 0x20);
                KEY(VK_NUMPAD4, 0x40);
                KEY(VK_NUMPAD6, 0x80);
                KEY(VK_NUMPAD7, 0x50);
                KEY(VK_NUMPAD9, 0x90);
                KEY(VK_NUMPAD1, 0x60);
                KEY(VK_NUMPAD3, 0xa0);
                KEY('Z', 0x1);
                KEY('X', 0x2);
                KEY(VK_SHIFT, 0x4);
                break;
            case 3:
                KEY('T', 0x10);
                KEY('B', 0x20);
                KEY('F', 0x40);
                KEY('H', 0x80);
                KEY('R', 0x50);
                KEY('Y', 0x90);
                KEY('V', 0x60);
                KEY('N', 0xa0);
                KEY('Z', 0x1);
                KEY('X', 0x2);
                KEY(VK_SHIFT, 0x4);
                break;
            case 4:
                KEY(VK_NUMPAD8, 0x10);
                KEY(VK_NUMPAD2, 0x20);
                KEY(VK_NUMPAD4, 0x40);
                KEY(VK_NUMPAD6, 0x80);
                KEY(VK_NUMPAD7, 0x50);
                KEY(VK_NUMPAD9, 0x90);
                KEY(VK_NUMPAD1, 0x60);
                KEY(VK_NUMPAD3, 0xa0);
                KEY(VK_DOWN, 0x1);
                KEY(VK_RIGHT, 0x2);
                KEY(VK_LEFT, 0x4);
                break;
            }
            KEY(VK_HOME, 0x800);
            KEY('P', 0x800);
            KEY('D', 0x2000);
            KEY(VK_ESCAPE, 0x8);
            KEY(VK_CONTROL, 0x100);
            KEY('Q', 0x200);
            KEY('S', 0x400);
            KEY('R', 0x4000);
            KEY(VK_RETURN, 0x1000);
        }
        else
        {
            hr = g_App.keyboard->GetDeviceState(sizeof(keyState), keyState);
            buttons = 0;
            if (hr == DIERR_INPUTLOST)
            {
                g_App.keyboard->Acquire();
            }
            else if (hr != DI_OK)
            {
                g_App.keyboard->Acquire();
            }
            else
            {
                if (player == 2)
                {
                    goto dikLayout2;
                }
                switch (g_App.config.pads[player])
                {
                case 2:
                dikLayout2:
                    KEY(DIK_UP, 0x10);
                    KEY(DIK_DOWN, 0x20);
                    KEY(DIK_LEFT, 0x40);
                    KEY(DIK_RIGHT, 0x80);
                    KEY(DIK_NUMPAD8, 0x10);
                    KEY(DIK_NUMPAD2, 0x20);
                    KEY(DIK_NUMPAD4, 0x40);
                    KEY(DIK_NUMPAD6, 0x80);
                    KEY(DIK_NUMPAD7, 0x50);
                    KEY(DIK_NUMPAD9, 0x90);
                    KEY(DIK_NUMPAD1, 0x60);
                    KEY(DIK_NUMPAD3, 0xa0);
                    KEY(DIK_Z, 0x1);
                    KEY(DIK_X, 0x2);
                    KEY(DIK_LSHIFT, 0x4);
                    break;
                case 3:
                    KEY(DIK_T, 0x10);
                    KEY(DIK_B, 0x20);
                    KEY(DIK_F, 0x40);
                    KEY(DIK_H, 0x80);
                    KEY(DIK_R, 0x50);
                    KEY(DIK_Y, 0x90);
                    KEY(DIK_V, 0x60);
                    KEY(DIK_N, 0xa0);
                    KEY(DIK_Z, 0x1);
                    KEY(DIK_X, 0x2);
                    KEY(DIK_LSHIFT, 0x4);
                    break;
                case 4:
                    KEY(DIK_NUMPAD8, 0x10);
                    KEY(DIK_NUMPAD2, 0x20);
                    KEY(DIK_NUMPAD4, 0x40);
                    KEY(DIK_NUMPAD6, 0x80);
                    KEY(DIK_NUMPAD7, 0x50);
                    KEY(DIK_NUMPAD9, 0x90);
                    KEY(DIK_NUMPAD1, 0x60);
                    KEY(DIK_NUMPAD3, 0xa0);
                    KEY(DIK_DOWN, 0x1);
                    KEY(DIK_RIGHT, 0x2);
                    KEY(DIK_LEFT, 0x4);
                    break;
                }
                KEY(DIK_HOME, 0x800);
                KEY(DIK_P, 0x800);
                KEY(DIK_D, 0x2000);
                KEY(DIK_ESCAPE, 0x8);
                KEY(DIK_LCONTROL, 0x100);
                KEY(DIK_Q, 0x200);
                KEY(DIK_S, 0x400);
                KEY(DIK_RETURN, 0x1000);
                KEY(DIK_R, 0x4000);
            }
        }
    }
    if (player >= 2)
    {
        if (g_App.config.pads[0] == 0 || g_App.config.pads[0] == 1)
        {
            buttons = ReadJoystick(0, g_App.config.pads[0] != 0, buttons);
        }
        if (g_App.config.pads[1] == 0 || g_App.config.pads[1] == 1)
        {
            buttons = ReadJoystick(1, g_App.config.pads[1] != 0, buttons);
        }
    }
    else if (g_App.config.pads[player] == 0 || g_App.config.pads[player] == 1)
    {
        buttons = ReadJoystick(player, g_App.config.pads[player] != 0, buttons);
    }
    input->unk_2 = input->unk_0;
    input->unk_0 = buttons;
    bit = 1;
    pressed = buttons;
    input->unk_4 = 0;
    held = input->heldFrames;
    for (i = 0; i < 16; i++, held++, bit <<= 1)
    {
        if (pressed & 1)
        {
            (*held)++;
            if (*held >= 26)
            {
                input->unk_4 |= bit;
                *held -= 8;
            }
        }
        else
        {
            *held = 0;
        }
        pressed >>= 1;
    }
    input->unk_6 = (input->unk_0 ^ input->unk_2) & input->unk_0;
    input->unk_8 = ~input->unk_0 & (input->unk_0 ^ input->unk_2);
    return buttons;
}

WORD ReadNetInput()
{
    WORD buttons = 0;
    BYTE keyState[0x100];
    HRESULT hr;

    if (g_GameWindow.isAppActive)
    {
        if (g_App.keyboard == NULL)
        {
            GetKeyboardState(keyState);
            KEY(VK_UP, 0x10);
            KEY(VK_DOWN, 0x20);
            KEY(VK_LEFT, 0x40);
            KEY(VK_RIGHT, 0x80);
            KEY(VK_NUMPAD8, 0x10);
            KEY(VK_NUMPAD2, 0x20);
            KEY(VK_NUMPAD4, 0x40);
            KEY(VK_NUMPAD6, 0x80);
            KEY(VK_NUMPAD7, 0x50);
            KEY(VK_NUMPAD9, 0x90);
            KEY(VK_NUMPAD1, 0x60);
            KEY(VK_NUMPAD3, 0xa0);
            KEY(VK_HOME, 0x800);
            KEY('P', 0x800);
            KEY('D', 0x2000);
            KEY('Z', 0x1);
            KEY('X', 0x2);
            KEY(VK_SHIFT, 0x4);
            KEY(VK_ESCAPE, 0x8);
            KEY(VK_CONTROL, 0x100);
            KEY('Q', 0x200);
            KEY('S', 0x400);
            KEY('R', 0x4000);
            KEY(VK_RETURN, 0x1000);
        }
        else
        {
            hr = g_App.keyboard->GetDeviceState(sizeof(keyState), keyState);
            buttons = 0;
            if (hr == DIERR_INPUTLOST)
            {
                g_App.keyboard->Acquire();
            }
            else if (hr != DI_OK)
            {
                g_App.keyboard->Acquire();
            }
            else
            {
                KEY(DIK_UP, 0x10);
                KEY(DIK_DOWN, 0x20);
                KEY(DIK_LEFT, 0x40);
                KEY(DIK_RIGHT, 0x80);
                KEY(DIK_NUMPAD8, 0x10);
                KEY(DIK_NUMPAD2, 0x20);
                KEY(DIK_NUMPAD4, 0x40);
                KEY(DIK_NUMPAD6, 0x80);
                KEY(DIK_NUMPAD7, 0x50);
                KEY(DIK_NUMPAD9, 0x90);
                KEY(DIK_NUMPAD1, 0x60);
                KEY(DIK_NUMPAD3, 0xa0);
                KEY(DIK_HOME, 0x800);
                KEY(DIK_P, 0x800);
                KEY(DIK_D, 0x2000);
                KEY(DIK_Z, 0x1);
                KEY(DIK_X, 0x2);
                KEY(DIK_LSHIFT, 0x4);
                KEY(DIK_ESCAPE, 0x8);
                KEY(DIK_LCONTROL, 0x100);
                KEY(DIK_Q, 0x200);
                KEY(DIK_S, 0x400);
                KEY(DIK_RETURN, 0x1000);
                KEY(DIK_R, 0x4000);
            }
        }
    }
    if (g_App.config.pads[0] == 0 || g_App.config.pads[0] == 1)
    {
        buttons = ReadJoystick(0, g_App.config.pads[0] != 0, buttons);
    }
    if (g_App.config.pads[1] == 0 || g_App.config.pads[1] == 1)
    {
        buttons = ReadJoystick(1, g_App.config.pads[1] != 0, buttons);
    }
    return buttons;
}

BYTE *DecryptData(BYTE *data, int size, BYTE key, BYTE step, int blockSize, int limit)
{
    int unused = 0;
    int tail;
    BYTE *wp;
    BYTE *dest;
    int i;
    BYTE *line;
    BYTE *p;

    tail = (size % blockSize < blockSize / 4) ? size % blockSize : 0;
    p = data;
    dest = (BYTE *)g_MemoryTracker.Alloc(size, GLOBAL_H);
    wp = dest;
    if (dest == NULL)
    {
        return data;
    }
    tail += size & 1;
    size -= tail;
    while (size > 0 && limit > 0)
    {
        if (size < blockSize)
        {
            blockSize = size;
        }
        line = wp;
        wp = wp + blockSize - 1;
        for (i = (blockSize + 1) / 2; i > 0; i--, p++)
        {
            *wp = *p ^ key;
            wp -= 2;
            key += step;
        }
        wp = line + blockSize - 2;
        for (i = blockSize / 2; i > 0; i--, p++)
        {
            *wp = *p ^ key;
            wp -= 2;
            key += step;
        }
        size -= blockSize;
        wp = line + blockSize;
        limit -= blockSize;
    }
    size += tail;
    if (size > 0)
    {
        memcpy(wp, p, size);
    }
    return dest;
}

BYTE *DecryptFileData(BYTE *data, int *size, int fileSize)
{
    DWORD j;
    BYTE *result;
    BYTE *p = data;

    if (p[0] == g_CryptMagic[0] - 0x20 && p[1] == g_CryptMagic[1] - 0x40 && p[2] == g_CryptMagic[2] - 0x60)
    {
        j = 0;
        while (p[3] != g_CryptParams[j].type - (int)(j * 16) - 0x10 && j < 8)
        {
            j++;
        }
        if (j >= 8)
        {
            return p;
        }
        result = DecryptData(p + 4, fileSize - 4, g_CryptParams[j].key, g_CryptParams[j].step,
                             g_CryptParams[j].blockSize, g_CryptParams[j].limit);
        g_MemoryTracker.Free(data);
        return result;
    }
    return p;
}

BYTE *EncryptData(BYTE *data, int size, BYTE key, BYTE step, int blockSize, int limit)
{
    int unused = 0;
    BYTE *start;
    int tail;
    BYTE *wp;
    BYTE *dest;
    int i;
    BYTE *p;

    tail = (size % blockSize < blockSize / 4) ? size % blockSize : 0;
    p = data;
    dest = (BYTE *)g_MemoryTracker.Alloc(size, GLOBAL_H);
    wp = dest;
    if (dest == NULL)
    {
        return data;
    }
    tail += size & 1;
    size -= tail;
    while (size > 0 && limit > 0)
    {
        if (size < blockSize)
        {
            blockSize = size;
        }
        start = p;
        p = p + blockSize - 1;
        for (i = (blockSize + 1) / 2; i > 0; i--, wp++)
        {
            *wp = *p ^ key;
            p -= 2;
            key += step;
        }
        p = start + blockSize - 2;
        for (i = blockSize / 2; i > 0; i--, wp++)
        {
            *wp = *p ^ key;
            p -= 2;
            key += step;
        }
        size -= blockSize;
        p = start + blockSize;
        limit -= blockSize;
    }
    size += tail;
    if (size > 0)
    {
        memcpy(wp, p, size);
    }
    return dest;
}

int FileExists(char *path)
{
    HANDLE file;

    g_App.EnterCriticalSection(2);
    file = CreateFileA(GetDataPath(path), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (file != INVALID_HANDLE_VALUE)
    {
        CloseHandle(file);
        g_App.LeaveCriticalSection(2);
        return 1;
    }
    g_App.LeaveCriticalSection(2);
    return 0;
}

int WriteToFile(char *path, void *data, int size)
{
    HANDLE file;
    DWORD written;

    path = GetDataPath(path);
    g_App.EnterCriticalSection(2);
    file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
    {
        char *msg;

        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msg, 0, NULL);
        LocalFree(msg);
        g_App.LeaveCriticalSection(2);
        return -1;
    }
    WriteFile(file, data, size, &written, NULL);
    if (size != written)
    {
        CloseHandle(file);
        g_App.LeaveCriticalSection(2);
        return -2;
    }
    CloseHandle(file);
    g_App.LeaveCriticalSection(2);
    return 0;
}

int LogPrintf(Log *log, const char *fmt, ...)
{
    int len2;
    char buf[0x2000];
    va_list args;

    va_start(args, fmt);
    g_App.EnterCriticalSection(3);
    vsprintf(buf, fmt, args);
    len2 = strlen(buf);
    if (log->cursor + len2 < log->buffer + sizeof(log->buffer) - 1)
    {
        strcpy(log->cursor, buf);
        log->cursor += len2;
        *log->cursor = '\0';
    }
    va_end(args);
    g_App.LeaveCriticalSection(3);
    return (int)fmt;
}

int LogError(Log *log, const char *fmt, ...)
{
    int len2;
    char buf[0x200];
    va_list args;

    va_start(args, fmt);
    g_App.EnterCriticalSection(3);
    vsprintf(buf, fmt, args);
    len2 = strlen(buf);
    if (log->cursor + len2 < log->buffer + sizeof(log->buffer) - 1)
    {
        strcpy(log->cursor, buf);
        log->cursor += len2;
        *log->cursor = '\0';
    }
    va_end(args);
    log->hasError = 1;
    g_App.LeaveCriticalSection(3);
    return (int)fmt;
}

int Chain::RunCalcChain()
{
    ChainElem *current;
    int updateCount;
    ChainElem *cut;
    int result;

    g_App.EnterCriticalSection(0);
restart:
    updateCount = 0;
    current = &this->calcChain;
    while (current != NULL)
    {
        if (current->callback != NULL)
        {
        again:
            g_App.LeaveCriticalSection(0);
            result = current->callback(current->arg);
            g_App.EnterCriticalSection(0);
            switch (result)
            {
            case 0:
                cut = current;
                current = current->next;
                this->RemoveLocked(cut);
                updateCount++;
                continue;
            case 2:
                goto again;
            case 4:
                updateCount = 0;
                goto end;
            case 3:
                updateCount = 1;
                goto end;
            case 5:
                updateCount = -1;
                goto end;
            case 6:
                goto restart;
            default:
                updateCount++;
                break;
            }
        }
        current = current->next;
    }
end:
    g_App.LeaveCriticalSection(0);
    return updateCount;
}

int Chain::RunDrawChain()
{
    ChainElem *current;
    int updateCount;
    ChainElem *cut;
    int result;

    updateCount = 0;
    current = &this->drawChain;
    g_App.EnterCriticalSection(0);
    while (current != NULL)
    {
        if (current->callback != NULL)
        {
        again:
            g_App.LeaveCriticalSection(0);
            result = current->callback(current->arg);
            g_App.EnterCriticalSection(0);
            switch (result)
            {
            case 0:
                cut = current;
                current = current->next;
                this->RemoveLocked(cut);
                updateCount++;
                continue;
            case 2:
                goto again;
            case 4:
                updateCount = 0;
                goto end;
            case 3:
                updateCount = 1;
                goto end;
            case 5:
                updateCount = -1;
                goto end;
            default:
                updateCount++;
                break;
            }
        }
        current = current->next;
    }
end:
    g_App.LeaveCriticalSection(0);
    return updateCount;
}

void Chain::Remove(ChainElem *elem)
{
    g_App.EnterCriticalSection(0);
    this->RemoveLocked(elem);
    g_App.LeaveCriticalSection(0);
}

WORD InitJoystick()
{
    JOYINFOEX joyInfo;

    joyInfo.dwSize = sizeof(joyInfo);
    joyInfo.dwFlags = JOY_RETURNALL;
    if (joyGetPosEx(JOYSTICKID1, &joyInfo) != JOYERR_NOERROR &&
        joyGetPosEx(JOYSTICKID2, &joyInfo) != JOYERR_NOERROR)
    {
        LogPrintf(&g_Logger, "使えるパッドが存在しないようです、残念\r\n");
        return 1;
    }
    joyGetDevCapsA(JOYSTICKID1, &g_JoyDeviceCaps[0], sizeof(g_JoyDeviceCaps[0]));
    joyGetDevCapsA(JOYSTICKID2, &g_JoyDeviceCaps[1], sizeof(g_JoyDeviceCaps[1]));
    return 0;
}

void *LoadFileData(char *path, int *size, int isExternal)
{
    char *name;
    DWORD dataSize;
    void *buffer;
    HANDLE handle;

    g_App.EnterCriticalSection(2);
    if (isExternal == 0)
    {
        name = strrchr(path, '\\');
        if (name == NULL)
        {
            name = path;
        }
        else
        {
            name++;
        }
        name = strrchr(name, '/');
        if (name == NULL)
        {
            name = path;
        }
        else
        {
            name++;
        }
        dataSize = g_Archive.GetEntrySize(name);
        if (size != NULL)
        {
            *size = dataSize;
        }
        if (dataSize == 0)
        {
            LogError(&g_Logger, "error : %s is not found in arcfile.\r\n", name);
            g_App.LeaveCriticalSection(2);
            return NULL;
        }
        buffer = g_MemoryTracker.Alloc(dataSize, path);
        if (buffer == NULL)
        {
            goto fail;
        }
        g_Archive.Decode(name, buffer);
        buffer = DecryptFileData((BYTE *)buffer, size, dataSize);
        goto done;
    }
    path = GetDataPath(path);
    handle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        goto fail;
    }
    dataSize = GetFileSize(handle, NULL);
    buffer = g_MemoryTracker.Alloc(dataSize, path);
    if (buffer == NULL)
    {
        CloseHandle(handle);
        goto fail;
    }
    ReadFile(handle, buffer, dataSize, &dataSize, NULL);
    if (size != NULL)
    {
        *size = dataSize;
    }
    CloseHandle(handle);
    buffer = DecryptFileData((BYTE *)buffer, size, dataSize);
done:
    g_App.LeaveCriticalSection(2);
    return buffer;
fail:
    g_App.LeaveCriticalSection(2);
    return NULL;
}

void Chain::ReleaseList(ChainElem *root)
{
    ChainElem list;
    ChainElem *tail;
    ChainElem *tmp;
    ChainElem *cur;

    tail = (ChainElem *)g_MemoryTracker.Register(new ChainElem(), sizeof(ChainElem), "funcChainInf");
    list.next = tail;
    cur = root;
    while (cur != NULL)
    {
        tail->self = cur;
        tail->next = (ChainElem *)g_MemoryTracker.Register(new ChainElem(), sizeof(ChainElem), "funcChainInf");
        tail = tail->next;
        cur = cur->next;
    }
    cur = &list;
    while (cur != NULL)
    {
        this->Remove(cur->self);
        cur = cur->next;
    }
    tail = list.next;
    while (tail != NULL)
    {
        tmp = tail->next;
        delete tail;
        tail = NULL;
        tail = tmp;
    }
}

void Chain::Release()
{
    g_App.CloseSubThread();
    this->ReleaseList(&this->calcChain);
    this->ReleaseList(&this->drawChain);
}

float Tan(float angle)
{
    return (float)tan((double)angle);
}

float Acos(float x)
{
    return (float)acos((double)x);
}

float Atan(float x)
{
    return (float)atan((double)x);
}

