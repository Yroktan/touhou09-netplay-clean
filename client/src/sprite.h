


#pragma once

#include <d3dx8.h>

#include "common.h"
#include "timer.h"

class AnmLoaded;


union AnmArg
{
    int i;
    float f;
    BYTE b[4];
};


struct AnmInstr
{
    short opcode;

    WORD size;

    short time;

    WORD paramMask;
    AnmArg args[1];
};
struct SpriteData;



struct SpriteBase
{

    D3DXVECTOR3 rotation;
    D3DXVECTOR3 rotationSpeed;

    float scaleX;
    float scaleY;

    float scaleSpeedX;
    float scaleSpeedY;

    float spriteWidth;
    float spriteHeight;

    float uvScrollX;
    float uvScrollY;

    Timer scriptTime;
    Timer waitTimer;

    Timer interpDuration[7];
    Timer interpFrame[7];

    BYTE posInterpEasing;
    BYTE colorInterpEasing;
    BYTE alphaInterpEasing;
    BYTE rotationInterpEasing;
    BYTE scaleInterpEasing;
    BYTE color2InterpEasing;
    BYTE color2AlphaInterpEasing;
    unsigned char pad_ff[0x1];


    int scriptVar0;
    int scriptVar1;
    int scriptVar2;
    int scriptVar3;
    float scriptVarF0;
    float scriptVarF1;
    float scriptVarF2;
    float scriptVarF3;
    int scriptVar8;
    int scriptVar9;

    float uvScrollSpeedX;
    float uvScrollSpeedY;

    D3DXMATRIX matrix;
    D3DXMATRIX drawMatrix;

    D3DXMATRIX sizeMatrix;
    union
    {
        D3DCOLOR color;
        struct
        {
            BYTE colorRgb[3];
            BYTE alpha;
        };
    };

    union
    {
        D3DCOLOR color2;
        struct
        {
            BYTE color2Rgb[3];
            BYTE color2Alpha;
        };
    };

    union
    {
        DWORD flags;
        struct
        {
            DWORD shown : 1;
            DWORD visible : 1;



            DWORD rotationChanged : 1;
            DWORD scaleChanged : 1;
            DWORD blendMode : 2;
            DWORD unk_bit6 : 1;
            DWORD pad_bit7 : 1;
            DWORD usePosOffset : 1;
            DWORD flip : 2;
            DWORD anchor : 2;
            DWORD zWriteDisabled : 1;
            DWORD stopped : 1;
            DWORD cameraSetup : 1;
            DWORD freezeMatrix : 1;
            DWORD useColor2 : 1;
            DWORD pad_bit18 : 1;
            DWORD scriptPaused : 1;
        };
    };

    short autoRotate;

    short pendingInterrupt;

    int drawMode;

    AnmLoaded *anm;
};
ASSERT_SIZE(SpriteBase, 0x208);


class Sprite : public SpriteBase
{
public:

    Sprite()
    {
        memset(this, 0, sizeof(Sprite));
        this->spriteIndex = -1;
    }

    D3DXVECTOR3 pos;
    short spriteIndex;

    short anmSlot;
    short baseSpriteIndex;

    short scriptIndex;

    AnmInstr *scriptStart;
    AnmInstr *scriptPos;
    SpriteData *spriteData;

    Timer interruptReturnTime;
    AnmInstr *interruptReturnPos;



    D3DXVECTOR3 moveStart;
    D3DXVECTOR3 moveEnd;
    D3DXVECTOR3 rotateStart;
    D3DXVECTOR3 rotateEnd;
    D3DVECTOR scaleTween;
    D3DVECTOR colorTween;
    D3DCOLOR color2TweenStart;
    D3DCOLOR color2TweenEnd;

    D3DXVECTOR3 posOffset;
    int spriteTime;


    BYTE fontWidth;
    BYTE fontHeight;
    unsigned char pad_29a[0xa];



    int ResolveIntArgument(int value);



    float *ResolveFloatArgumentPtr(float *value, WORD mask, int bit);
    int *ResolveIntArgumentPtr(int *value, WORD mask, int bit);


    float ResolveFloatArgument(float value);





    void SetMoveInterp(int frames, BYTE easing, D3DXVECTOR3 *from, D3DXVECTOR3 *to);
    void SetColorInterp(int frames, BYTE easing, D3DCOLOR from, D3DCOLOR to);


