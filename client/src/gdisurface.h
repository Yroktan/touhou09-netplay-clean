

#pragma once

#include <windows.h>
#include <d3d8.h>

#include "common.h"


struct FormatInfo
{

    int format;
    int bitCount;
    DWORD alphaMask;
    DWORD redMask;
    DWORD greenMask;
    DWORD blueMask;
};
ASSERT_SIZE(FormatInfo, 0x18);


extern FormatInfo g_FormatInfos[7];


struct A1R5G5B5
{
    WORD b : 5;
    WORD g : 5;
    WORD r : 5;
    WORD a : 1;
};



class GdiSurface
{
public:

    void *GetBits()
    {
        return this->bits;
    }
    int GetFormat()
    {
        return this->format;
    }
    HDC GetHdc()
    {
        return this->hdc;
    }
    int GetWidthBytes()
    {
        return this->widthBytes;
    }
    int GetWidth()
    {
        return this->width;
    }
    int GetHeight()
    {
        return this->height;
    }
    bool IsCreated()
    {
        return this->bitmap != NULL;
    }

    GdiSurface();
    ~GdiSurface();


    bool Release();


    FormatInfo *FindFormat(int format);


    bool CreateInFormat(int width, int height, int format);


    bool Create(int width, int height, int format);




    bool InvertAlpha(int x, int y, int width, int height, int lightShading);



    bool CopyToSurface(IDirect3DSurface8 *surface);

    int format;
    int width;
    int height;
    DWORD imageSize;
    int widthBytes;
    HDC hdc;
    HGDIOBJ oldObject;
    HBITMAP bitmap;
    void *bits;
};
ASSERT_SIZE(GdiSurface, 0x24);


extern IDirect3DSurface8 *g_TextSurface;



void CreateTextSurface();
void ReleaseTextSurface();


extern COLORREF g_TextOutlineColor;





void RenderTextToTextureLarge(int x, int y, int width, int unused, int fontHeight, int destHeight,
                              COLORREF textColor, COLORREF shadowColor, char *text, IDirect3DTexture8 *texture);

void RenderTextToTexture(int x, int y, int width, int unused, int fontHeight, int destHeight, COLORREF textColor,
                         COLORREF shadowColor, char *text, IDirect3DTexture8 *texture);
