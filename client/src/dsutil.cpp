


#include "dsutil.h"
#include "sound.h"
#include "unsorted.h"


#define SAFE_DELETE(p) { if(p) { delete (p); (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p); (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#define DXTRACE_ERR(str, hr) (hr)

CSoundManager::CSoundManager()
{
    m_pDS = NULL;
}

CSoundManager::~CSoundManager()
{
    SAFE_RELEASE(m_pDS);
}

HRESULT CSoundManager::SetPrimaryBufferFormat(DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate)
{
    HRESULT hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    if (m_pDS == NULL)
        return CO_E_NOTINITIALIZED;


    DSBUFFERDESC dsbd;
    ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat = NULL;

    if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &pDSBPrimary, NULL)))
        return DXTRACE_ERR(TEXT("CreateSoundBuffer"), hr);

    WAVEFORMATEX wfx;
    ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = (WORD)dwPrimaryChannels;
    wfx.nSamplesPerSec = dwPrimaryFreq;
    wfx.wBitsPerSample = (WORD)dwPrimaryBitRate;
    wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    if (FAILED(hr = pDSBPrimary->SetFormat(&wfx)))
        return DXTRACE_ERR(TEXT("SetFormat"), hr);

    SAFE_RELEASE(pDSBPrimary);

    return S_OK;
}

HRESULT CStreamingSound::RecreateBuffers()
{
    DWORD i;
    DSBPOSITIONNOTIFY *aPosNotify;
    LPDIRECTSOUNDNOTIFY pNotify;
    DWORD j;

    this->isPlaying = 0;
    for (i = 0; i < m_dwNumBuffers; i++)
    {
        SAFE_RELEASE(m_apDSBuffer[i]);
    }
    SAFE_DELETE_ARRAY(m_apDSBuffer);

    aPosNotify = NULL;
    pNotify = NULL;
    m_apDSBuffer = new LPDIRECTSOUNDBUFFER[m_dwNumBuffers];
    for (i = 0; i < m_dwNumBuffers; i++)
    {
        if (FAILED(this->manager->m_pDS->CreateSoundBuffer(&this->dsbd, &m_apDSBuffer[i], NULL)))
            return E_FAIL;

        if (FAILED(m_apDSBuffer[i]->QueryInterface(IID_IDirectSoundNotify, (VOID **)&pNotify)))
            return E_FAIL;

        aPosNotify = new DSBPOSITIONNOTIFY[16];
        if (aPosNotify == NULL)
            return E_OUTOFMEMORY;

        for (j = 0; j < 16; j++)
        {
            aPosNotify[j].dwOffset = (m_dwNotifySize * j) + m_dwNotifySize - 1;
            aPosNotify[j].hEventNotify = this->hNotifyEvent;
        }

        if (FAILED(pNotify->SetNotificationPositions(16, aPosNotify)))
        {
            SAFE_RELEASE(pNotify);
            SAFE_DELETE(aPosNotify);
            return E_FAIL;
        }

        SAFE_RELEASE(pNotify);
        SAFE_DELETE(aPosNotify);
    }
    return S_OK;
}

