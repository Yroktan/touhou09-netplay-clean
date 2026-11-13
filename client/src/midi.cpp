
#include "midi.h"
#include "unsorted.h"

MidiDevice::MidiDevice()
{
    this->handle = 0;
    this->deviceId = 0;
}

int MidiDevice::Close()
{
    if (this->handle == 0)
    {
        return -1;
    }
    midiOutReset(this->handle);
    midiOutClose(this->handle);
    this->handle = 0;
    return 0;
}

int MidiDevice::SendLongMsg(LPMIDIHDR header)
{
    if (this->handle == 0)
    {
        return 0;
    }
    if (midiOutPrepareHeader(this->handle, header, sizeof(MIDIHDR)) != MMSYSERR_NOERROR)
    {
        return 1;
    }
    return midiOutLongMsg(this->handle, header, sizeof(MIDIHDR)) != MMSYSERR_NOERROR;
}

int MidiDevice::SendShortMsg(BYTE status, BYTE data1, BYTE data2)
{
    MidiShortMsg msg;

    if (this->handle == 0)
    {
        return 0;
    }
    msg.bytes[0] = status;
    msg.bytes[1] = data1;
    msg.bytes[2] = data2;
    return midiOutShortMsg(this->handle, msg.dword) != MMSYSERR_NOERROR;
}

MmTimer::MmTimer()
{
    timeGetDevCaps(&this->caps, sizeof(this->caps));
    this->timerId = 0;
}


void MmTimer::OnTimer()
{
}

int MmTimer::Stop()
{
    if (this->timerId != 0)
    {
        timeKillEvent(this->timerId);
    }
    timeEndPeriod(this->caps.wPeriodMin);
    this->timerId = 0;
    return 1;
}

void CALLBACK MmTimer::DefaultCallback(UINT id, UINT msg, DWORD_PTR user, DWORD_PTR dw1, DWORD_PTR dw2)
{
    MmTimer *timer = (MmTimer *)user;
    timer->OnTimer();
}

WORD SwapBytes16(WORD value)
{
    union
    {
        WORD word;
        BYTE bytes[2];
    } swapped;

    swapped.bytes[0] = ((BYTE *)&value)[1];
    swapped.bytes[1] = ((BYTE *)&value)[0];
    return swapped.word;
}

DWORD SwapBytes32(DWORD value)
{
    union
    {
        DWORD dword;
        BYTE bytes[4];
    } swapped;

    swapped.bytes[0] = ((BYTE *)&value)[3];
    swapped.bytes[1] = ((BYTE *)&value)[2];
    swapped.bytes[2] = ((BYTE *)&value)[1];
    swapped.bytes[3] = ((BYTE *)&value)[0];
    return swapped.dword;
}

DWORD ReadVarLen(BYTE **cursor)
{
    DWORD value = 0;
    BYTE byte;

    do
    {
        byte = **cursor;
        (*cursor)++;
        value = (value << 7) + (byte & 0x7f);
    } while (byte & 0x80);
    return value;
}

void MidiPlayer::Rewind()
{
    int i;
    MidiTrack *track;

    track = this->tracks;
    this->volumeScale = 1.0f;
    this->unk_2dc = 0;
    this->fading = 0;
    this->ticks = 0;
    this->baseTime = 0;
    for (i = 0; i < this->trackCount; i++, track++)
    {
        track->cursor = track->data;
        track->loopCursor = track->cursor;
        track->active = 1;
        track->delta = ReadVarLen(&track->cursor);
    }
}

int MidiPlayer::FadeOut(int duration)
{
    this->volumeScale = 0.0f;
    this->fadeDuration = duration;
    this->fadeElapsed = 0;
    this->unk_2dc = 0;
    this->fading = 1;
    return 0;
}

void MidiPlayer::SendVolume(int offset)
{
    int cc;
    int i;
    int chStatus;
    int data2;
    int volume;

    if (this->suppressVolume != 0)
    {
        return;
    }
    cc = 7;
    for (i = 0; i < 0x10; i++)
    {
        chStatus = (BYTE)(i + 0xb0);
        volume = (int)(this->channels[i].volume * this->volumeScale) + offset;
        if (volume < 0)
        {
            volume = 0;
        }
        else if (volume > 0x7f)
        {
            volume = 0x7f;
        }
        data2 = (BYTE)volume;
        this->device.SendShortMsg(chStatus, cc, data2);
    }
}

