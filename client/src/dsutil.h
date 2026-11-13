



#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#include "common.h"

#define WAVEFILE_READ 1

struct BgmFormat;
class CStreamingSound;

class CWaveFile
{
public:
    CWaveFile();
    ~CWaveFile();



    HRESULT Open(LPTSTR strFileName, BgmFormat *format, DWORD dwFlags);


    HRESULT SwitchTrack(BgmFormat *format);


    HRESULT OpenFromMemory(BYTE *pbData, ULONG ulDataSize, BgmFormat *format, DWORD dwFlags);
    DWORD GetSize();



    HRESULT ResetFile(bool bLoop);
    HRESULT Read(BYTE *pBuffer, DWORD dwSizeToRead, DWORD *pdwSizeRead);

    HRESULT Close();

    BgmFormat *GetFormat()
    {
        return this->format;
    }



    WAVEFORMATEX *m_pwfx;
    MMCKINFO m_ck;
    MMCKINFO m_ckRiff;
    DWORD m_dwSize;
    MMIOINFO m_mmioinfoOut;
    DWORD m_dwFlags;
    BOOL m_bIsReadingFromMemory;
    BYTE *m_pbData;
    BYTE *m_pbDataCur;
    ULONG m_ulDataSize;

    HANDLE hFile;

    BgmFormat *format;
};
ASSERT_SIZE(CWaveFile, 0x94);

class CSoundManager
{
public:
    CSoundManager();
    ~CSoundManager();

    HRESULT Initialize(HWND hWnd, DWORD dwCoopLevel, DWORD dwPrimaryChannels, DWORD dwPrimaryFreq,
                       DWORD dwPrimaryBitRate);
    HRESULT SetPrimaryBufferFormat(DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate);


    HRESULT CreateStreaming(CStreamingSound **ppStreamingSound, LPTSTR strWaveFileName, DWORD dwCreationFlags,
                            GUID guid3DAlgorithm, DWORD dwNotifyCount, DWORD dwNotifySize, HANDLE hNotifyEvent,
                            BgmFormat *format);


    HRESULT CreateStreamingFromMemory(CStreamingSound **ppStreamingSound, void *data, DWORD size, BgmFormat *format,
                       DWORD dwCreationFlags, GUID guid3DAlgorithm, DWORD dwNotifyCount, DWORD dwNotifySize,
                       HANDLE hNotifyEvent);

    inline LPDIRECTSOUND8 GetDirectSound()
    {
        return m_pDS;
    }

    LPDIRECTSOUND8 m_pDS;
};

class CSound
{
public:
    CSound(LPDIRECTSOUNDBUFFER *apDSBuffer, DWORD dwDSBufferSize, DWORD dwNumBuffers, CWaveFile *pWaveFile);
    virtual ~CSound();

    HRESULT FillBufferWithSound(LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger);
    HRESULT RestoreBuffer(LPDIRECTSOUNDBUFFER pDSB, BOOL *pbWasRestored);
    LPDIRECTSOUNDBUFFER GetFreeBuffer();
    LPDIRECTSOUNDBUFFER GetBuffer(DWORD dwIndex);
    HRESULT Play(DWORD dwPriority, DWORD dwFlags);





    HRESULT SetVolume(LONG lVolume);
    HRESULT Stop();

    HRESULT PauseBuffer();

    HRESULT ResumeBuffer();
    HRESULT Reset();

    CWaveFile *GetWaveFile()
    {
        return this->m_pWaveFile;
    }

    LPDIRECTSOUNDBUFFER *m_apDSBuffer;
    DWORD m_dwDSBufferSize;
    CWaveFile *m_pWaveFile;
    DWORD m_dwNumBuffers;



    int fadeFrames;
    int fadeTotal;
    int fading;

    DWORD dwPlayPriority;
    DWORD dwPlayFlags;
    DWORD pad_28;

    DWORD clearedOnPlay;


    BOOL played;


    BOOL isPlaying;
};

class CStreamingSound : public CSound
{
public:
    CStreamingSound(LPDIRECTSOUNDBUFFER pDSBuffer, DWORD dwDSBufferSize, CWaveFile *pWaveFile, DWORD dwNotifySize);
    ~CStreamingSound();






    int UpdateFade();

    int UpdateFadeIn();

    int UpdateFadeUp();

    int UpdateFadeDown();


    void FadeOut(float seconds)
    {
        this->fading = 1;
        this->fadeFrames = seconds * 60.0f;
        this->fadeTotal = this->fadeFrames;
    }

    void FadeIn(float seconds)
    {
        this->fading = 2;
        this->fadeFrames = seconds * 60.0f;
        this->fadeTotal = this->fadeFrames;
        this->SetVolume(-10000);
    }

    void FadeDown(float seconds)
    {
        this->fading = 4;
        this->fadeFrames = seconds * 60.0f;
        this->fadeTotal = this->fadeFrames;
    }

    void FadeUp(float seconds)
    {
        this->fading = 3;
        this->fadeFrames = seconds * 60.0f;
        this->fadeTotal = this->fadeFrames;
        this->SetVolume(-1000);
    }



    HRESULT RecreateBuffers();

    HRESULT HandleWaveStreamNotification(BOOL bLoopedPlay);
    HRESULT Reset();



    DSBUFFERDESC dsbd;
    CSoundManager *manager;
    DWORD m_dwLastPlayPos;
    DWORD m_dwPlayProgress;
    DWORD m_dwNextWriteOffset;
    BOOL m_bFillNextNotificationWithSilence;
    DWORD m_dwNotifySize;

    HANDLE hNotifyEvent;

    int isBuffering;
};