HRESULT CSound::RestoreBuffer(LPDIRECTSOUNDBUFFER pDSB, BOOL *pbWasRestored)
{
    HRESULT hr;

    if (pDSB == NULL)
        return CO_E_NOTINITIALIZED;
    if (pbWasRestored)
        *pbWasRestored = FALSE;

    DWORD dwStatus;
    if (FAILED(hr = pDSB->GetStatus(&dwStatus)))
        return DXTRACE_ERR(TEXT("GetStatus"), hr);

    if (dwStatus & DSBSTATUS_BUFFERLOST)
    {




        do
        {
            hr = pDSB->Restore();
            if (hr == DSERR_BUFFERLOST)
                Sleep(10);
        } while (hr = pDSB->Restore());

        if (pbWasRestored != NULL)
            *pbWasRestored = TRUE;

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

LPDIRECTSOUNDBUFFER CSound::GetFreeBuffer()
{
    BOOL bIsPlaying = FALSE;
    DWORD i;

    if (m_apDSBuffer == NULL)
        return FALSE;

    for (i = 0; i < m_dwNumBuffers; i++)
    {
        if (m_apDSBuffer[i])
        {
            DWORD dwStatus = 0;
            m_apDSBuffer[i]->GetStatus(&dwStatus);
            if ((dwStatus & DSBSTATUS_PLAYING) == 0)
                break;
        }
    }

    if (i != m_dwNumBuffers)
        return m_apDSBuffer[i];
    else
        return m_apDSBuffer[rand() % m_dwNumBuffers];
}

LPDIRECTSOUNDBUFFER CSound::GetBuffer(DWORD dwIndex)
{
    if (m_apDSBuffer == NULL)
        return NULL;
    if (dwIndex >= m_dwNumBuffers)
        return NULL;

    return m_apDSBuffer[dwIndex];
}

HRESULT CSound::SetVolume(LONG lVolume)
{
    float scale = g_SoundPlayer.bgmVolume / 100.0f;

    if (g_SoundPlayer.bgmVolume != 0)
    {
        scale = (scale = 1.0f - scale) * scale;
        scale = 1.0f - scale;
        return m_apDSBuffer[0]->SetVolume((LONG)((lVolume + 5000) * scale) - 5000);
    }
    else
    {
        return m_apDSBuffer[0]->SetVolume(DSBVOLUME_MIN);
    }
}

HRESULT CSound::Stop()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    HRESULT hr = 0;

    this->isPlaying = 0;
    this->played = 0;
    for (DWORD i = 0; i < m_dwNumBuffers; i++)
    {
        hr |= m_apDSBuffer[i]->Stop();
        hr |= m_apDSBuffer[i]->SetCurrentPosition(0);
    }
    this->fading = 0;

    return hr;
}

HRESULT CSound::PauseBuffer()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    if (!this->played)
        return S_OK;

    HRESULT hr = 0;

    this->isPlaying = 0;
    hr |= m_apDSBuffer[0]->Stop();

    return hr;
}

HRESULT CSound::ResumeBuffer()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;
    if (!this->played)
        return S_OK;

    LPDIRECTSOUNDBUFFER pDSB = m_apDSBuffer[0];

    this->isPlaying = 1;
    return pDSB->Play(0, this->dwPlayPriority, this->dwPlayFlags);
}

HRESULT CSound::Reset()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    HRESULT hr = 0;

    for (DWORD i = 0; i < m_dwNumBuffers; i++)
        hr |= m_apDSBuffer[i]->SetCurrentPosition(0);

    return hr;
}

int CStreamingSound::UpdateFade()
{
    LONG lVolume;
    HRESULT hr;

    if (this->fading == 1)
    {
        this->fadeFrames--;
        if (this->fadeFrames <= 0)
        {
            this->fading = 0;
            m_apDSBuffer[0]->Stop();
            return 1;
        }
        lVolume = this->fadeFrames * 5000 / this->fadeTotal - 5000;
        hr = SetVolume(lVolume);
    }
    return 0;
}

int CStreamingSound::UpdateFadeIn()
{
    LONG lVolume;
    HRESULT hr;

    if (this->fading == 2)
    {
        this->fadeFrames--;
        if (this->fadeFrames <= 0)
        {
            this->fading = 0;
            return 1;
        }
        lVolume = 0 - this->fadeFrames * 5000 / this->fadeTotal;
        hr = SetVolume(lVolume);
    }
    return 0;
}

int CStreamingSound::UpdateFadeUp()
{
    LONG lVolume;
    HRESULT hr;

    if (this->fading == 3)
    {
        this->fadeFrames--;
        if (this->fadeFrames <= 0)
        {
            this->fading = 0;
            return 1;
        }
        lVolume = 0 - this->fadeFrames * 1000 / this->fadeTotal;
        hr = SetVolume(lVolume);
    }
    return 0;
}

int CStreamingSound::UpdateFadeDown()
{
    LONG lVolume;
    HRESULT hr;

    if (this->fading == 4)
    {
        this->fadeFrames--;
        if (this->fadeFrames <= 0)
        {
            this->fading = 0;
            return 1;
        }
        lVolume = this->fadeFrames * 1000 / this->fadeTotal - 1000;
        hr = SetVolume(lVolume);
    }
    return 0;
}

CWaveFile::CWaveFile()
{
    this->format = NULL;
    m_pwfx = NULL;
    m_dwSize = 0;
    m_bIsReadingFromMemory = FALSE;
}

