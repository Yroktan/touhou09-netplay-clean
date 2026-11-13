



#pragma once

struct App
{

    App();




    void SetView(int view)
    {
        this->camera = &this->cameras[view];
        this->SetupCamera3D(this->camera);
        this->d3dDevice->SetViewport(&this->camera->viewport);
        this->view = view;
    }


    void SetView2D(int view)
    {
        this->camera = &this->cameras[view];
        this->SetupCamera(this->camera);
        this->d3dDevice->SetViewport(&this->camera->viewport);
        this->view = view;
    }

    void SetupCamera(Camera *camera);


    void ResetView()
    {
        this->SetupCamera(this->camera);
        this->d3dDevice->SetViewport(&this->camera->viewport);
    }


    void SetupCamera3D(Camera *camera);



    void InitCameras();

    void ResetView3D()
    {
        this->SetupCamera3D(this->camera);
        this->d3dDevice->SetViewport(&this->camera->viewport);
    }


    void SetCamera(int view)
    {
        this->camera = &this->cameras[view];
    }

    int GetNoFog()
    {
        return this->config.flags.noFog;
    }


    int GetNoScore()
    {
        return this->stateFlags >> 3 & 1;
    }



    int GetForce16BitTextures()
    {
        return this->config.flags.force16BitTextures;
    }
    int GetReferenceRasterizer()
    {
        return this->config.flags.referenceRasterizer;
    }
    int GetWindowed()
    {
        return this->config.windowed;
    }
    int GetNoDirectInput()
    {
        return this->config.flags.noDirectInput;
    }
    int GetPreloadBgm();
    int GetNoVsync();
    int GetNoTextRenderDetect();


    int SaveScreenshot(char *path);


    BOOL HasSubThread()
    {
        return this->subThread != NULL;
    }


    void EnterCriticalSection(int idx);
    void LeaveCriticalSection(int idx);



    void AddRecord(int side, int value, WORD unknown, WORD seed);



    void InsertRecord(int side, int value, WORD seed);



    WORD PopRecord(int side, WORD *seed, int *unk_8);


    BOOL RecordsMeet(int side);



    void ResetRun();



    int LoadBgm(int id);

    int PlayBgm();


    int StopMusic();


    int UnloadBgm(int id);



    int FadeOutMusic(float seconds);


    int PlayMusicById(int id, int arg);



    int PlayMusic(char *path, int heard);


    void TickTimer(int *frames, float *subFrame);


    void StartLoadingSprites(D3DXVECTOR3 *pos);


    void StopLoadingSprites();


    void StartLoadingSpritesOverScreen(D3DXVECTOR3 *pos);


    void FinishLoadingSprites();


    void StartLoadingScreenEffect(int type);

    void InitCriticalSections();
    void DeleteCriticalSections();


    int EnableFog();

    int DisableFog();


    int SetRenderState(D3DRENDERSTATETYPE state, DWORD value);


    void AddPlayTime();


    void AddTotalPlayTime();



    int CheckVersionFile(char *version, DWORD size, DWORD checksum);





    int LoadConfig(char *path);


    void CloseSubThread();

    int StartSubThread(LPTHREAD_START_ROUTINE proc, LPVOID arg);




    static int OnUpdate(App *app);
    static int AddedCallback(App *app);
    static int DeletedCallback(App *app);
    static int OnDraw(App *app);
    static int OnDrawLogo(App *app);
    static int OnDrawLoading(App *app);


    static int RegisterChain();

    static void LoadingThread(App *app);


    int UpdateNetPlay();

    void ResetSubThread();

    void ResetClockDriftCounters();


    HINSTANCE hInstance;

    IDirect3D8 *d3d;
    IDirect3DDevice8 *d3dDevice;


    IDirectInput8 *dinput;
    IDirectInputDevice8 *keyboard;
    union
    {
        struct
        {
            IDirectInputDevice8 *joystick;
            IDirectInputDevice8 *joystick2;
        };

        IDirectInputDevice8 *joysticks[2];
    };
    DIDEVCAPS joystickCaps;
    DIDEVCAPS joystickCaps2;

    HWND hwnd;

    Camera cameras[3];

    Camera *camera;

    int view;

    D3DPRESENT_PARAMETERS presentParams;

    MidiTimer *timer;
    GameConfig config;

    int frames;
    int unk_458;
    int unk_45c;
    int unk_460;

    DWORD unk_464;
    DWORD unk_468;
    DWORD unk_46c;
    unsigned char pad_470[0x474 - 0x470];
    DWORD unk_474;
    int unk_478;

    RunRecord records[2][10];
    RunRecord lastRecords[2];

    Sprite *unk_584;

    DWORD connectStartTime;

    int curScene;

    int scene;
    int prevScene;


    int freesResources;

    int unk_59c;
    unsigned char pad_5a0[0x5a4 - 0x5a0];


    int redrawFrames;

    int hideFps;

    int noVsync;

    int wants60Hz;

    int clearedOnD3dInit;

    float gameSpeed;
    MidiPlayer *midi;


    float achievedFrames;
    float expectedFrames;


    short roundedFps;
    unsigned char pad_5ca[0x5cc - 0x5ca];

    AnmLoaded *textAnm;

    AnmLoaded *loadingAnm;
    union
    {
        DWORD stateFlags;
        struct
        {
            unsigned int unusedBits5d4_0 : 9;

            unsigned int loadsResources : 1;
        };
    };

    D3DCAPS8 caps;

    HANDLE subThread;
    DWORD subThreadId;

    int closeSubThread;
    int subThreadRunning;

    int loadingThreadState;
    CRITICAL_SECTION criticalSections[5];

    BYTE criticalSectionDepths[5];
    unsigned char pad_73d[0x740 - 0x73d];


    int loadingState;


    DWORD perfFrequency[2];
    unsigned char pad_74c[0x77c - 0x74c];
    int unk_77c;

    int fpsWarning;
    int unk_784;
    int unk_788;
    int unk_78c;
    int unk_790;

    DWORD fogState;

    DWORD lastPlayTimeTick;
    DWORD lastTotalPlayTimeTick;


    DWORD exeChecksum;
    DWORD exeSize;


    DWORD verFileSize;
    char *verFile;
};
extern App g_App;


int FreesResources();


extern Sprite g_LoadingScreenSprites[3];




BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext);
BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext);
BOOL CALLBACK EnumAxesCallback2(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext);



int LoadScoreFile();
int SaveScoreDat();

class ScreenEffect;

extern ScreenEffect *g_LoadingScreenEffect;