void MidiTimer::OnTimer()
{
    QueryPerformanceCounter(&g_MidiTimerLastCounter);
}

void MidiTimer::StopTimer()
{
    this->Stop();
}

MidiDevice::~MidiDevice()
{
    this->Close();
}

int MidiDevice::Open(UINT deviceId)
{
    if (this->handle != 0)
    {
        if (this->deviceId != deviceId)
        {
            this->Close();
        }
        else
        {
            return 0;
        }
    }
    this->deviceId = deviceId;
    return midiOutOpen(&this->handle, deviceId, (DWORD_PTR)g_App.hwnd, 0, CALLBACK_WINDOW) != MMSYSERR_NOERROR;
}

MmTimer::~MmTimer()
{
    this->Stop();
    timeEndPeriod(this->caps.wPeriodMin);
}

UINT MmTimer::Start(UINT delay, LPTIMECALLBACK callback, DWORD_PTR user)
{
    this->Stop();
    timeBeginPeriod(this->caps.wPeriodMin);
    if (callback != NULL)
    {
        this->timerId = timeSetEvent(delay, this->caps.wPeriodMin, callback, user, TIME_PERIODIC);
    }
    else
    {
        this->timerId = timeSetEvent(delay, this->caps.wPeriodMin, DefaultCallback, (DWORD_PTR)this, TIME_PERIODIC);
    }
    return this->timerId;
}

MidiPlayer::MidiPlayer()
{
    int i;
    int j;

    this->tracks = NULL;
    this->division = 0;
    this->tempo = 0;
    this->trackCount = 0;
    this->transpose = 0;
    this->volumeScale = 0.0f;
    this->lastVolume = 0;
    this->unk_2d0 = 0;
    this->suppressVolume = 0;
    this->unk_2d8 = 0;
    this->unk_2dc = 0;
    this->fading = 0;
    for (i = 0; i < 0x20; i++)
    {
        this->fileData[i] = NULL;
    }
    for (j = 0; j < 0x20; j++)
    {
        this->headers[j] = NULL;
    }
    this->nextHeader = 0;
    this->playingSlot = -1;
}

void MidiPlayer::UnloadFile(int slot)
{
    g_MemoryTracker.Free(this->fileData[slot]);
    this->fileData[slot] = NULL;
}

void MidiPlayer::FreeTracks()
{
    int i;

    for (i = 0; i < this->trackCount; i++)
    {
        g_MemoryTracker.Free(this->tracks[i].data);
    }
    g_MemoryTracker.Free(this->tracks);
    this->tracks = NULL;
    this->trackCount = 0;
}

int MidiPlayer::ParseFile(int slot)
{
    int i;
    BYTE *cursor;
    BYTE *file;
    BYTE *track;
    DWORD headerLength;
    MidiChunk hdr;
    BYTE *header;
    DWORD trackLen;

    this->FreeTracks();
    cursor = (BYTE *)this->fileData[slot];
    file = cursor;
    if (cursor == NULL)
    {
        return -1;
    }
    memcpy(&hdr, cursor, sizeof(MidiChunk));
    cursor += sizeof(MidiChunk);
    headerLength = SwapBytes32(hdr.length);
    header = cursor;
    cursor += headerLength;
    this->format = SwapBytes16(*(WORD *)header);
    this->division = SwapBytes16(*(WORD *)(header + 4));
    this->trackCount = SwapBytes16(*(WORD *)(header + 2));
    this->tracks = (MidiTrack *)g_MemoryTracker.Alloc(this->trackCount * sizeof(MidiTrack), "midi");
    memset(this->tracks, 0, this->trackCount * sizeof(MidiTrack));
    for (i = 0; i < this->trackCount; i++)
    {
        track = cursor;
        cursor += sizeof(MidiChunk);
        trackLen = SwapBytes32(((MidiChunk *)track)->length);
        this->tracks[i].length = trackLen;
        this->tracks[i].data = (BYTE *)g_MemoryTracker.Alloc(trackLen, "midi");
        this->tracks[i].active = 1;
        memcpy(this->tracks[i].data, cursor, trackLen);
        cursor += trackLen;
    }
    this->tempo = 1000000;
    this->playingSlot = slot;
    return 0;
}