HRESULT CWaveFile::OpenFromMemory(BYTE *pbData, ULONG ulDataSize, BgmFormat *format, DWORD dwFlags)
{
    this->format = format;
    m_ulDataSize = ulDataSize;
    m_pbData = pbData;
    m_pbDataCur = m_pbData;
    m_bIsReadingFromMemory = TRUE;

    if (dwFlags != WAVEFILE_READ)
        return E_NOTIMPL;

    return S_OK;
}

DWORD CWaveFile::GetSize()
{
    return m_dwSize;
}

HRESULT CWaveFile::ResetFile(bool bLoop)
{
    DWORD dwPos;

    if (m_bIsReadingFromMemory)
    {
        m_pbDataCur = m_pbData;
        if (this->format->loopEnd > 0)
        {
            m_ulDataSize = this->format->loopEnd;
        }
        if (bLoop && this->format->loopStart > 0)
        {
            m_pbDataCur += this->format->loopStart;
        }
    }
    else
    {
        if (this->hFile == NULL)
            return CO_E_NOTINITIALIZED;


        if (bLoop && this->format->loopStart > 0)
        {
            dwPos = SetFilePointer(this->hFile, g_SoundPlayer.bgmFileOffset + this->format->loopStart + this->format->offset,
                                   NULL, FILE_BEGIN);
            m_ck.cksize = this->format->loopEnd - this->format->loopStart;
        }
        else
        {
            dwPos = SetFilePointer(this->hFile, g_SoundPlayer.bgmFileOffset + this->format->offset, NULL, FILE_BEGIN);
            m_ck.cksize = this->format->loopEnd;
        }
    }

    return S_OK;
}

HRESULT CWaveFile::Read(BYTE *pBuffer, DWORD dwSizeToRead, DWORD *pdwSizeRead)
{
    if (m_bIsReadingFromMemory)
    {
        if (m_pbDataCur == NULL)
            return CO_E_NOTINITIALIZED;
        if (pdwSizeRead != NULL)
            *pdwSizeRead = 0;

        if ((BYTE *)(m_pbDataCur + dwSizeToRead) > (BYTE *)(m_pbData + m_ulDataSize))
        {
            dwSizeToRead = m_ulDataSize - (DWORD)(m_pbDataCur - m_pbData);
        }

        CopyMemory(pBuffer, m_pbDataCur, dwSizeToRead);
        m_pbDataCur += dwSizeToRead;

        if (pdwSizeRead != NULL)
            *pdwSizeRead = dwSizeToRead;

        return S_OK;
    }
    else
    {
        DWORD nRead;

        if (this->hFile == NULL)
            return CO_E_NOTINITIALIZED;
        if (pBuffer == NULL || pdwSizeRead == NULL)
            return E_INVALIDARG;

        UINT cbDataIn = dwSizeToRead;
        if (cbDataIn > m_ck.cksize)
            cbDataIn = m_ck.cksize;

        m_ck.cksize -= cbDataIn;

        ReadFile(this->hFile, pBuffer, cbDataIn, &nRead, NULL);

        if (pdwSizeRead != NULL)
            *pdwSizeRead = nRead;

        return S_OK;
    }
}

HRESULT CWaveFile::Close()
{
    if (m_dwFlags == WAVEFILE_READ)
    {
        CloseHandle(this->hFile);
        this->hFile = INVALID_HANDLE_VALUE;
    }

    return S_OK;
}

HRESULT CSoundManager::Initialize(HWND hWnd, DWORD dwCoopLevel, DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate)
{
    HRESULT hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    SAFE_RELEASE(m_pDS);


    if (FAILED(hr = DirectSoundCreate8(NULL, &m_pDS, NULL)))
        return DXTRACE_ERR(TEXT("DirectSoundCreate8"), hr);


    if (FAILED(hr = m_pDS->SetCooperativeLevel(hWnd, dwCoopLevel)))
        return DXTRACE_ERR(TEXT("SetCooperativeLevel"), hr);


    SetPrimaryBufferFormat(dwPrimaryChannels, dwPrimaryFreq, dwPrimaryBitRate);

    return S_OK;
}

