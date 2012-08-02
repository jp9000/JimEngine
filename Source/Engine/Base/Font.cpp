/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Font.h:  Font Definition Class

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

#include <ft2build.h>
#include FT_FREETYPE_H


struct GlyphInfo
{
    ByteRect rect;
    BYTE width;
    wchar_t character;

    int cacheID;
    int cachePos;
};

struct FontFaceInfo
{
    String strFile;
    FT_Face ftFace;
    int refs;
};

FT_Library freetype = NULL;


SafeList<FontFaceInfo> Font::FaceList;

//=================================================
// Freetype allocation callbacks
//=================================================

void *engineftAlloc(FT_Memory memInfo, long size);
void *engineftRealloc(FT_Memory memInfo, long curSize, long newSize, void* mem);
void engineftFree(FT_Memory memInfo, void* mem);


void *engineftAlloc(FT_Memory memInfo, long size)
{
    return Allocate(size);
}

void *engineftRealloc(FT_Memory memInfo, long curSize, long newSize, void* mem)
{
    return ReAllocate(mem, newSize);
}

void engineftFree(FT_Memory memInfo, void* mem)
{
    Free(mem);
}

//=================================================
// Freetype file callbacks
//=================================================

void* engineftOpenFile(CTSTR filePath)
{
    XFile *file = new XFile;
    if(!file->Open(filePath, XFILE_READ, XFILE_OPENEXISTING))
    {
        delete file;
        return NULL;
    }

    return file;
}

unsigned long engineftReadFile(void *fileHandle, unsigned char *buffer, unsigned long count)
{
    XFile *file = static_cast<XFile*>(fileHandle);
    return file->Read(buffer, count);
}

void engineftSetFilePos(void *fileHandle, unsigned long offset)
{
    XFile *file = static_cast<XFile*>(fileHandle);
    file->SetPos(offset, XFILE_BEGIN);
}

unsigned long engineftGetFileSize(void *fileHandle)
{
    XFile *file = static_cast<XFile*>(fileHandle);
    return (unsigned long)file->GetFileSize();
}

void engineftCloseFile(void *fileHandle)
{
    XFile *file = static_cast<XFile*>(fileHandle);
    delete file;
}


//=================================================
// Freetype init/destroy
//=================================================

void InitFontStuff()
{
    traceIn(InitFontStuff);

    FT_SetFileCallbacks(engineftOpenFile, engineftReadFile, engineftSetFilePos, engineftGetFileSize, engineftCloseFile);

    FT_Memory ftmem;

    ftmem = (FT_Memory)malloc(sizeof(*ftmem));
    ftmem->user = 0;
    ftmem->alloc = engineftAlloc;
    ftmem->realloc = engineftRealloc;
    ftmem->free = engineftFree;

    if(FT_Init_FreeType_Jim(&freetype, ftmem))
        ErrOut(TEXT("Could not initialize freetype"));

    traceOut;
}

void DestroyFontStuff()
{
    traceIn(DestroyFontStuff);

    FT_Done_FreeType(freetype);

    traceOut;
}


//=================================================
// Font Class
//=================================================

