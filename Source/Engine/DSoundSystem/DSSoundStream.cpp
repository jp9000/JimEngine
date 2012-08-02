/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  DSSoundStream.cpp

  Copyright (c) 2001-2007, Hugh Bailey
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * The name of Hugh Bailey may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

  THIS SOFTWARE IS PROVIDED BY HUGH BAILEY "AS IS" AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL HUGH BAILEY BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUISNESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Blah blah blah blah.  To the fricken' code already!
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#include "DSSystem.h"


List<DSSoundStream*> DSSoundStream::StreamList;


DSSoundStream::DSSoundStream(const SoundStreamInfo &ssi)
{
    traceIn(DSSoundStream::DSSoundStream);

    WAVEFORMATEX wfe;
    DSBUFFERDESC dsbd;

    IDirectSoundBuffer *baseBuffer;
    IDirectSound8 *dSound = static_cast<DSSystem*>(SS)->dSound;

    zero(&wfe, sizeof(wfe));
    wfe.cbSize = sizeof(WAVEFORMATEX);
    wfe.nChannels = ssi.Channels;
    wfe.nSamplesPerSec = ssi.SamplesPerSecond;
    wfe.wBitsPerSample = ssi.BitsPerSample;
    wfe.wFormatTag = 1;
    wfe.nBlockAlign = (wfe.nChannels*wfe.wBitsPerSample)/8;
    wfe.nAvgBytesPerSec = (wfe.nSamplesPerSec*wfe.nBlockAlign);

    curBlockSize = wfe.nAvgBytesPerSec;

    dwStreamSize = curBlockSize*5;
    dwStreamDelay = 1000;

    zero(&dsbd, sizeof(dsbd));
    dsbd.lpwfxFormat = &wfe;
    dsbd.dwBufferBytes = dwStreamSize;
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwReserved = NULL;
    dsbd.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2;

    if(!SUCCEEDED(dSound->CreateSoundBuffer(&dsbd, &baseBuffer, NULL)))
        ErrOut(TEXT("DSSystem: Could not create sound stream buffer"));

    if(!SUCCEEDED(baseBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&buffer)))
        ErrOut(TEXT("DSSystem: Could not query the main buffer object"));
    baseBuffer->Release();

    hStreamThread = NULL;
    bPlaying = 0;
    curWritePos = 0;

    StreamList << this;

    traceOut;
}

DSSoundStream::~DSSoundStream()
{
    traceIn(DSSoundStream::~DSSoundStream);

    if(buffer)
    {
        Stop();
        buffer->Release();
    }

    StreamList.RemoveItem(this);

    traceOut;
}

BOOL DSSoundStream::RestoreBuffer()
{
    assert(buffer);

    DWORD status;
    buffer->GetStatus(&status);

    if(status & DSBSTATUS_BUFFERLOST)
    {
        if(buffer->Restore() == DSERR_BUFFERLOST)
            return 0;
    }

    return 1;
}

void DSSoundStream::Play()
{
    traceIn(DSSoundStream::Play);

    assert(buffer);

    hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    PlayThreadInfo *pti = (PlayThreadInfo*)Allocate(sizeof(PlayThreadInfo));
    pti->stream = this;
    pti->param = TSTReamProcParam;
    hStreamThread = OSCreateThread((ENGINETHREAD)PlayThread, pti);

    traceOut;
}

DWORD ENGINEAPI PlayThread(PlayThreadInfo *pti)
{
    traceIn((DSoundSystem)PlayThread);

    DSSoundStream *stream = pti->stream;
    IDirectSoundBuffer8 *buffer = stream->buffer;
    DWORD dwBytes;

    if(stream->bPlaying)
        return 0;

    stream->bPlaying = 1;

    BOOL bHasPlayed = 0, bValidEvent = 1;
    DWORD dwNextPos=stream->curBlockSize;

    while(stream->bPlaying)
    {
        DWORD eventRet = WaitForSingleObject(stream->hStopEvent, stream->dwStreamDelay/2);

        if(!stream->bPlaying || (eventRet == WAIT_OBJECT_0))
            break;

        if(bValidEvent)
            dwBytes = stream->StreamProc(stream, pti->param);

        if(!bHasPlayed)
        {
            if(stream->RestoreBuffer())
                buffer->Play(0, 0, DSBPLAY_LOOPING);

            bHasPlayed = 1;
        }

        DWORD status;
        buffer->GetStatus(&status);
        if(!(status & DSBSTATUS_PLAYING))
            break;

        DWORD dwCurPos;
        buffer->GetCurrentPosition(&dwCurPos, NULL);

        if((dwCurPos >= dwNextPos) && (dwCurPos < dwNextPos+stream->curBlockSize))
        {
            dwNextPos += stream->curBlockSize;
            if(dwNextPos == stream->dwStreamSize)
                dwNextPos = 0;
            bValidEvent = 1;
        }
        else
            bValidEvent = 0;
    }

    buffer->Stop();
    Free(pti);

    stream->bPlaying = 0;

    traceOutStop;

    return 0;
}


void DSSoundStream::Stop()
{
    traceIn(DSSoundStream::Stop);

    if(hStreamThread)
    {
        assert(buffer);
        assert(hStreamThread);

        bPlaying = 0;
        buffer->Stop();

        SetEvent(hStopEvent);
        OSWaitForThread(hStreamThread, WAIT_INFINITE);
        OSCloseThread(hStreamThread, NULL);

        CloseHandle(hStopEvent);
        hStopEvent = hStreamThread = NULL;
    }

    traceOut;
}


void DSSoundStream::SetVolume(float vol)
{
    if(vol > 1.0f) vol = 1.0f;
    if(vol < 0.0f) vol = 0.0f;

    streamVol = vol;

    buffer->SetVolume(ConvertToDBVolume(vol));
}


void DSSoundStream::AddStreamData(LPBYTE lpData)
{
    traceIn(DSSoundStream::AddStreamData);

    assert(buffer);

    LPBYTE lpWaveData=NULL, lpWrapWaveData=NULL;
    DWORD dwLockSize, dwWrapLockSize;

    if(curWritePos == dwStreamSize)
        curWritePos = 0;

    buffer->Lock(curWritePos, curBlockSize, (LPVOID*)&lpWaveData, &dwLockSize, (LPVOID*)&lpWrapWaveData, &dwWrapLockSize, 0);
    if(dwLockSize)
        mcpy(lpWaveData, lpData, dwLockSize);

    if(lpWrapWaveData)
    {
        mcpy(lpWrapWaveData, lpData+dwLockSize, dwWrapLockSize);
        curWritePos = dwWrapLockSize;
    }
    else
        curWritePos += dwLockSize;

    buffer->Unlock(lpWaveData, dwLockSize, lpWrapWaveData, dwWrapLockSize);

    traceOut;
}

void DSSoundStream::SetStreamCallback(NEXTSTREAMSEGMENTPROC proc, LPVOID param)
{
    StreamProc = proc;
    TSTReamProcParam = param;
}
