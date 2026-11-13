

#include <string.h>

#include <d3dx8tex.h>

#include "gdisurface.h"
#include "unsorted.h"

GdiSurface::GdiSurface()
{
    this->format = -1;
    this->width = 0;
    this->height = 0;
    this->hdc = NULL;
    this->bitmap = NULL;
    this->oldObject = NULL;
    this->bits = NULL;
}

bool GdiSurface::Release()
{
    if (this->hdc != NULL)
    {
        SelectObject(this->hdc, this->oldObject);
        DeleteDC(this->hdc);
        DeleteObject(this->bitmap);
        this->format = -1;
        this->width = 0;
        this->height = 0;
        this->hdc = NULL;
        this->bitmap = NULL;
        this->oldObject = NULL;
        this->bits = NULL;
        return true;
    }
    return false;
}

FormatInfo *GdiSurface::FindFormat(int format)
{
    int i;
    FormatInfo *fmt;

    for (i = 0; (fmt = &g_FormatInfos[i])->format != -1 && fmt->format != format; i++)
    {
    }
    if (format == -1)
    {
        return NULL;
    }
    return &g_FormatInfos[i];
}

bool GdiSurface::InvertAlpha(int x, int y, int width, int height, int lightShading)
{
    BYTE *line;
    int offset;
    int total;
    int val;

    total = width * height * 2;
    line = (BYTE *)this->GetBits() + y * width * 2;
    switch (this->format)
    {
    case D3DFMT_A8R8G8B8:
        for (offset = 3; offset < total; offset += 4)
        {
            line[offset] ^= 0xff;
        }
        break;
    case D3DFMT_A1R5G5B5:
        A1R5G5B5 *px;
        for (px = (A1R5G5B5 *)line, offset = 0; offset < total; offset += 2, px++)
        {
            px->a ^= 1;
            if (px->a)
            {
                if (lightShading)
                {
                    if (px->r >= px->b)
                    {
                        val = px->r - px->r * offset / total / 4;
                        px->r = val >= 32 ? 31 : val;
                        val = px->g - px->g * offset / total / 4;
                        px->g = val >= 32 ? 31 : val;
                    }
                    else
                    {
                        val = px->b - px->b * offset / total / 4;
                        px->b = val >= 32 ? 31 : val;
                        val = px->g - px->g * offset / total / 4;
                        px->g = val >= 32 ? 31 : val;
                    }
                }
            }
            else
            {
                px->r = 0;
                px->g = 0;
                px->b = 0;
            }
        }
        break;
    case D3DFMT_A4R4G4B4:
        for (offset = 1; offset < total; offset += 2)
        {
            line[offset] ^= 0xf0;
        }
        break;
    default:
        return false;
    }
    return true;
}

bool GdiSurface::CopyToSurface(IDirect3DSurface8 *surface)
{
    BYTE *dst;
    int lineSize;
    RECT region;
    int row;
    int srcPitch;
    D3DSURFACE_DESC surfaceDesc;
    BYTE *src;
    D3DLOCKED_RECT lockedSurface;

    if (!this->IsCreated())
    {
        return false;
    }
    surface->GetDesc(&surfaceDesc);
    region.left = 0;
    region.top = 0;
    region.right = this->GetWidth();
    region.bottom = this->GetHeight();
    if (surface->LockRect(&lockedSurface, &region, 0) != D3D_OK)
    {
        return false;
    }
    lineSize = lockedSurface.Pitch;
    srcPitch = this->GetWidthBytes();
    src = (BYTE *)this->GetBits();
    dst = (BYTE *)lockedSurface.pBits;
    if (surfaceDesc.Format == this->GetFormat())
    {
        for (row = 0; row < this->GetHeight(); row++)
        {
            memcpy(dst, src, srcPitch);
            src += srcPitch;
            dst += lineSize;
        }
    }
    surface->UnlockRect();
    return true;
}

void CreateTextSurface()
{
    g_App.d3dDevice->CreateImageSurface(1024, 64, D3DFMT_A1R5G5B5, &g_TextSurface);
}

