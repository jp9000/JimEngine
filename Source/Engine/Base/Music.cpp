/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Music.h:  Music class

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


#include "Base.h"

#include <vorbis\codec.h>
#include <vorbis\vorbisfile.h>

DefineClass(MusicManager);




typedef size_t (*OGGREADFUNC) (void *ptr, size_t size, size_t nmemb, void *datasource);
typedef int    (*OGGSEEKFUNC) (void *datasource, ogg_int64_t offset, int whence);
typedef int    (*OGGCLOSEFUNC)(void *datasource);
typedef long   (*OGGTELLFUNC) (void *datasource);


size_t ogg_fileread_cb(void *datainput, size_t HiIAmUseless, size_t size, XFile* file);
int ogg_fileseek_cb(XFile* file, ogg_int64_t offset, int whence);
int ogg_fileclose_cb(XFile* file);
long ogg_filetell_cb(XFile* file);


MusicManager::MusicManager()
{
    vorbisIntroData = (HANDLE)Allocate(sizeof(OggVorbis_File));
    vorbisLoopData  = (HANDLE)Allocate(sizeof(OggVorbis_File));
    curVol = 1.0f;
}

MusicManager::~MusicManager()
{
    Free(vorbisIntroData);
    Free(vorbisLoopData);
}


DWORD ENGINEAPI MusicManager::NextMusicSegment(SoundStream *stream, MusicManager *music)
{
    traceIn(MusicManager::NextMusicSegment);

    int curBlockSize = stream->GetCurBlockSize();
    LPBYTE lpPCMData = (LPBYTE)Allocate(curBlockSize*4);
    int curBitstream = 0, err = 0;
    int BytesRead, TotalBytes=0;

    if(music->overBufferSize)
    {
        TotalBytes = music->overBufferSize;
        mcpy(lpPCMData, music->overBuffer, music->overBufferSize);
    }

    while(TotalBytes < curBlockSize)
    {
        BytesRead = ov_read((OggVorbis_File*)music->curVorbisData, (char*)lpPCMData+TotalBytes, curBlockSize, 0, 2, 1, &curBitstream);
        if(BytesRead <= 0)
        {
            if(music->bHasIntro && (music->curVorbisData == music->vorbisIntroData))
            {
                curBitstream = 0;
                music->curVorbisData = music->vorbisLoopData;
            }

            ov_raw_seek((OggVorbis_File*)music->curVorbisData, 0);
            BytesRead = ov_read((OggVorbis_File*)music->curVorbisData, (char*)lpPCMData+TotalBytes, curBlockSize, 0, 2, 1, &curBitstream);
        }

        if(BytesRead <= 0)
            break;

        TotalBytes += BytesRead;
    }

    stream->AddStreamData(lpPCMData);

    if(TotalBytes > curBlockSize)
    {
        music->overBufferSize = TotalBytes-curBlockSize;
        mcpy(music->overBuffer, lpPCMData+curBlockSize, music->overBufferSize);
    }
    else
        music->overBufferSize = 0;
    Free(lpPCMData);

    //if(BytesRead < 0)
    //    CrashError(TEXT("Argh."));

    return (DWORD)TotalBytes;

    traceOut;
}


void MusicManager::Destroy()
{
    traceIn(MusicManager::Destroy);

    Stop();

    Super::Destroy();

    traceOut;
}


void MusicManager::Tick(float fTime)
{
    traceInFast(MusicManager::Tick);

    if(stream)
    {
        if(bFadingIn)
        {
            if(transitionTime < transitionSpeed)
            {
                float fVolAdjust = fTime/transitionSpeed;
                transitionTime += fTime;

                float fVolume = stream->GetVolume();
                float newVolume = (fVolume+fVolAdjust);

                if( (transitionTime > transitionSpeed)  ||
                    (newVolume > 1.0f)                  )
                {
                    stream->SetVolume(1.0f);

                    transitionTime = transitionSpeed = 0.0f;
                    bFadingIn = FALSE;
                }
                else
                    stream->SetVolume(newVolume);
            }
        }
        else
        {
            if(transitionTime > 1e-04f)
            {
                float fVolAdjust = fTime/transitionSpeed;
                transitionTime -= fTime;

                float fVolume = stream->GetVolume();
                float newVolume = (fVolume-fVolAdjust);

                if( (newVolume < 0.0f)        ||
                    (transitionTime < 1e-04f) )
                {
                    if(nextPlayPos != 0.0)
                    {
                        stream->SetVolume(0.0f);

                        transitionTime = 0.0f;
                        transitionSpeed = 2.0f;

                        bFadingIn = TRUE;
                    }

                    PlayForRealPlz();
                }
                else
                    stream->SetVolume(newVolume);
            }
        }
    }

    traceOutFast;
}


