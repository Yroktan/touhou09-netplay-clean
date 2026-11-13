

#pragma once

#include <windows.h>

#include "common.h"
#include "unsorted.h"

struct Timer
{


    Timer()
    {
        this->Initialize();
    }


    void Initialize()
    {
        this->current = 0;
        this->previous = -999999;
        this->currentF = 0.0f;
    }


    int NextTick()
    {
        this->previous = this->current;
        g_App.TickTimer(&this->current, &this->currentF);
        return this->current;
    }

    void Set(int frames)
    {
        this->current = frames;
        this->previous = -999999;
        this->currentF = (float)frames;
    }



    void Add(float frames);

    void Sub(float frames)
    {
        this->Add(-frames);
    }

    void Add(int frames)
    {
        this->Add((float)frames);
    }

    void Sub(int frames)
    {
        this->Add(-frames);
    }

    void operator+=(int frames)
    {
        this->Add(frames);
    }
    void operator+=(float frames)
    {
        this->Add(frames);
    }
    void operator-=(float frames)
    {
        this->Sub(frames);
    }
    void operator-=(int frames)
    {
        this->Sub(frames);
    }

    void operator--(int)
    {
        this->Sub(1);
    }

    BOOL HasTicked()
    {
        return this->current != this->previous;
    }


    BOOL HasTickedEvery(int n)
    {
        return this->current != this->previous && this->current % n == 0;
    }


    int AsFrames()
    {
        return this->current;
    }

    float AsFloat()
    {
        return this->currentF;
    }

    void operator=(int frames)
    {
        this->Set(frames);
    }

    void operator++(int)
    {
        this->NextTick();
    }
    BOOL operator>=(int frames)
    {
        return this->current >= frames;
    }
    BOOL operator<(int frames)
    {
        return this->current < frames;
    }

    BOOL operator>(int frames)
    {
        return this->current > frames;
    }

    BOOL operator==(int frames)
    {
        return this->current == frames;
    }

    BOOL operator<=(int frames)
    {
        return this->current <= frames;
    }

    int operator%(int n)
    {
        return this->current % n;
    }


    int previous;

    float currentF;
    int current;
};
ASSERT_SIZE(Timer, 0xc);