void ReleaseTextSurface()
{
    if (g_TextSurface != NULL)
    {
        g_TextSurface->Release();
        g_TextSurface = NULL;
    }
}

GdiSurface::~GdiSurface()
{
    this->Release();
}

bool GdiSurface::CreateInFormat(int width, int height, int format)
{
    int widthBytes;
    HDC dc;
    HGDIOBJ oldObject;
    BITMAPV4HEADER bitmapInfo;
    FormatInfo *fmt;
    HBITMAP bitmap;
    void *data;

    this->Release();
    memset(&bitmapInfo, 0, sizeof(bitmapInfo));
    fmt = this->FindFormat(format);
    if (fmt == NULL)
    {
        return false;
    }
    widthBytes = (width * fmt->bitCount / 8 + 3) / 4 * 4;
    bitmapInfo.bV4Size = sizeof(bitmapInfo);
    bitmapInfo.bV4Width = width;
    bitmapInfo.bV4Height = -(height + 1);
    bitmapInfo.bV4Planes = 1;
    bitmapInfo.bV4BitCount = fmt->bitCount;
    bitmapInfo.bV4SizeImage = height * widthBytes;
    if (format != D3DFMT_X1R5G5B5 && format != D3DFMT_X8R8G8B8)
    {
        bitmapInfo.bV4V4Compression = BI_BITFIELDS;
        bitmapInfo.bV4RedMask = fmt->redMask;
        bitmapInfo.bV4GreenMask = fmt->greenMask;
        bitmapInfo.bV4BlueMask = fmt->blueMask;
        bitmapInfo.bV4AlphaMask = fmt->alphaMask;
    }
    bitmap = CreateDIBSection(NULL, (BITMAPINFO *)&bitmapInfo, DIB_RGB_COLORS, &data, NULL, 0);
    if (bitmap == NULL)
    {
        return false;
    }
    memset(data, 0, bitmapInfo.bV4SizeImage);
    dc = CreateCompatibleDC(NULL);
    oldObject = SelectObject(dc, bitmap);
    this->hdc = dc;
    this->bitmap = bitmap;
    this->bits = data;
    this->imageSize = bitmapInfo.bV4SizeImage;
    this->oldObject = oldObject;
    this->width = width;
    this->height = height;
    this->format = format;
    this->widthBytes = widthBytes;
    return true;
}

bool GdiSurface::Create(int width, int height, int format)
{
    if (this->CreateInFormat(width, height, format))
    {
        return true;
    }
    if (format == D3DFMT_A1R5G5B5 || format == D3DFMT_A4R4G4B4)
    {
        return this->CreateInFormat(width, height, D3DFMT_A8R8G8B8);
    }
    if (format == D3DFMT_R5G6B5)
    {
        return this->CreateInFormat(width, height, D3DFMT_X8R8G8B8);
    }
    return false;
}