Font* GraphicsSystem::CreateFont(TSTR lpName)
{
    traceIn(GraphicsSystem::CreateFont);

    assert(lpName);

    String path;
    if(!Engine::ConvertResourceName(lpName, TEXT("fonts"), path))
        return NULL;

    //-------------------------------------------------------------------
    // Get font information from font definition file

    ConfigFile fontFile;
    if(!fontFile.Open(path))
    {
        AppWarning(TEXT("Could not find font definition file '%s'"), path);
        return NULL;
    }

    String fontFileName = fontFile.GetString(TEXT("Font"), TEXT("File"));
    if(fontFileName.IsEmpty())
        return NULL;

    int size = fontFile.GetInt(TEXT("Font"), TEXT("Size"));
    if(size < 8 || size > 80)
    {
        AppWarning(TEXT("Font size too big or too small in font resource '%s'"), lpName);
        return NULL;
    }

    int maxCacheCount = fontFile.GetInt(TEXT("Font"), TEXT("MaxCacheCount"), 2);
    if(maxCacheCount > 10)
    {
        maxCacheCount = 10;
        AppWarning(TEXT("Font cache count too large in font resource '%s'"), lpName);
    }
    else if(maxCacheCount < 1)
    {
        maxCacheCount = 1;
        AppWarning(TEXT("Font cache count too small in font resource '%s'"), lpName);
    }

    //-------------------------------------------------------------------
    // Load font face data

    FontFaceInfo *face = NULL;
    FT_Error error;

    UINT faceID;

    for(int i=0; i<Font::FaceList.Num(); i++)
    {
        FontFaceInfo &faceInfo = Font::FaceList[i];
        if(faceInfo.strFile.CompareI(fontFileName))
        {
            face = &faceInfo;
            ++face->refs;
            faceID = i;
            break;
        }
    }

    if(!face)
    {
        String fontPath = (GetPathDirectory(path) + TEXT("/") + fontFileName);

        FT_Face ftFace = NULL;
        error = FT_New_Face(freetype, fontPath, 0, &ftFace);
        if(error)
        {
            AppWarning(TEXT("Could not intialize font from file '%s'"), fontFileName.Array());
            return NULL;
        }

        if(!ftFace->num_glyphs)
        {
            AppWarning(TEXT("Woa, font file '%s' has zero glyphs!  SCOOBY DOO WHERE ARE YOU!?!"), fontFileName.Array());
            return NULL;
        }

        face = Font::FaceList.CreateNew(&faceID);
        face->ftFace = ftFace;
        face->refs = 1;
        face->strFile = fontFileName;
    }

    if(!face)
    {
        AppWarning(TEXT("Shouldn't get to this warning."));
        return NULL;
    }

    int internalSize = size;

    int fullSize;
    while(true)
    {
        if(FT_Set_Pixel_Sizes(face->ftFace, 0, internalSize))
        {
            AppWarning(TEXT("Unsupported font size '%d' for font file '%s'"), size, fontFileName.Array());

            if(!--face->refs)
            {
                face->strFile.Clear();
                FT_Done_Face(face->ftFace);
                Font::FaceList.Remove(faceID);
            }
            return NULL;
        }

        FT_Size_Metrics &sizeInfo = face->ftFace->size->metrics;

        fullSize = (sizeInfo.ascender - sizeInfo.descender) >> 6;
        if(fullSize > size)
        {
            --internalSize;
            continue;
        }
        else
            break;
    }

    //-------------------------------------------------------------------
    // Create font

    Font *newFont = CreateObject(Font);
    newFont->fontData = face->ftFace;
    newFont->faceID = faceID;
    newFont->strName = lpName;
    newFont->size = size;
    newFont->internalSize = internalSize;
    newFont->maxCaches = maxCacheCount;
    newFont->maxGlyphsPerRow = (256/size);
    newFont->maxGlyphsPerCache = newFont->maxGlyphsPerRow*newFont->maxGlyphsPerRow;
    newFont->system = this;

    FontList.Add(newFont);

    return newFont;

    traceOut;
}

Font::~Font()
{
    traceIn(Font::~Font);

    traceIn(Removing font);
    if(system)
        system->FontList.RemoveItem(this);
    traceOut;

    traceIn(calling FT_Done_Face);
    if(fontData)
    {
        FontFaceInfo *face = &FaceList[faceID];
        if(!--face->refs)
        {
            FT_Done_Face(face->ftFace);
            face->strFile.Clear();
            FaceList.Remove(faceID);
        }
    }
    traceOut;

    traceIn(destroying tex cache);
    for(int i=0; i<TexCache.Num(); i++)
        DestroyObject(TexCache[i]);
    traceOut;

    traceOut;
}