    void SetPendingInterrupt(short interrupt)
    {
        this->pendingInterrupt = interrupt;
    }


    void SetRotationZ(float angle)
    {
        this->rotation.z = angle;
        this->rotationChanged = 1;
    }

    void SetBlendModeAdd()
    {
        this->blendMode = 1;
    }
    void SetBlendModeNormal()
    {
        this->blendMode = 0;
    }


    BOOL IsShown()
    {
        return this->flags & 1;
    }

    void ClearShown()
    {
        this->flags &= ~1;
    }




    void Initialize()
    {
        memset(this, 0, sizeof(SpriteBase));
        this->scaleX = 1.0f;
        this->scaleY = 1.0f;
        this->color = 0xffffffff;
        D3DXMatrixIdentity(&this->matrix);
        *(WORD *)&this->flags = 7;
        this->scriptTime.Initialize();
    }
};
ASSERT_SIZE(Sprite, 0x2a4);



struct SpriteData
{

    int sourceFileIndex;

    IDirect3DTexture8 *texture;

    float startX;
    float startY;
    float endX;
    float endY;

    float textureHeight;
    float textureWidth;

    float uvStartX;
    float uvStartY;
    float uvEndX;
    float uvEndY;

    float height;

    float width;

    float scaleX;
    float scaleY;
    int pad_40;
};
ASSERT_SIZE(SpriteData, 0x44);


struct AnmTexture
{
    IDirect3DTexture8 *texture;
    void *data;
    int size;
};



struct AnmImage
{
    unsigned char pad_0[0x6];

    short format;
    short width;
    short height;
    unsigned char pad_c[0x4];
    BYTE data[1];
};



struct AnmSprite
{
    int id;
    float x;
    float y;
    float width;
    float height;
};



struct AnmFile
{
    int numSprites;
    int numScripts;
    int textureIndex;
    int width;
    int height;
    int format;
    D3DCOLOR colorKey;

    int nameOffset;
    int spriteOffset;
    int alphaNameOffset;

    int version;

    DWORD priority;

    int imageOffset;
    BYTE hasImage;
    BYTE ownsData;
    unsigned char pad_36[0x2];

    int nextOffset;
    unsigned char pad_3c[0x4];
    int spriteOffsets[1];
};

class AnmLoaded
{
public:


    void StartScriptAtOrigin(Sprite *vm, int script)
    {
        vm->scriptIndex = script;
        vm->pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->fontHeight = 15;
        vm->fontWidth = 15;
        this->SetScript(vm, this->scripts[script]);
    }

    void StartScript(Sprite *vm, int script)
    {
        vm->anm = this;
        vm->scriptIndex = script;
        this->SetScript(vm, this->scripts[script]);
    }


    void SetScript(Sprite *vm, void *script);



    void StartSpriteSeries(Sprite *vms, int script, int count);


    int SetSprite(Sprite *vm, int sprite);


    void InitSprite(Sprite *vm, int sprite)
    {
        vm->Initialize();
        vm->anm = this;
        this->SetSprite(vm, sprite);
    }


    void LoadSprite(int index, SpriteData *sprite);



    SpriteData *GetSprite(int sprite)
    {
        return &this->sprites[sprite];
    }


    int slot;
    AnmFile *file;

    int anmCount;
    SpriteData *sprites;
    void **scripts;
    AnmTexture *textures;


    int loadStep;

    int releaseRequested;

    char path[0x100];
};
ASSERT_SIZE(AnmLoaded, 0x120);



struct VertexTex1DiffuseXyzrwh
{
    D3DXVECTOR3 pos;
    float w;
    D3DCOLOR diffuse;
    struct
    {
        float x;
        float y;
    } uv;
};


struct VertexDiffuseXyzrwh
{
    D3DXVECTOR3 pos;
    float w;
    D3DCOLOR diffuse;
};



struct VertexTex1Xyzrwh
{
    D3DXVECTOR3 pos;
    float w;
    struct
    {
        float x;
        float y;
    } uv;
};



struct VertexTex1Xyz
{
    D3DXVECTOR3 pos;
    D3DXVECTOR2 uv;
};



struct VertexTex1DiffuseXyz
{
    D3DXVECTOR3 pos;
    D3DCOLOR diffuse;
    D3DXVECTOR2 uv;
};