HRESULT CSound::FillBufferWithSound(LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger)
{
    HRESULT hr;
    VOID *pDSLockedBuffer = NULL;
    DWORD dwDSLockedBufferSize = 0;
    DWORD dwWavDataRead = 0;

    if (pDSB == NULL)
        return CO_E_NOTINITIALIZED;



    if (FAILED(hr = RestoreBuffer(pDSB, NULL)))
        return DXTRACE_ERR(TEXT("RestoreBuffer"), hr);


    if (FAILED(hr = pDSB->Lock(0, m_dwDSBufferSize, &pDSLockedBuffer, &dwDSLockedBufferSize, NULL, NULL, 0L)))
        return DXTRACE_ERR(TEXT("Lock"), hr);


    m_pWaveFile->ResetFile(false);

    if (FAILED(hr = m_pWaveFile->Read((BYTE *)pDSLockedBuffer, dwDSLockedBufferSize, &dwWavDataRead)))
        return DXTRACE_ERR(TEXT("Read"), hr);

    if (dwWavDataRead == 0)
    {

        FillMemory((BYTE *)pDSLockedBuffer, dwDSLockedBufferSize,
                   (BYTE)(m_pWaveFile->format->format.wBitsPerSample == 8 ? 128 : 0));
    }
    else if (dwWavDataRead < dwDSLockedBufferSize)
    {


        if (bRepeatWavIfBufferLarger)
        {

            DWORD dwReadSoFar = dwWavDataRead;
            while (dwReadSoFar < dwDSLockedBufferSize)
            {


                if (FAILED(hr = m_pWaveFile->ResetFile(false)))
                    return DXTRACE_ERR(TEXT("ResetFile"), hr);

                hr = m_pWaveFile->Read((BYTE *)pDSLockedBuffer + dwReadSoFar, dwDSLockedBufferSize - dwReadSoFar,
                                       &dwWavDataRead);
                if (FAILED(hr))
                    return DXTRACE_ERR(TEXT("Read"), hr);

                dwReadSoFar += dwWavDataRead;
            }
        }
        else
        {

            FillMemory((BYTE *)pDSLockedBuffer + dwWavDataRead, dwDSLockedBufferSize - dwWavDataRead,
                       (BYTE)(m_pWaveFile->format->format.wBitsPerSample == 8 ? 128 : 0));
        }
    }


    pDSB->Unlock(pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0);

    return S_OK;
}

HRESULT CSound::Play(DWORD dwPriority, DWORD dwFlags)
{
    HRESULT hr;
    BOOL bRestored;

    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    LPDIRECTSOUNDBUFFER pDSB = GetFreeBuffer();

    if (pDSB == NULL)
        return DXTRACE_ERR(TEXT("GetFreeBuffer"), E_FAIL);


    if (FAILED(hr = RestoreBuffer(pDSB, &bRestored)))
        return DXTRACE_ERR(TEXT("RestoreBuffer"), hr);

    if (bRestored)
    {

        if (FAILED(hr = FillBufferWithSound(pDSB, FALSE)))
            return DXTRACE_ERR(TEXT("FillBufferWithSound"), hr);


        Reset();
    }

    this->fading = 0;
    this->fadeFrames = 0;
    this->fadeTotal = 0;
    SetVolume(0);
    this->isPlaying = 1;
    this->dwPlayPriority = dwPriority;
    this->dwPlayFlags = dwFlags;
    this->clearedOnPlay = 0;
    this->played = 1;
    return pDSB->Play(0, dwPriority, dwFlags);
}