GlyphInfo* Font::NewGlyph(TCHAR newChar)
{
    traceIn(Font::NewGlyph);

    int i;

    FT_GlyphSlot slot = ((FT_Face)fontData)->glyph;

    if(FT_Set_Pixel_Sizes((FT_Face)fontData, 0, internalSize))
        return NULL;

    UINT glyphIndex = FT_Get_Char_Index((FT_Face)fontData, newChar);

    if(FT_Load_Glyph((FT_Face)fontData, glyphIndex, FT_LOAD_DEFAULT))
        return NULL;

    if(FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL))
        return NULL;

    Texture *cache;
    BYTE* texImage;
    GlyphInfo *gi;

    if(CurGlyphs.Num() == (maxGlyphsPerCache*maxCaches))
    {
        int glyphID = LastUsedGlyphs[0];
        LastUsedGlyphs.MoveToEnd(0);

        gi = CurGlyphs+glyphID;

        cache = TexCache[gi->cacheID];
        texImage = (BYTE*)cache->GetImage();

        int cachePos = gi->cachePos;
        for(i=0; i<size; i++)
            zero(texImage+cachePos+(i*256), size);
    }
    else
    {
        int cacheID;
        int glyphOffset = (CurGlyphs.Num() % maxGlyphsPerCache);

        LastUsedGlyphs.Add(CurGlyphs.Num());

        gi = CurGlyphs.CreateNew();

        if(glyphOffset == 0)
        {
            LPBYTE lpData = (BYTE*)Allocate(256*256);
            zero(lpData, 256*256);

            cache = system->CreateTexture(256, 256, GS_ALPHA, lpData, FALSE, FALSE);
            cacheID = TexCache.Add(cache);
            gi->cachePos = 0;
        }
        else
        {
            cacheID = TexCache.Num()-1;
            cache = TexCache[cacheID];
            gi->cachePos =  (glyphOffset/maxGlyphsPerRow)*size*256;
            gi->cachePos += (glyphOffset%maxGlyphsPerRow)*size;
        }

        texImage = (BYTE*)cache->GetImage();

        gi->cacheID = cacheID;
        gi->rect.x = (gi->cachePos%256);
        gi->rect.y = gi->cachePos/256;
        gi->rect.y2 = gi->rect.y+size;
    }

    int bearingX = (slot->metrics.horiBearingX >> 6);
    int bearingY = (slot->metrics.horiBearingY >> 6);

    int descender = (-((FT_Face)fontData)->size->metrics.descender) >> 6;

    int adjY = (size-slot->bitmap_top)-descender;

    int texPos = gi->cachePos + (adjY*256) + bearingX;
    int bufPos = 0;
    for(i=0; i<slot->bitmap.rows; i++)
    {
        mcpy(texImage+texPos, slot->bitmap.buffer+bufPos, slot->bitmap.width);

        texPos += 256;
        bufPos += slot->bitmap.pitch;
    }

    gi->width = (slot->advance.x >> 6);
    gi->rect.x2 = gi->rect.x+gi->width;
    gi->character = newChar;

    cache->SetImage(texImage);

    return gi;

    traceOut;
}

GlyphInfo* Font::GetGlyph(TCHAR chr, BOOL bAdd)
{
    traceIn(Font::GetGlyph);

    int i;

    int lastID = (LastUsedGlyphs.Num()-1);
    for(i=lastID; i>=0; i--)
    {
        int glyphID = LastUsedGlyphs[i];
        GlyphInfo &gi = CurGlyphs[glyphID];

        if(gi.character == chr)
        {
            if(bAdd && (i < lastID))
                LastUsedGlyphs.MoveToEnd(i);

            return &gi;
        }
    }

    if(bAdd)
        return NewGlyph(chr);

    return NULL;

    traceOut;
}


int Font::LetterWidth(TCHAR letter)
{
    traceIn(Font::LetterWidth);

    if(letter == '\t')
        return LetterWidth(' ')*4;
    else if(letter == 0xA0)
        letter = ' ';

    GlyphInfo *gi = GetGlyph(letter);
    return gi->width;

    traceOut;
}

int Font::WordWidth(CTSTR lpStr)
{
    traceIn(Font::WordWidth);

    int totalWidth = 0;

    while(*lpStr)
    {
        TCHAR letter = *lpStr;

        if((letter == ' ') || (letter == '\t') || (letter == 0) || (letter == 0xA0) || (letter == 0x3000))
            break;

        GlyphInfo *gi = GetGlyph(letter);
        totalWidth += gi->width;
        ++lpStr;
    }

    return totalWidth;

    traceOut;
}

int Font::TextWidth(CTSTR lpStr)
{
    traceIn(Font::TextWidth);

    int totalWidth = 0;

    while(*lpStr)
    {
        TCHAR letter = *lpStr;

        if(letter == 0)
            break;

        if(letter == '\t')
        {
            totalWidth += LetterWidth(' ')*4;
            continue;
        }
        else if(letter == 0xA0)
            letter = ' ';

        GlyphInfo *gi = GetGlyph(letter);
        totalWidth += gi->width;
        ++lpStr;
    }
    return totalWidth;

    traceOut;
}