class SpriteEngine
{
public:


    SpriteEngine();

    ~SpriteEngine();


    void ClearTexture()
    {
        this->currentTexture = NULL;
    }

    void SetLastCameraSetupFlag(BYTE value)
    {
        this->lastCameraSetupFlag = value;
    }

    void ResetColorModulation()
    {
        this->modulateColorEnabled = 0;
        this->modulateColor = 0x80808080;
    }

    void SetColorModulation(D3DCOLOR color)
    {
        this->modulateColorEnabled = 1;
        this->modulateColor = color;
    }

    void ClearBlendMode()
    {
        this->blendMode = 3;
    }

    void ClearVertexFormat()
    {
        this->vertexFormat = 0xff;
    }

    void ClearSpriteData()
    {
        this->currentSpriteData = NULL;
    }

    void ClearColorOp()
    {
        this->colorOp = 0xff;
    }

    void ClearZWrite()
    {
        this->zWriteDisabled = 0xff;
    }

    void ClearLastCameraSetupFlag()
    {
        this->lastCameraSetupFlag = 0xff;
    }



    int SetPendingSurfaceCopy(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft,
                              int dstTop, int dstWidth, int dstHeight);


    void ClearDrawStats()
    {
        this->stepCount = 0;
        this->renderStateSetCount = 0;
        this->scriptStartCount = 0;
        this->batchDrawCount = 0;
    }



    void ResetFrameState();





    AnmLoaded *PreloadAnmFile(int slot, char *path);


    AnmLoaded *LoadAnm(int slot, char *path);


    AnmLoaded *PreloadAnm(int slot, char *path);



    int LoadAnmSprites(AnmLoaded *anm, int index, int spriteOffset, int scriptOffset, AnmFile *file);

    void ReleaseAnm(int slot);


    int LoadSurface(int surface, char *path);



    int PreloadSurface(int surface, char *path);



    AnmLoaded *PostloadAnm(AnmLoaded *anm);


    int StepAnmLoading();

    void DrawSprite(Sprite *vm);


    int DrawUnrotated(Sprite *vm);

    int DrawNoRound(Sprite *vm);

    int DrawUnrotatedMirrored(Sprite *vm);

    int DrawRotatedNoRound(Sprite *vm);



    int ProjectSprite(Sprite *vm);

    void ProjectSprite3D(Sprite *vm);


    int DrawQuad(Sprite *vm, int flags);



    int DrawVertexStrip(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count);


    int Draw3D(Sprite *vm);


    int DrawVertexQuad(Sprite *vm, VertexTex1DiffuseXyzrwh *quad);



    int DrawColoredStrip(Sprite *vm, VertexDiffuseXyzrwh *vertices, int count);
    int DrawColoredFan(Sprite *vm, VertexDiffuseXyzrwh *vertices, int count);


    int DrawVertexFan(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count);


    void CreateQuad();


    void SetRenderStateForSprite(Sprite *vm);



    int FillSpriteStripVertices(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count);

    int FillVerticalStripVertices(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count);


    void SetBlendModeForSprite(Sprite *vm);

    int DrawQuadMirrored(Sprite *vm, int flags);

    int DrawProjected(Sprite *vm);

    int DrawProjected3D(Sprite *vm);

    int Step(Sprite *vm);


    void RotatePoint(float *out, float x, float y, float sine, float cosine, float cx, float cy);


    void ReleaseTexture(AnmTexture *tex);




    void RenderTextToTextureIndex(IDirect3DTexture8 *texture, int x, int y, int width, int unused, int fontHeight,
                                  int destHeight, D3DCOLOR textColor, D3DCOLOR shadowColor, char *text, float scaleX,
                                  float scaleY);


    void RenderText(Sprite *sprite, D3DCOLOR textColor, D3DCOLOR shadowColor, char *fmt, ...);

    void RenderTextCentered(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...);


    void ReleaseSurface(int surface);



    void CopySurfaceToBackBuffer(int surface, int left, int top, int x, int y);


    void CopySurfaceRectToBackBuffer(int surface, int x, int y, int left, int top, int width, int height);


    void ReleaseSurfaces();


    void DoPendingCopy();


    void CopyBackBufferToTexture(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft,
                                 int dstTop, int dstWidth, int dstHeight);