HRESULT CStreamingSound::HandleWaveStreamNotification(BOOL bLoopedPlay)
{
    HRESULT hr;
    DWORD dwPlayCsr;
    DWORD dwWriteCsr;
    DWORD dwCurrentPlayPos;
    DWORD dwPlayDelta;
    DWORD dwBytesWrittenToBuffer;

    if (m_apDSBuffer == NULL || m_pWaveFile == NULL)
        return CO_E_NOTINITIALIZED;



    m_apDSBuffer[0]->GetCurrentPosition(&dwPlayCsr, &dwWriteCsr);
    if ((m_dwNextWriteOffset >= dwWriteCsr - m_dwNotifySize && m_dwNextWriteOffset < dwWriteCsr) ||
        (dwWriteCsr - m_dwNotifySize < 0 && m_dwNextWriteOffset >= m_dwDSBufferSize - m_dwNotifySize))
    {
        return CO_E_NOTINITIALIZED;
    }


    BOOL bRestored;
    if (FAILED(hr = RestoreBuffer(m_apDSBuffer[0], &bRestored)))
    {
        return DXTRACE_ERR(TEXT("RestoreBuffer"), hr);
    }

    if (bRestored)
    {

        if (FAILED(hr = FillBufferWithSound(m_apDSBuffer[0], FALSE)))
        {
            return DXTRACE_ERR(TEXT("FillBufferWithSound"), hr);
        }
        return S_OK;
    }

    VOID *pLockedBuffer = NULL;
    VOID *pLockedBuffer2 = NULL;
    DWORD ulLength1;
    DWORD ulLength2;


    if (FAILED(hr = m_apDSBuffer[0]->Lock(m_dwNextWriteOffset, m_dwNotifySize, &pLockedBuffer,
                                          &ulLength1, &pLockedBuffer2, &ulLength2, 0L)))
    {
        return DXTRACE_ERR(TEXT("Lock"), hr);
    }



    if (pLockedBuffer2 != NULL)
        return E_UNEXPECTED;

    if (!m_bFillNextNotificationWithSilence)
    {

        if (FAILED(hr = m_pWaveFile->Read((BYTE *)pLockedBuffer, ulLength1, &dwBytesWrittenToBuffer)))
        {
            return DXTRACE_ERR(TEXT("Read"), hr);
        }
    }
    else
    {

        FillMemory(pLockedBuffer, ulLength1, (BYTE)(m_pWaveFile->format->format.wBitsPerSample == 8 ? 128 : 0));
        dwBytesWrittenToBuffer = ulLength1;
    }



    if (dwBytesWrittenToBuffer < ulLength1)
    {
        if (!bLoopedPlay)
        {

            FillMemory((BYTE *)pLockedBuffer + dwBytesWrittenToBuffer, ulLength1 - dwBytesWrittenToBuffer,
                       (BYTE)(m_pWaveFile->format->format.wBitsPerSample == 8 ? 128 : 0));


            m_bFillNextNotificationWithSilence = TRUE;
        }
        else
        {

            DWORD dwReadSoFar = dwBytesWrittenToBuffer;
            while (dwReadSoFar < ulLength1)
            {

                if (FAILED(hr = m_pWaveFile->ResetFile(true)))
                {
                    return DXTRACE_ERR(TEXT("ResetFile"), hr);
                }

                if (FAILED(hr = m_pWaveFile->Read((BYTE *)pLockedBuffer + dwReadSoFar,
                                                  ulLength1 - dwReadSoFar, &dwBytesWrittenToBuffer)))
                {
                    return DXTRACE_ERR(TEXT("Read"), hr);
                }

                dwReadSoFar += dwBytesWrittenToBuffer;
            }
        }
    }


    m_apDSBuffer[0]->Unlock(pLockedBuffer, ulLength1, NULL, 0);





    if (FAILED(hr = m_apDSBuffer[0]->GetCurrentPosition(&dwCurrentPlayPos, NULL)))
    {
        return DXTRACE_ERR(TEXT("GetCurrentPosition"), hr);
    }


    if (dwCurrentPlayPos < m_dwLastPlayPos)
        dwPlayDelta = (m_dwDSBufferSize - m_dwLastPlayPos) + dwCurrentPlayPos;
    else
        dwPlayDelta = dwCurrentPlayPos - m_dwLastPlayPos;

    m_dwPlayProgress += dwPlayDelta;
    m_dwLastPlayPos = dwCurrentPlayPos;



    if (m_bFillNextNotificationWithSilence)
    {

        if (m_dwPlayProgress >= m_pWaveFile->GetSize())
        {
            m_apDSBuffer[0]->Stop();
        }
    }


    m_dwNextWriteOffset += ulLength1;
    m_dwNextWriteOffset %= m_dwDSBufferSize;

    return S_OK;
}

HRESULT CStreamingSound::Reset()
{
    HRESULT hr;

    if (m_apDSBuffer[0] == NULL || m_pWaveFile == NULL)
        return CO_E_NOTINITIALIZED;

    m_dwLastPlayPos = 0;
    m_dwPlayProgress = 0;
    m_dwNextWriteOffset = 0;
    m_bFillNextNotificationWithSilence = FALSE;


    BOOL bRestored;
    if (FAILED(hr = RestoreBuffer(m_apDSBuffer[0], &bRestored)))
        return DXTRACE_ERR(TEXT("RestoreBuffer"), hr);

    if (bRestored)
    {

        if (FAILED(hr = FillBufferWithSound(m_apDSBuffer[0], FALSE)))
            return DXTRACE_ERR(TEXT("FillBufferWithSound"), hr);
    }

    m_pWaveFile->ResetFile(false);

    return m_apDSBuffer[0]->SetCurrentPosition(0L);
}