int Font::GetFontHeight()
{
    return size;
}

void Font::DrawLetter(TCHAR letter, int x, int y, DWORD color)
{
    traceIn(Font::DrawLetter);

    float newX = x;
    float newY = y;

    if((letter == ' ') || (letter == '\t') || (letter == 0) || (letter == 0xA0) || (letter == 0x3000))
        return;

    GlyphInfo *gi = GetGlyph(letter);
    float u1 = ((float)gi->rect.x)/256.0f;
    float u2 = ((float)gi->rect.x2)/256.0f;
    float v1 = ((float)gi->rect.y)/256.0f;
    float v2 = ((float)gi->rect.y2)/256.0f;

    system->DrawSpriteEx(TexCache[gi->cacheID], color, newX, newY, newX+gi->width, newY+size,
        u1, v1,
        u2, v2);

    traceOut;
}


void GraphicsSystem::DrawText(int x, int y, int cx, int cy, BOOL bWrapWords, CTSTR lpText)
{
    traceIn(GraphicsSystem::DrawText);

    if(!curFont)
    {
        AppWarning(TEXT("Tried to draw but no font is currently set."));
        return;
    }

    if(!lpText || !*lpText) return;

    int curX=x, curY=y;
    CTSTR lpTemp = lpText;
    BOOL bStopText = 0, bSkipLetter = 0;

    if(cx == 0) cx = curDisplayMode.dwWidth  - x;
    else        cx += x;

    if(cy == 0) cy = curDisplayMode.dwHeight - y;
    else        cy += y;


    do
    {
        float cxPos = cx, cyPos = cy;
        switch(*lpTemp)
        {
            case 0:
                break;

            case ' ':
                if(!bWrapWords || ((curX+curFont->WordWidth(&lpTemp[1])) < cxPos))
                    break;
            case '\n':
                bSkipLetter = 1;
                curX = x;
                curY += curFont->size;
                if((curY+curFont->size) >= cyPos)
                    bStopText = 1;
        }

        if(!bWrapWords && (curX+curFont->LetterWidth(*lpTemp)) > cxPos)
        {
            curX = x;
            curY += curFont->size;
            if((curY+curFont->size) >= cyPos)
                bStopText = 1;
        }

        if(bStopText)
            break;

        if(!bSkipLetter)
        {
            curFont->DrawLetter(*lpTemp, curX, curY, curFontColor);
            curX += curFont->LetterWidth(*lpTemp);
        }
        else
            bSkipLetter = 0;
    }
    while(*(++lpTemp));

    traceOut;
}

void GraphicsSystem::SetCurFont(TSTR lpName)
{
    curFont = GetFont(lpName);
    assert(curFont);
}

void GraphicsSystem::SetCurFont(Font *font)
{
    curFont = font;
    assert(curFont);
}

void GraphicsSystem::SetFontColor(DWORD color)
{
    curFontColor = color;
}

void  GraphicsSystem::DrawTextCenter(int x, int y, TSTR lpText)
{
    traceIn(GraphicsSystem::DrawTextCenter);

    assert(curFont);

    int width = curFont->TextWidth(lpText);
    int widthD2 = width/2;

    DrawText(x-widthD2, y, 0, 0, FALSE, lpText);

    traceOut;
}

void  GraphicsSystem::Printf(int x, int y, int cx, int cy, BOOL bWrapWords, CTSTR lpFormat, ...)
{
    traceIn(GraphicsSystem::Printf);

    va_list arglist;

    va_start(arglist, lpFormat);

    String blah = FormattedStringva(lpFormat, arglist);

    DrawText(x, y, cx, cy, bWrapWords, blah);

    traceOut;
}

Font* GraphicsSystem::GetFont(TSTR lpName)
{
    traceIn(GraphicsSystem::GetFont);

    for(DWORD i=0; i<FontList.Num(); i++)
    {
        if(FontList[i]->strName.CompareI(lpName))
            return FontList[i];
    }

    return CreateFont(lpName);

    traceOut;
}