void RenderTextToTextureLarge(int x, int y, int width, int unused, int fontHeight, int destHeight, COLORREF textColor,
        COLORREF shadowColor, char *text, IDirect3DTexture8 *texture)
{
    HDC hdcMem;
    HFONT hNewFont;
    D3DSURFACE_DESC sd;
    HGDIOBJ oldObject;
    RECT rcSrc;
    RECT rcDst;

    hNewFont = CreateFontA(fontHeight * 2, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_ROMAN | FIXED_PITCH,
                           "ＭＳ ゴシック");
    GdiSurface dib;
    g_TextSurface->GetDesc(&sd);
    dib.Create(sd.Width, sd.Height, sd.Format);
    hdcMem = dib.GetHdc();
    oldObject = SelectObject(hdcMem, hNewFont);
    dib.InvertAlpha(0, 0, width * 2, fontHeight * 2 + 6, 0);
    SetBkMode(hdcMem, TRANSPARENT);
    if (shadowColor != (COLORREF)-1)
    {
        SetTextColor(hdcMem, g_TextOutlineColor);
        TextOutA(hdcMem, x * 2 + 4, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 0, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 4, text, strlen(text));
    }
    else
    {
        SetTextColor(hdcMem, g_TextOutlineColor);
        TextOutA(hdcMem, x * 2 + 3, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 1, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 1, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 3, text, strlen(text));
    }
    SetTextColor(hdcMem, textColor);
    TextOutA(hdcMem, x * 2 + 2, 2, text, strlen(text));
    SelectObject(hdcMem, oldObject);
    dib.InvertAlpha(0, 0, width * 2, fontHeight * 2 + 6, shadowColor == (COLORREF)-1);
    dib.CopyToSurface(g_TextSurface);
    SelectObject(hdcMem, oldObject);
    DeleteObject(hNewFont);
    rcDst.left = 0;
    rcDst.top = y;
    rcDst.right = width;
    rcDst.bottom = y + destHeight;
    rcSrc.left = 0;
    rcSrc.top = 0;
    rcSrc.right = width * 2 + 2;
    rcSrc.bottom = fontHeight * 2 + 2;
    if (rcSrc.right > 1024)
    {
        rcSrc.right = 1024;
    }

    {
        IDirect3DSurface8 *dstSurface;
        texture->GetSurfaceLevel(0, &dstSurface);
        D3DXLoadSurfaceFromSurface(dstSurface, NULL, &rcDst, g_TextSurface, NULL, &rcSrc, D3DX_FILTER_TRIANGLE, 0);
        if (dstSurface != NULL)
        {
            dstSurface->Release();
            dstSurface = NULL;
        }
    }
}

void RenderTextToTexture(int x, int y, int width, int unused, int fontHeight, int destHeight, COLORREF textColor,
        COLORREF shadowColor, char *text, IDirect3DTexture8 *texture)
{
    HDC hdcMem;
    HFONT hNewFont;
    D3DSURFACE_DESC sd;
    HGDIOBJ oldObject;
    RECT rcSrc;
    RECT rcDst;

    hNewFont = CreateFontA(fontHeight * 2, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_ROMAN | FIXED_PITCH,
                           "ＭＳ ゴシック");
    GdiSurface dib;
    g_TextSurface->GetDesc(&sd);
    dib.Create(sd.Width, sd.Height, sd.Format);
    hdcMem = dib.GetHdc();
    oldObject = SelectObject(hdcMem, hNewFont);
    dib.InvertAlpha(0, 0, sd.Width, fontHeight * 2 + 6, 0);
    SetBkMode(hdcMem, TRANSPARENT);
    if (shadowColor != (COLORREF)-1)
    {
        SetTextColor(hdcMem, g_TextOutlineColor);
        TextOutA(hdcMem, x * 2 + 4, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 0, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 4, text, strlen(text));
    }
    else
    {
        SetTextColor(hdcMem, g_TextOutlineColor);
        TextOutA(hdcMem, x * 2 + 3, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 1, 2, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 1, text, strlen(text));
        TextOutA(hdcMem, x * 2 + 2, 3, text, strlen(text));
    }
    SetTextColor(hdcMem, textColor);
    TextOutA(hdcMem, x * 2 + 2, 2, text, strlen(text));
    SelectObject(hdcMem, oldObject);
    dib.InvertAlpha(0, 0, sd.Width, fontHeight * 2 + 6, shadowColor == (COLORREF)-1);
    dib.CopyToSurface(g_TextSurface);
    SelectObject(hdcMem, oldObject);
    DeleteObject(hNewFont);
    rcDst.left = 0;
    rcDst.top = y;
    rcDst.right = width;
    rcDst.bottom = y + destHeight;
    rcSrc.left = 0;
    rcSrc.top = 0;
    rcSrc.right = width * 2;
    rcSrc.bottom = fontHeight * 2;
    if (rcSrc.right > 1024)
    {
        rcSrc.right = 1024;
    }

    {
        IDirect3DSurface8 *dstSurface;
        texture->GetSurfaceLevel(0, &dstSurface);
        D3DXLoadSurfaceFromSurface(dstSurface, NULL, &rcDst, g_TextSurface, NULL, &rcSrc, D3DX_FILTER_TRIANGLE, 0);
        if (dstSurface != NULL)
        {
            dstSurface->Release();
            dstSurface = NULL;
        }
    }
}