CWaveFile::~CWaveFile()
{
    Close();
}

HRESULT CWaveFile::Open(LPTSTR strFileName, BgmFormat *format, DWORD dwFlags)
{
    m_dwFlags = dwFlags;
    m_bIsReadingFromMemory = FALSE;

    if (m_dwFlags == WAVEFILE_READ)
    {
        if (strFileName == NULL)
            return E_INVALIDARG;

        this->hFile = CreateFileA(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (this->hFile == INVALID_HANDLE_VALUE)
            return E_FAIL;

        this->format = format;
        ResetFile(false);


        m_dwSize = m_ck.cksize;
    }

    return S_OK;
}

HRESULT CWaveFile::SwitchTrack(BgmFormat *format)
{
    if (m_bIsReadingFromMemory)
        return E_FAIL;
    if (this->hFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    this->format = format;
    ResetFile(false);
    m_dwSize = m_ck.cksize;

    return S_OK;
}

CSound::CSound(LPDIRECTSOUNDBUFFER *apDSBuffer, DWORD dwDSBufferSize, DWORD dwNumBuffers, CWaveFile *pWaveFile)
{
    DWORD i;

    m_apDSBuffer = new LPDIRECTSOUNDBUFFER[dwNumBuffers];
    for (i = 0; i < dwNumBuffers; i++)
        m_apDSBuffer[i] = apDSBuffer[i];

    m_dwDSBufferSize = dwDSBufferSize;
    m_dwNumBuffers = dwNumBuffers;
    m_pWaveFile = pWaveFile;
    this->played = 0;

    FillBufferWithSound(m_apDSBuffer[0], FALSE);


    for (i = 0; i < dwNumBuffers; i++)
        m_apDSBuffer[i]->SetCurrentPosition(0);

    this->isPlaying = 0;
}

CSound::~CSound()
{
    for (DWORD i = 0; i < m_dwNumBuffers; i++)
    {
        SAFE_RELEASE(m_apDSBuffer[i]);
    }

    SAFE_DELETE_ARRAY(m_apDSBuffer);
    SAFE_DELETE(m_pWaveFile);
}

CStreamingSound::CStreamingSound(LPDIRECTSOUNDBUFFER pDSBuffer, DWORD dwDSBufferSize, CWaveFile *pWaveFile, DWORD dwNotifySize)
    : CSound(&pDSBuffer, dwDSBufferSize, 1, pWaveFile)
{
    m_dwLastPlayPos = 0;
    m_dwPlayProgress = 0;
    m_dwNotifySize = dwNotifySize;
    m_dwNextWriteOffset = 0;
    m_bFillNextNotificationWithSilence = FALSE;
}

CStreamingSound::~CStreamingSound()
{
}

HRESULT CSoundManager::CreateStreaming(CStreamingSound **ppStreamingSound, LPTSTR strWaveFileName, DWORD dwCreationFlags,
                                       GUID guid3DAlgorithm, DWORD dwNotifyCount, DWORD dwNotifySize,
                                       HANDLE hNotifyEvent, BgmFormat *format)
{
    HRESULT hr;

    if (m_pDS == NULL)
        return CO_E_NOTINITIALIZED;

    LPDIRECTSOUNDBUFFER pDSBuffer = NULL;
    CWaveFile *pWaveFile = NULL;
    DSBPOSITIONNOTIFY *aPosNotify = NULL;
    LPDIRECTSOUNDNOTIFY pDSNotify = NULL;

    pWaveFile = new CWaveFile();
    if (pWaveFile->Open(strWaveFileName, format, WAVEFILE_READ) != S_OK)
    {
        delete pWaveFile;
        return E_FAIL;
    }


    DWORD dwDSBufferSize = dwNotifySize * dwNotifyCount;




    DSBUFFERDESC dsbd;
    ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = dwCreationFlags | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 |
                   DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE;
    dsbd.dwBufferBytes = dwDSBufferSize;
    dsbd.guid3DAlgorithm = guid3DAlgorithm;
    dsbd.lpwfxFormat = &pWaveFile->format->format;

    if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &pDSBuffer, NULL)))
        return DXTRACE_ERR(TEXT("CreateSoundBuffer"), E_FAIL);



    if (FAILED(hr = pDSBuffer->QueryInterface(IID_IDirectSoundNotify, (VOID **)&pDSNotify)))
        return DXTRACE_ERR(TEXT("QueryInterface"), E_FAIL);

    aPosNotify = new DSBPOSITIONNOTIFY[dwNotifyCount];
    if (aPosNotify == NULL)
        return E_OUTOFMEMORY;

    for (DWORD i = 0; i < dwNotifyCount; i++)
    {
        aPosNotify[i].dwOffset = (dwNotifySize * i) + dwNotifySize - 1;
        aPosNotify[i].hEventNotify = hNotifyEvent;
    }



    if (FAILED(hr = pDSNotify->SetNotificationPositions(dwNotifyCount, aPosNotify)))
    {
        SAFE_RELEASE(pDSNotify);
        SAFE_DELETE(aPosNotify);
        return DXTRACE_ERR(TEXT("SetNotificationPositions"), E_FAIL);
    }

    SAFE_RELEASE(pDSNotify);
    SAFE_DELETE(aPosNotify);


    *ppStreamingSound = new CStreamingSound(pDSBuffer, dwDSBufferSize, pWaveFile, dwNotifySize);
    (*ppStreamingSound)->dsbd = dsbd;
    (*ppStreamingSound)->manager = this;
    (*ppStreamingSound)->hNotifyEvent = hNotifyEvent;
    (*ppStreamingSound)->isBuffering = 0;

    return S_OK;
}