    void CopyBackBufferToSurface(int surface, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft,
                                 int dstTop, int dstWidth, int dstHeight);


    int CreateTexture(AnmTexture *tex, int width, int height, int format);



    int PreloadAnmTexture(AnmLoaded *anm, int index, int *spriteIndex, int *scriptIndex, AnmFile *file);



    int LoadTexture(AnmTexture *tex, int format, D3DCOLOR colorKey);



    int CreateTextureFromImage(AnmTexture *tex, AnmImage *image, int format);


    void SendInterruptToLoadedSprites(Sprite *vms, int count, short interrupt);

    void StepSprites(Sprite *vms, int count);

    void BeginQuadBatch();

    void FlushQuadBatch();


    int AddQuadToBatch(VertexTex1DiffuseXyzrwh *quad);

    int Draw(Sprite *vm);


    void DrawAndFlush(Sprite *vm)
    {
        this->Draw(vm);
        this->FlushQuadBatch();
    }


    AnmLoaded *GetAnm(int slot)
    {
        return &this->anms[slot];
    }

    AnmFile *GetAnmFile(int slot)
    {
        return this->GetAnm(slot)->file;
    }



    BOOL HasTexture(Sprite *vm)
    {
        if (vm->spriteData == NULL)
        {
            return FALSE;
        }
        if (vm->spriteData->sourceFileIndex < 0)
        {
            return FALSE;
        }
        return this->anms[vm->spriteData->sourceFileIndex].textures != NULL;
    }


    IDirect3DSurface8 *GetSurface(int surface)
    {
        return this->surfaces[surface];
    }

    void ReleaseVertexBuffer();

    D3DCOLOR modulateColor;
    int modulateColorEnabled;


    int pendingSurfaceCopySurface;

    int scriptStartCount;

    int stepCount;
    int renderStateSetCount;
    int batchDrawCount;

    float shakeX;
    float shakeY;

    AnmLoaded anms[0x100];

    D3DXMATRIX lastWorldMatrix;
    Sprite unknownSprite;
    unsigned char pad_12308[0x123fc - 0x12308];


    IDirect3DSurface8 *surfaces[0x20];

    IDirect3DSurface8 *surfacesBackup[0x20];

    void *surfaceFiles[0x20];
    int surfaceFileSizes[0x20];

    D3DXIMAGE_INFO surfaceInfos[0x20];

    D3DCOLOR currentColor;


    IDirect3DTexture8 *currentTexture;
    BYTE blendMode;
    BYTE colorOp;
    BYTE vertexFormat;
    BYTE zWriteDisabled;
    BYTE lastCameraSetupFlag;
    unsigned char pad_12889[0x1288c - 0x12889];

    SpriteData *currentSpriteData;

    IDirect3DVertexBuffer8 *vertexBuffer;

    VertexTex1Xyz quad[4];

    int batchQuadCount;

    VertexTex1DiffuseXyzrwh batchVertices[0x18000];

    VertexTex1DiffuseXyzrwh *batchCursor;
    VertexTex1DiffuseXyzrwh *batchStart;

    int pendingTexCopyTexture;

    int pendingTexCopySrcLeft;
    int pendingTexCopySrcTop;
    int pendingTexCopySrcWidth;
    int pendingTexCopySrcHeight;
    int pendingTexCopyDstLeft;
    int pendingTexCopyDstTop;
    int pendingTexCopyDstWidth;
    int pendingTexCopyDstHeight;
    int pendingSurfaceCopySrcLeft;
    int pendingSurfaceCopySrcTop;
    int pendingSurfaceCopySrcWidth;
    int pendingSurfaceCopySrcHeight;
    int pendingSurfaceCopyDstLeft;
    int pendingSurfaceCopyDstTop;
    int pendingSurfaceCopyDstWidth;
    int pendingSurfaceCopyDstHeight;
};
extern SpriteEngine *g_SpriteEngine;


extern VertexTex1Xyzrwh g_TexQuad[4];


extern VertexTex1DiffuseXyz g_DiffuseQuadXyz[4];


extern const float g_Half;



extern VertexTex1DiffuseXyzrwh g_DiffuseQuad[4];


extern const D3DFORMAT g_TextureFormats[];

extern const int g_TextureFormatSizes[];