BOOL MusicManager::Play(CTSTR lpFileIntro, CTSTR lpFileLoop, float playPos, MusicTransition transitionType)
{
    traceIn(MusicManager::Play);

    if(engine->InEditor() || SS->IsOf(TEXT("NullSound")))
        return FALSE;

    if(scmpi(lpFileIntro, lpFileLoop) == 0)
        lpFileIntro = NULL;

    if(!Engine::ConvertResourceName(lpFileLoop, TEXT("music"), nextLoopFile))
        return FALSE;

    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(nextLoopFile, ofd);
    if(!hFind)
    {
        AppWarning(TEXT("MusicManager: File '%s' could not be opened"), nextLoopFile.Array());
        return FALSE;
    }
    OSFindClose(hFind);

    if(lpFileIntro && lpFileIntro[0])
    {
        if(!Engine::ConvertResourceName(lpFileIntro, TEXT("music"), nextIntroFile))
            return FALSE;

        hFind = OSFindFirstFile(nextIntroFile, ofd);
        if(!hFind)
        {
            AppWarning(TEXT("MusicManager: File '%s' could not be opened"), nextIntroFile.Array());
            return FALSE;
        }
        OSFindClose(hFind);
    }
    else
        nextIntroFile.Clear();

    nextPlayPos = playPos;

    bFadingIn = FALSE;

    String strFileName;
    switch(transitionType)
    {
        case MusicTransition_Instant:
            transitionTime = 0.0f;
            PlayForRealPlz();
            break;
        case MusicTransition_FastFade:
            transitionSpeed = transitionTime = 1.0f;
            break;
        case MusicTransition_Fade:
            transitionSpeed = transitionTime = 2.0f;
            break;
        case MusicTransition_SlowFade:
            transitionSpeed = transitionTime = 3.0f;
            break;
    }

    return TRUE;

    traceOut;
}

