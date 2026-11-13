                                                             
  
                                                                        
                                                                     
                                                               
                                                                     
                                                                    
                                                                      
                                                                       
                                                                        
                                                                  
                                                                     
  
                                                                    
                                                                       
                  

#include <windows.h>
#include <d3d8.h>

#include "trace.h"

static IDirect3D8 *(WINAPI *s_realDirect3DCreate8)(UINT sdkVersion);
                                                       
static DWORD s_nullDraws;

static HRESULT(STDMETHODCALLTYPE *s_realCreateDevice)(IDirect3D8 *d3d, UINT adapter, D3DDEVTYPE type, HWND window,
                                                      DWORD flags, D3DPRESENT_PARAMETERS *params,
                                                      IDirect3DDevice8 **device);

static HRESULT STDMETHODCALLTYPE NullPresent(IDirect3DDevice8 *device, CONST RECT *src, CONST RECT *dst, HWND window,
                                             CONST RGNDATA *dirty)
{
    s_nullDraws++;
    return D3D_OK;
}

static HRESULT STDMETHODCALLTYPE NullClear(IDirect3DDevice8 *device, DWORD count, CONST D3DRECT *rects, DWORD flags,
                                           D3DCOLOR color, float z, DWORD stencil)
{
    s_nullDraws++;
    return D3D_OK;
}

static HRESULT STDMETHODCALLTYPE NullDrawPrimitive(IDirect3DDevice8 *device, D3DPRIMITIVETYPE type, UINT start,
                                                   UINT count)
{
    s_nullDraws++;
    return D3D_OK;
}

static HRESULT STDMETHODCALLTYPE NullDrawIndexedPrimitive(IDirect3DDevice8 *device, D3DPRIMITIVETYPE type,
                                                          UINT minIndex, UINT vertices, UINT start, UINT count)
{
    s_nullDraws++;
    return D3D_OK;
}

static HRESULT STDMETHODCALLTYPE NullDrawPrimitiveUP(IDirect3DDevice8 *device, D3DPRIMITIVETYPE type, UINT count,
                                                     CONST void *data, UINT stride)
{
    s_nullDraws++;
    return D3D_OK;
}

static HRESULT STDMETHODCALLTYPE NullDrawIndexedPrimitiveUP(IDirect3DDevice8 *device, D3DPRIMITIVETYPE type,
                                                            UINT minIndex, UINT vertices, UINT count,
                                                            CONST void *indices, D3DFORMAT format,
                                                            CONST void *data, UINT stride)
{
    s_nullDraws++;
    return D3D_OK;
}

                                                                 
                                                      
static BOOL Unprotect(void *vtbl, SIZE_T size, DWORD *old)
{
    return VirtualProtect(vtbl, size, PAGE_READWRITE, old);
}

static HRESULT STDMETHODCALLTYPE NullCreateDevice(IDirect3D8 *d3d, UINT adapter, D3DDEVTYPE type, HWND window,
                                                  DWORD flags, D3DPRESENT_PARAMETERS *params,
                                                  IDirect3DDevice8 **device)
{
    HRESULT res = s_realCreateDevice(d3d, adapter, type, window, flags, params, device);
    IDirect3DDevice8Vtbl *vtbl;
    DWORD old;

    if (FAILED(res) || *device == NULL)
    {
        return res;
    }
    vtbl = (IDirect3DDevice8Vtbl *)(*device)->lpVtbl;
    if (Unprotect(vtbl, sizeof(*vtbl), &old))
    {
        vtbl->Present = NullPresent;
        vtbl->Clear = NullClear;
        vtbl->DrawPrimitive = NullDrawPrimitive;
        vtbl->DrawIndexedPrimitive = NullDrawIndexedPrimitive;
        vtbl->DrawPrimitiveUP = NullDrawPrimitiveUP;
        vtbl->DrawIndexedPrimitiveUP = NullDrawIndexedPrimitiveUP;
        VirtualProtect(vtbl, sizeof(*vtbl), old, &old);
    }
    return res;
}

static IDirect3D8 *WINAPI NullDirect3DCreate8(UINT sdkVersion)
{
    IDirect3D8 *d3d = s_realDirect3DCreate8(sdkVersion);
    IDirect3D8Vtbl *vtbl;
    DWORD old;

    if (d3d == NULL)
    {
        return NULL;
    }
    vtbl = (IDirect3D8Vtbl *)d3d->lpVtbl;
    if (vtbl->CreateDevice != NullCreateDevice && Unprotect(vtbl, sizeof(*vtbl), &old))
    {
        s_realCreateDevice = vtbl->CreateDevice;
        vtbl->CreateDevice = NullCreateDevice;
        VirtualProtect(vtbl, sizeof(*vtbl), old, &old);
    }
    return d3d;
}

BOOL TraceInstallRenderer(void)
{
    if (TraceEnvNumber("TRACE_RENDER", 0) != 0)
    {
        return TRUE;
    }
    *(void **)&s_realDirect3DCreate8 = TracePatchImport("d3d8.dll", "Direct3DCreate8", (void *)NullDirect3DCreate8);
    TraceInfo(" nullRenderer=%d", s_realDirect3DCreate8 != NULL);
    return s_realDirect3DCreate8 != NULL;
}
