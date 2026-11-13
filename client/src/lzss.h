



#pragma once

#include <windows.h>

#include "common.h"



struct LzssNode
{
    int parent;
    int smallerChild;
    int largerChild;
};
ASSERT_SIZE(LzssNode, 0xc);


extern LzssNode g_LzssTree[0x2001];

extern BYTE g_LzssWindow[0x2000];




void *LzssDecompress(BYTE *data, int dataSize, BYTE *out, int outSize);

void LzssTreeInitNode(int pos);

void LzssClearWindow();


void LzssReplaceNode(int oldNode, int newNode);

void LzssReplaceChild(int oldNode, int newNode);


int LzssPredecessor(int node);



int LzssInsertNode(int pos, int *matchPosition);

void LzssDeleteNode(int pos);


void *LzssCompress(BYTE *data, int size, int *outSize);
