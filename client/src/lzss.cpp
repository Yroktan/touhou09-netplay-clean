

#include "lzss.h"


#define FETCH_BYTE() \
    { \
        currentByte = *in; \
        if (in - data >= size) \
        { \
            currentByte = 0; \
        } \
        else \
        { \
            in++; \
        } \
        checksum += currentByte; \
    }


#define READ_BIT(bit) \
    { \
        if (mask == 0x80) \
        { \
            FETCH_BYTE(); \
        } \
        bit = currentByte & mask; \
        mask >>= 1; \
        if (mask == 0) \
        { \
            mask = 0x80; \
        } \
    }



#define READ_BITS(value, count) \
    { \
        bitMask = 1 << ((count) - 1); \
        value = 0; \
        while (bitMask != 0) \
        { \
            if (mask == 0x80) \
            { \
                FETCH_BYTE(); \
            } \
            if (currentByte & mask) \
            { \
                value |= bitMask; \
            } \
            bitMask >>= 1; \
            mask >>= 1; \
            if (mask == 0) \
            { \
                mask = 0x80; \
            } \
        } \
    }

void *LzssDecompress(BYTE *data, int dataSize, BYTE *out, int outSize)
{
    int wndPos;
    DWORD bitMask;
    int ch;

    DWORD checksum;
    int len;
    int size;
    DWORD bits;
    BYTE *in;
    BYTE mask;
    int i;

    int position;
    BYTE *dst;
    DWORD currentByte;

    mask = 0x80;
    currentByte = 0;
    checksum = 0;
    size = dataSize;
    if (out == NULL)
    {
        out = (BYTE *)GlobalAlloc(GMEM_FIXED, outSize);
        if (out == NULL)
        {
            return NULL;
        }
    }
    in = data;
    dst = out;
    wndPos = 1;
    for (;;)
    {
        READ_BIT(bits);
        if (bits)
        {
            READ_BITS(bits, 8);
            *dst = bits;
            dst++;
            g_LzssWindow[wndPos] = bits;
            wndPos = (wndPos + 1) & 0x1fff;
        }
        else
        {
            READ_BITS(bits, 13);
            position = bits;
            if (position == 0)
            {
                break;
            }
            READ_BITS(bits, 4);
            len = bits + 2;
            for (i = 0; i <= len; i++)
            {
                ch = g_LzssWindow[(position + i) & 0x1fff];
                *dst = ch;
                dst++;
                g_LzssWindow[wndPos] = ch;
                wndPos = (wndPos + 1) & 0x1fff;
            }
        }
    }
    while (mask != 0x80)
    {
        READ_BIT(bits);
    }
    return out;
}

void LzssTreeInitNode(int pos)
{
    g_LzssTree[0x2000].largerChild = pos;
    g_LzssTree[pos].parent = 0x2000;
    g_LzssTree[pos].largerChild = 0;
    g_LzssTree[pos].smallerChild = 0;
}

void LzssClearWindow()
{
    int i;

    for (i = 0; i < 0x2000; i++)
    {
        g_LzssWindow[i] = 0;
    }
    for (i = 0; i < 0x2001; i++)
    {
        g_LzssTree[i].parent = 0;
        g_LzssTree[i].smallerChild = 0;
        g_LzssTree[i].largerChild = 0;
    }
}

void LzssReplaceNode(int oldNode, int newNode)
{
    g_LzssTree[newNode].parent = g_LzssTree[oldNode].parent;
    if (g_LzssTree[g_LzssTree[oldNode].parent].largerChild == oldNode)
    {
        g_LzssTree[g_LzssTree[oldNode].parent].largerChild = newNode;
    }
    else
    {
        g_LzssTree[g_LzssTree[oldNode].parent].smallerChild = newNode;
    }
    g_LzssTree[oldNode].parent = 0;
}

void LzssReplaceChild(int oldNode, int newNode)
{
    int parent;

    parent = g_LzssTree[oldNode].parent;
    if (g_LzssTree[parent].smallerChild == oldNode)
    {
        g_LzssTree[parent].smallerChild = newNode;
    }
    else
    {
        g_LzssTree[parent].largerChild = newNode;
    }
    g_LzssTree[newNode] = g_LzssTree[oldNode];
    g_LzssTree[g_LzssTree[newNode].smallerChild].parent = newNode;
    g_LzssTree[g_LzssTree[newNode].largerChild].parent = newNode;
    g_LzssTree[oldNode].parent = 0;
}

int LzssPredecessor(int node)
{
    int next;

    next = g_LzssTree[node].smallerChild;
    while (g_LzssTree[next].largerChild != 0)
    {
        next = g_LzssTree[next].largerChild;
    }
    return next;
}