void MusicManager::PlayForRealPlz()
{
    traceIn(MusicManager::PlayForRealPlz);

    if(engine->InEditor())
        return;

    Stop();

    ov_callbacks callbacks;

    callbacks.read_func  = (OGGREADFUNC)ogg_fileread_cb;
    callbacks.seek_func  = (OGGSEEKFUNC)ogg_fileseek_cb;
    callbacks.close_func = (OGGCLOSEFUNC)ogg_fileclose_cb;
    callbacks.tell_func  = (OGGTELLFUNC)ogg_filetell_cb;

    oggLoopFile = new XFile;//(XFile*)Allocate(sizeof(XFile));//
    if(!oggLoopFile->Open(nextLoopFile, XFILE_READ|XFILE_WRITE, XFILE_OPENEXISTING))
    {
        delete oggLoopFile;
        Log(TEXT("MusicManager: File '%s' could not be opened"), (TSTR)nextLoopFile);
        return;
    }

    if(ov_open_callbacks((void*)oggLoopFile, (OggVorbis_File*)vorbisLoopData, NULL, 0, callbacks) < 0)
    {
        delete oggLoopFile;
        Log(TEXT("MusicManager: File '%s' is not a valid ogg morbis file"), (TSTR)nextLoopFile);
        return;
    }

    vorbis_info *vi = ov_info((OggVorbis_File*)vorbisLoopData, -1);

    if(!ov_seekable((OggVorbis_File*)vorbisLoopData))
    {
        Stop();
        Log(TEXT("MusicManager: '%s' is not seekable for some..  odd reason"), (TSTR)nextLoopFile);
        return;
    }

    if(!nextIntroFile.IsEmpty())
    {
        oggIntroFile = new XFile;//(XFile*)Allocate(sizeof(XFile));//
        if(!oggIntroFile->Open(nextIntroFile, XFILE_READ|XFILE_WRITE, XFILE_OPENEXISTING))
        {
            delete oggIntroFile;
            Stop();
            Log(TEXT("MusicManager: File '%s' is not a valid ogg morbis file"), (TSTR)nextIntroFile);
            return;
        }

        if(ov_open_callbacks((void*)oggIntroFile, (OggVorbis_File*)vorbisIntroData, NULL, 0, callbacks) < 0)
        {
            delete oggIntroFile;
            Stop();
            Log(TEXT("MusicManager: File '%s' is not a valid ogg morbis file"), (TSTR)nextIntroFile);
            return;
        }

        vorbis_info *vi = ov_info((OggVorbis_File*)vorbisIntroData, -1);

        if(!ov_seekable((OggVorbis_File*)vorbisIntroData))
        {
            Stop();
            Log(TEXT("MusicManager: '%s' is not seekable for some..  odd reason"), (TSTR)nextIntroFile);
            return;
        }

        introLength = ov_time_total((OggVorbis_File*)vorbisIntroData, -1);

        bHasIntro = TRUE;
    }
    else
        bHasIntro = FALSE;

    SoundStreamInfo ssi;

    ssi.BitsPerSample = 16;
    ssi.Channels = vi->channels;
    ssi.SamplesPerSecond = vi->rate;

    stream = SS->CreateSoundStream(ssi);
    stream->SetStreamCallback((NEXTSTREAMSEGMENTPROC)NextMusicSegment, (LPVOID)this);

    stream->SetVolume(curVol);

    overBuffer = (LPBYTE)Allocate(stream->GetCurBlockSize());
    overBufferSize = 0;

    double dPos = double(nextPlayPos);

    if(bHasIntro)
    {
        if(dPos < introLength)
            curVorbisData = vorbisIntroData;
        else
        {
            curVorbisData = vorbisLoopData;
            dPos -= introLength;
        }
    }
    else
        curVorbisData = vorbisLoopData;

    ov_time_seek((OggVorbis_File*)curVorbisData, dPos);

    for(int i=0; i<4; i++)
        NextMusicSegment(stream, this);

    stream->Play();

    traceOut;
}

void MusicManager::Stop()
{
    traceIn(MusicManager::Stop);

    if(engine->InEditor())
        return;

    if(stream)
    {
        stream->Stop();
        delete stream;
        stream = NULL;
    }
    if(overBuffer)
    {
        Free(overBuffer);
        overBuffer = NULL;
        overBufferSize = 0;
    }
    if(oggIntroFile)
    {
        ov_clear((OggVorbis_File*)vorbisIntroData);
        oggIntroFile = NULL;
    }
    if(oggLoopFile)
    {
        ov_clear((OggVorbis_File*)vorbisLoopData);
        oggLoopFile = NULL;
    }

    traceOut;
}

void MusicManager::SetVolume(float volume)
{
    traceIn(MusicManager::SetVolume);

    curVol = volume;
    if(stream)
        stream->SetVolume(volume);

    traceOut;
}

float MusicManager::GetCurrentPos()
{
    traceIn(MusicManager::GetCurrentPos);

    double curTime = ov_time_tell((OggVorbis_File*)curVorbisData);

    if(bHasIntro && (curVorbisData == vorbisLoopData))
        curTime += introLength;

    return float(curTime);

    traceOut;
}


size_t ogg_fileread_cb(void *datainput, size_t HiIAmUseless, size_t size, XFile* file)
{
    DWORD bytesRead;
    bytesRead = file->Read(datainput, (DWORD)size);

    if(bytesRead == XFILE_ERROR)
    {
        _set_errno(1);
        return 0;
    }

    return bytesRead;
}

int ogg_fileseek_cb(XFile* file, ogg_int64_t offset, int whence)
{
    DWORD seekType = 0;
    switch(whence)
    {
        case SEEK_CUR:
            seekType = XFILE_CURPOS;
            break;

        case SEEK_END:
            seekType = XFILE_END;
            break;

        case SEEK_SET:
            seekType = XFILE_BEGIN;
            break;
    }

    return (size_t)file->SetPos((int)offset, seekType);
}

int ogg_fileclose_cb(XFile* file)
{
    delete file;
    return 0;
}

long ogg_filetell_cb(XFile* file)
{
    return (size_t)file->GetPos();
}
