#ifdef TH09_NATIVE_NETPLAY
#include "UiText.h"
#include "ascii.h"
#include "gamemanager.h"
#include <d3dx8core.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace ui {
struct Line {float x,y;DWORD color;char text[128];};
static Line lines[24];
static int lineCount=0,fontHeight=0;
static ID3DXFont *font=0;
void Format(float x,float y,DWORD color,const char *format,...) {
 char text[64];va_list args;va_start(args,format);_vsnprintf(text,sizeof(text)-1,format,args);va_end(args);
 text[sizeof(text)-1]=0;
 D3DXVECTOR3 pos(x,y,0);g_AsciiText.color=color;g_AsciiText.SetScale(1,1);g_AsciiText.inArena=0;
 g_AsciiText.AddString(&pos,text);
}
void Title(float x,float y,DWORD color,const char *text) {
 if(lineCount>=24)return;
 Line &line=lines[lineCount++];line.x=x;line.y=y;line.color=color;
 strncpy(line.text,text,sizeof(line.text)-1);line.text[sizeof(line.text)-1]=0;
}
void ReleaseFonts(){if(font){font->Release();font=0;}fontHeight=0;}
void Draw() {
 if(!lineCount)return;
 g_SpriteEngine->FlushQuadBatch();g_App.SetView(2);
 D3DVIEWPORT8 viewport;g_App.d3dDevice->GetViewport(&viewport);
 float sx=viewport.Width/640.0f,sy=viewport.Height/480.0f;
 int height=int(14*sy+.5f);
 if(fontHeight!=height){ReleaseFonts();fontHeight=height;}
 if(!font) {

  HFONT handle=CreateFontA(-height,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
   DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,
   FF_SWISS|VARIABLE_PITCH,"Arial");
  if(handle){D3DXCreateFont(g_App.d3dDevice,handle,&font);DeleteObject(handle);}
 }
 if(font)for(int i=0;i<lineCount;++i) {
  Line &line=lines[i];
  RECT rect={LONG(viewport.X+line.x*sx),LONG(viewport.Y+line.y*sy),LONG(viewport.X+635*sx),LONG(viewport.Y+480*sy)};
  bool selected=line.color==0xffffe0a0;
  bool disabled=line.color==0xff506070;
  DWORD outline=disabled?0xff606078:selected?0xffffe0a0:0xffddd2df;
  DWORD ink=disabled?0xff303040:selected?0xff632c18:0xff51384f;
  font->Begin();
  RECT shadow=rect;OffsetRect(&shadow,2,2);
  font->DrawTextA(line.text,-1,&shadow,DT_NOCLIP|DT_SINGLELINE,0xff101020);
  for(int dx=-1;dx<=1;++dx)for(int dy=-1;dy<=1;++dy)if(dx||dy) {
   RECT edge=rect;OffsetRect(&edge,dx,dy);font->DrawTextA(line.text,-1,&edge,DT_NOCLIP|DT_SINGLELINE,outline);
  }
  font->DrawTextA(line.text,-1,&rect,DT_NOCLIP|DT_SINGLELINE,ink);font->End();
 }
 lineCount=0;g_SpriteEngine->ClearTexture();g_SpriteEngine->ClearBlendMode();
 g_SpriteEngine->ClearSpriteData();g_SpriteEngine->ClearColorOp();
}
}
#endif