int MidiPlayer::Play()
{
    if (this->tracks == NULL)
    {
        return -1;
    }
    this->Rewind();
    this->device.Open(MIDI_MAPPER);
    this->Start(1, NULL, 0);
    return 0;
}

int MidiPlayer::ReleaseHeader(LPMIDIHDR header)
{
    int i;

    for (i = 0; i < 0x20; i++)
    {
        if (this->headers[i] == header)
        {
            goto found;
        }
    }
    return -1;

found:
    this->headers[i] = NULL;
    midiOutUnprepareHeader(this->device.handle, header, sizeof(MIDIHDR));
    g_MemoryTracker.Free(header->lpData);
    g_MemoryTracker.Free(header);
    return 0;
}

void MidiPlayer::ProcessEvent(MidiTrack *track)
{
    int deltaTime;
    int i;
    BYTE byte2;
    int newVolume;
    BYTE channel;
    BYTE event;
    BYTE byte1;
    BYTE runStatus;
    int numBytes;

    runStatus = *track->cursor;
    if (runStatus < 0x80)
    {
        runStatus = track->runningStatus;
    }
    else
    {
        track->cursor++;
    }
    event = runStatus & 0xf0;
    channel = runStatus & 0x0f;
    switch (event)
    {
    case 0xf0:
        if (runStatus == 0xf0)
        {
            LPMIDIHDR header;

            if (this->headers[this->nextHeader] != NULL)
            {
                this->ReleaseHeader(this->headers[this->nextHeader]);
            }
            this->headers[this->nextHeader] = (LPMIDIHDR)g_MemoryTracker.Alloc(sizeof(MIDIHDR), "midiHDR");
            header = this->headers[this->nextHeader];
            numBytes = ReadVarLen(&track->cursor);
            memset(header, 0, sizeof(MIDIHDR));
            header->lpData = (LPSTR)g_MemoryTracker.Alloc(numBytes + 1, "midiHDR->lpData");
            header->lpData[0] = 0xf0;
            header->dwFlags = 0;
            header->dwBufferLength = numBytes + 1;
            for (i = 0; i < numBytes; i++)
            {
                header->lpData[i + 1] = *track->cursor;
                track->cursor++;
            }
            if (this->device.SendLongMsg(header) != 0)
            {
                g_MemoryTracker.Free(header->lpData);
                g_MemoryTracker.Free(header);
                this->headers[this->nextHeader] = NULL;
            }
            this->nextHeader++;
            this->nextHeader %= 0x20;
        }
        else if (runStatus == 0xff)
        {
            BYTE metaEvent;
            int bpm;

            metaEvent = *track->cursor;
            track->cursor++;
            numBytes = ReadVarLen(&track->cursor);
            if (metaEvent == 0x2f)
            {

                track->active = 0;
                return;
            }
            if (metaEvent == 0x51)
            {

                this->baseTime += this->ticks * this->division * 1000 / this->tempo;
                this->ticks = 0;
                this->tempo = 0;


                for (i = 0; i < numBytes; i++)
                {
                    this->tempo = (this->tempo << 8) + *track->cursor + this->tempo;
                    track->cursor++;
                }

                bpm = 60000000 / this->tempo;
                break;
            }
            track->cursor += numBytes;
        }
        break;
    case 0x80:
    case 0x90:
    case 0xa0:
    case 0xb0:
    case 0xe0:
        byte1 = *track->cursor;
        track->cursor++;
        byte2 = *track->cursor;
        track->cursor++;
        break;
    case 0xc0:
    case 0xd0:
        byte1 = *track->cursor;
        track->cursor++;
        byte2 = 0;
        break;
    }
    switch (event)
    {
    case 0x90:
        if (byte2 != 0)
        {
            byte1 += this->transpose;
            this->channels[channel].notes[byte1 >> 3] |= (BYTE)(1 << (byte1 & 7));
            break;
        }

    case 0x80:
        byte1 += this->transpose;
        this->channels[channel].notes[byte1 >> 3] &= (BYTE)~(1 << (byte1 & 7));
        break;
    case 0xc0:
        this->channels[channel].program = byte1;
        break;
    case 0xb0:
        switch (byte1)
        {
        case 0:
            this->channels[channel].bank = byte2;
            break;
        case 7:
            this->channels[channel].volume = byte2;
            newVolume = (float)byte2 * this->volumeScale;
            if (newVolume < 0)
            {
                newVolume = 0;
            }
            else if (newVolume > 0x7f)
            {
                newVolume = 0x7f;
            }
            this->channels[channel].scaledVolume = newVolume;
            byte2 = newVolume;
            break;
        case 91:
            this->channels[channel].reverb = byte2;
            break;
        case 93:
            this->channels[channel].chorus = byte2;
            break;
        case 10:
            this->channels[channel].pan = byte2;
            break;
        case 2:
        {

            MidiTrack *saved;

            saved = this->tracks;
            for (i = 0; i < this->trackCount; i++, saved++)
            {
                saved->loopCursor = saved->cursor;
                saved->loopDelta = saved->delta;
            }
            this->loopTempo = this->tempo;
            this->loopTicks = this->ticks;
            this->loopBaseTime = this->baseTime;
            break;
        }
        case 4:
        {

            MidiTrack *restored;

            restored = this->tracks;
            for (i = 0; i < this->trackCount; i++, restored++)
            {
                restored->cursor = restored->loopCursor;
                restored->delta = restored->loopDelta;
            }
            this->tempo = this->loopTempo;
            this->ticks = this->loopTicks;
            this->baseTime = this->loopBaseTime;
            break;
        }
        }
        break;
    }
    if (runStatus < 0xf0)
    {
        this->device.SendShortMsg(runStatus, byte1, byte2);
    }
    track->runningStatus = runStatus;
    deltaTime = ReadVarLen(&track->cursor);
    track->delta += deltaTime;
}

