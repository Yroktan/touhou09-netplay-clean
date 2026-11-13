#pragma once
#include <d3d8.h>
namespace ui {
void Format(float x,float y,DWORD color,const char *format,...);
void Title(float x,float y,DWORD color,const char *text);
void Draw();
void ReleaseFonts();
}
