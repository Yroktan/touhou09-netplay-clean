



#pragma once

#include <windows.h>

#include "common.h"




typedef int (*ChainCallback)(void *arg);


struct ChainElem
{

    ChainElem();


    ~ChainElem();




    void SetCallback(ChainCallback callback)
    {
        this->callback = callback;
        this->addedCallback = NULL;
        this->deletedCallback = NULL;
    }


    short priority;

    unsigned short allocated : 1;
    ChainCallback callback;


    ChainCallback addedCallback;
    ChainCallback deletedCallback;
    ChainElem *prev;
    ChainElem *next;


    ChainElem *self;
    void *arg;
};
ASSERT_SIZE(ChainElem, 0x20);


class Chain
{
public:

    Chain();

    ~Chain();




    int AddToCalcChain(ChainElem *elem, int priority);

    int AddToDrawChain(ChainElem *elem, int priority);

    ChainElem *CreateElem(ChainCallback callback);



    void RemoveLocked(ChainElem *elem);





    int RunCalcChain();


    int RunDrawChain();

    void Remove(ChainElem *elem);


    void ReleaseList(ChainElem *root);

    void Release();

    ChainElem calcChain;
    ChainElem drawChain;
};
ASSERT_SIZE(Chain, 0x40);
extern Chain g_Chain;