void MidiTimer::StartTimer()
{
    this->Start(16, NULL, 0);
}

int MidiPlayer::Stop()
{
    int i;

    if (this->tracks == NULL)
    {
        return -1;
    }
    for (i = 0; i < 0x20; i++)
    {
        if (this->headers[i] != NULL)
        {
            this->ReleaseHeader(this->headers[i]);
        }
    }
    MmTimer::Stop();
    this->device.Close();
    this->playingSlot = -1;
    return 0;
}

void MidiPlayer::OnTimer()
{
    int i;
    unsigned __int64 time;
    int anyPlaying;

    anyPlaying = 0;
    time = this->baseTime + this->ticks * this->division * 1000 / this->tempo;
    if (this->fading != 0)
    {
        if (this->fadeElapsed < this->fadeDuration)
        {
            this->volumeScale = 1.0f - (float)this->fadeElapsed / (float)this->fadeDuration;
            if ((int)(this->volumeScale * 128.0f) != this->lastVolume)
            {
                this->SendVolume(0);
            }
            this->lastVolume = (int)(this->volumeScale * 128.0f);
            this->fadeElapsed++;
        }
        else
        {
            this->volumeScale = 0.0f;
            return;
        }
    }
    for (i = 0; i < this->trackCount; i++)
    {
        if (this->tracks[i].active)
        {
            anyPlaying = 1;
            while (this->tracks[i].active && this->tracks[i].delta <= time)
            {
                this->ProcessEvent(&this->tracks[i]);
                time = this->baseTime + this->ticks * this->division * 1000 / this->tempo;
            }
        }
    }
    this->ticks++;
    if (!anyPlaying)
    {
        this->Rewind();
    }
}

MidiPlayer::~MidiPlayer()
{
    int i;

    this->Stop();
    this->FreeTracks();
    for (i = 0; i < 0x20; i++)
    {
        this->UnloadFile(i);
    }
}

int MidiPlayer::LoadFile(int slot, char *path)
{
    if (this->playingSlot == slot)
    {
        this->Stop();
    }
    this->UnloadFile(slot);
    this->fileData[slot] = LoadFileData(path, NULL, 0);
    if (this->fileData[slot] == NULL)
    {

        LogPrintf(&g_Logger, "error : MIDI File が読み込めない %s \rv\r\n", path);
        return -1;
    }
    return 0;
}

int MidiPlayer::Load(char *path)
{
    if (this->LoadFile(0x1f, path) != 0)
    {
        return -1;
    }
    this->ParseFile(0x1f);
    this->UnloadFile(0x1f);
    return 0;
}