int LzssInsertNode(int pos, int *matchPosition)
{
    int delta;
    int bestLen;
    int test;
    int *child;
    int i;

    if (pos == 0)
    {
        return 0;
    }
    test = g_LzssTree[0x2000].largerChild;
    bestLen = 0;
    for (;;)
    {
        for (i = 0; i < 0x12; i++)
        {
            delta = g_LzssWindow[(pos + i) & 0x1fff] - g_LzssWindow[(test + i) & 0x1fff];
            if (delta != 0)
            {
                break;
            }
        }
        if (i >= bestLen)
        {
            bestLen = i;
            *matchPosition = test;
            if (bestLen >= 0x12)
            {
                LzssReplaceChild(test, pos);
                return bestLen;
            }
        }
        if (delta >= 0)
        {
            child = &g_LzssTree[test].largerChild;
        }
        else
        {
            child = &g_LzssTree[test].smallerChild;
        }
        if (*child == 0)
        {
            *child = pos;
            g_LzssTree[pos].parent = test;
            g_LzssTree[pos].largerChild = 0;
            g_LzssTree[pos].smallerChild = 0;
            return bestLen;
        }
        test = *child;
    }
}

void LzssDeleteNode(int pos)
{
    int replacement;

    if (g_LzssTree[pos].parent == 0)
    {
        return;
    }
    if (g_LzssTree[pos].largerChild == 0)
    {
        LzssReplaceNode(pos, g_LzssTree[pos].smallerChild);
    }
    else if (g_LzssTree[pos].smallerChild == 0)
    {
        LzssReplaceNode(pos, g_LzssTree[pos].largerChild);
    }
    else
    {
        replacement = LzssPredecessor(pos);
        LzssDeleteNode(replacement);
        LzssReplaceChild(pos, replacement);
    }
}


#define GET_BYTE(c) \
    { \
        if (in - data >= size) \
        { \
            c = -1; \
        } \
        else \
        { \
            c = *in; \
            in++; \
        } \
    }


#define PUT_BIT(bit) \
    { \
        if (bit) \
        { \
            currentByte |= mask; \
        } \
        mask >>= 1; \
        if (mask == 0) \
        { \
            *dst = currentByte; \
            dst++; \
            checksum += currentByte; \
            currentByte = 0; \
            mask = 0x80; \
        } \
    }



#define PUT_BITS(value, count) \
    { \
        bitMask = 1 << ((count) - 1); \
        while (bitMask != 0) \
        { \
            PUT_BIT(bitMask & (value)); \
            bitMask >>= 1; \
        } \
    }

void *LzssCompress(BYTE *data, int size, int *outSize)
{
    HGLOBAL hMem;
    DWORD currentByte;

    int position;
    BYTE *dst;
    int replaceLen;
    int i;
    BYTE mask;
    BYTE *in;

    int len;

    DWORD checksum;
    DWORD bitMask;

    int wndPos;
    int ch;
    int lookAheadBytes;

    mask = 0x80;
    currentByte = 0;
    checksum = 0;
    hMem = GlobalAlloc(GMEM_FIXED, size * 2);
    if (hMem == NULL)
    {
        return NULL;
    }
    in = data;
    dst = (BYTE *)hMem;
    *outSize = 0;
    LzssClearWindow();
    wndPos = 1;
    for (i = 0; i < 0x12; i++)
    {
        GET_BYTE(ch);
        if (ch == -1)
        {
            break;
        }
        g_LzssWindow[wndPos + i] = ch;
    }
    lookAheadBytes = i;
    LzssTreeInitNode(wndPos);
    len = 0;
    position = 0;
    while (lookAheadBytes > 0)
    {
        if (len > lookAheadBytes)
        {
            len = lookAheadBytes;
        }
        if (len <= 2)
        {
            replaceLen = 1;
            PUT_BIT(1);
            PUT_BITS(g_LzssWindow[wndPos], 8);
        }
        else
        {
            PUT_BIT(0);
            PUT_BITS(position, 13);
            PUT_BITS(len - 3, 4);
            replaceLen = len;
        }
        for (i = 0; i < replaceLen; i++)
        {
            LzssDeleteNode((wndPos + 0x12) & 0x1fff);
            GET_BYTE(ch);
            if (ch == -1)
            {
                lookAheadBytes--;
            }
            else
            {
                g_LzssWindow[(wndPos + 0x12) & 0x1fff] = ch;
            }
            wndPos = (wndPos + 1) & 0x1fff;
            if (lookAheadBytes != 0)
            {
                len = LzssInsertNode(wndPos, &position);
            }
        }
    }
    PUT_BIT(0);
    PUT_BITS(0, 13);
    *outSize = dst - (BYTE *)hMem;
    return hMem;
}