HRESULT CSoundManager::CreateStreamingFromMemory(CStreamingSound **ppStreamingSound, void *data, DWORD size, BgmFormat *format,
                                  DWORD dwCreationFlags, GUID guid3DAlgorithm, DWORD dwNotifyCount, DWORD dwNotifySize,
                                  HANDLE hNotifyEvent)
{
    HRESULT hr;

    if (m_pDS == NULL)
        return CO_E_NOTINITIALIZED;

    LPDIRECTSOUNDBUFFER pDSBuffer = NULL;
    CWaveFile *pWaveFile = NULL;
    DSBPOSITIONNOTIFY *aPosNotify = NULL;
    LPDIRECTSOUNDNOTIFY pDSNotify = NULL;

    pWaveFile = new CWaveFile();
    pWaveFile->OpenFromMemory((BYTE *)data, size, format, 0);

    DWORD dwDSBufferSize = dwNotifySize * dwNotifyCount;

    DSBUFFERDESC dsbd;
    ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = dwCreationFlags | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 |
                   DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE;
    dsbd.dwBufferBytes = dwDSBufferSize;
    dsbd.guid3DAlgorithm = guid3DAlgorithm;
    dsbd.lpwfxFormat = &pWaveFile->format->format;

    if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &pDSBuffer, NULL)))
        return DXTRACE_ERR(TEXT("CreateSoundBuffer"), E_FAIL);

    if (FAILED(hr = pDSBuffer->QueryInterface(IID_IDirectSoundNotify, (VOID **)&pDSNotify)))
        return DXTRACE_ERR(TEXT("QueryInterface"), E_FAIL);

    aPosNotify = new DSBPOSITIONNOTIFY[dwNotifyCount];
    if (aPosNotify == NULL)
        return E_OUTOFMEMORY;

    for (DWORD i = 0; i < dwNotifyCount; i++)
    {
        aPosNotify[i].dwOffset = (dwNotifySize * i) + dwNotifySize - 1;
        aPosNotify[i].hEventNotify = hNotifyEvent;
    }

    if (FAILED(hr = pDSNotify->SetNotificationPositions(dwNotifyCount, aPosNotify)))
    {
        SAFE_RELEASE(pDSNotify);
        SAFE_DELETE(aPosNotify);
        return DXTRACE_ERR(TEXT("SetNotificationPositions"), E_FAIL);
    }

    SAFE_RELEASE(pDSNotify);
    SAFE_DELETE(aPosNotify);

    *ppStreamingSound = new CStreamingSound(pDSBuffer, dwDSBufferSize, pWaveFile, dwNotifySize);
    (*ppStreamingSound)->dsbd = dsbd;
    (*ppStreamingSound)->manager = this;
    (*ppStreamingSound)->hNotifyEvent = hNotifyEvent;
    (*ppStreamingSound)->isBuffering = 0;

    return S_OK;
}
