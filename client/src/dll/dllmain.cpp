
#include <windows.h>

struct Hook
{
    DWORD original;
    void *replacement;
};

extern "C" const Hook g_Hooks[];



extern "C" int _fltused = 0;

extern "C" __declspec(dllexport) void Init()
{
}

static void Log(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    wvsprintfA(buf, fmt, args);
    va_end(args);
    OutputDebugStringA(buf);
}

static BOOL InstallHook(const Hook *hook)
{
    BYTE *code = (BYTE *)hook->original;
    DWORD protect;
    if (!VirtualProtect(code, 5, PAGE_EXECUTE_READWRITE, &protect))
    {
        return FALSE;
    }
    code[0] = 0xe9;
    *(DWORD *)(code + 1) = (DWORD)hook->replacement - (hook->original + 5);
    VirtualProtect(code, 5, protect, &protect);
    return code[0] == 0xe9;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    if (reason != DLL_PROCESS_ATTACH)
    {
        return TRUE;
    }
    int installed = 0;
    for (const Hook *hook = g_Hooks; hook->original; hook++)
    {
        if (!InstallHook(hook))
        {
            Log("th09: cannot hook %08x\n", hook->original);
            return FALSE;
        }
        Log("th09: %08x -> %08x\n", hook->original, hook->replacement);
        installed++;
    }
    FlushInstructionCache(GetCurrentProcess(), NULL, 0);
    Log("th09: %d hooks installed\n", installed);
    return TRUE;
}
