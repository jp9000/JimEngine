/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Config.h:  Configuration Files

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

//WARNING: LEAVE THIS FILE NOW OR YOU WILL BE FOREVER SCARRED BY THE HORRENDOUS CODE I WROTE 4000 YEARS AGO
//
// ...yes, I really need to rewrite this file, I know.  Don't tell me.  Please.


/*=========================================================
    Config
===========================================================*/

BOOL ConfigFile::Create(CTSTR lpConfigFile)
{
    strFileName = lpConfigFile;
    lpFileData = NULL;
    bOpen  = 0;

    if(LoadFile(XFILE_CREATEALWAYS))
        LoadData();
    else
        return 0;

    return 1;
}

BOOL ConfigFile::Open(CTSTR lpConfigFile, BOOL bOpenAlways)
{
    strFileName = lpConfigFile;
    lpFileData = NULL;
    bOpen  = 0;

    if(LoadFile(bOpenAlways ? XFILE_OPENALWAYS : XFILE_OPENEXISTING))
        LoadData();
    else
        return 0;

    return 1;
}

BOOL ConfigFile::LoadFile(DWORD dwOpenMode)
{
    if(bOpen)
        Close();

    XFile file;
    if(!file.Open(strFileName, XFILE_READ, dwOpenMode))
    {
        //Log(TEXT("Couldn't load config file: \"%s\""), (TSTR)strFileName);
        return 0;
    }

    dwLength = file.GetFileSize();

    LPSTR lpTempFileData = (LPSTR)Allocate(dwLength+5);
    file.Read(&lpTempFileData[2], dwLength);
    lpTempFileData[0] = lpTempFileData[dwLength+2] = 13;
    lpTempFileData[1] = lpTempFileData[dwLength+3] = 10;
    lpTempFileData[dwLength+4] = 0;
    file.Close();

    lpFileData = utf8_createTstr(lpTempFileData);
    dwLength = slen(lpFileData);
    Free(lpTempFileData);

    bOpen = 1;

    return 1;
}

void ConfigFile::LoadData()
{
    TSTR lpCurLine = lpFileData, lpNextLine;
    ConfigSection *lpCurSection=NULL;
    DWORD i;

    lpNextLine = schr(lpCurLine, '\r');

    while(*(lpCurLine = (lpNextLine+2)))
    {
        lpNextLine = schr(lpCurLine, '\r');
        *lpNextLine = 0;

        if((*lpCurLine == '[') && (*(lpNextLine-1) == ']'))
        {
            lpCurSection = Sections.CreateNew();

            lpCurSection->name = sfix(sdup(lpCurLine+1));
            lpCurSection->name[lpNextLine-lpCurLine-2] = 0;
        }
        else if(lpCurSection && *lpCurLine && (*(LPWORD)lpCurLine != '//'))
        {
            TSTR lpValuePtr = schr(lpCurLine, '=');

            if(lpValuePtr[1] != 0)
            {
                ConfigKey *key=NULL;

                *lpValuePtr = 0;

                for(i=0; i<lpCurSection->Keys.Num(); i++)
                {
                    if(scmpi(lpCurLine, lpCurSection->Keys[i].name) == 0)
                    {
                        key = &lpCurSection->Keys[i];
                        break;
                    }
                }

                if(!key)
                {
                    key = lpCurSection->Keys.CreateNew();
                    key->name = sfix(sdup(lpCurLine));
                }

                *lpValuePtr = '=';

                lpCurLine = lpValuePtr+1;

                TSTR value = sfix(sdup(lpCurLine));
                key->ValueList << value;
            }
        }

        *lpNextLine = '\r';
    }
}

void ConfigFile::Close()
{
    DWORD i,j,k;

    for(i=0; i<Sections.Num(); i++)
    {
        ConfigSection &section = Sections[i];
        Free(section.name);

        for(j=0; j<section.Keys.Num(); j++)
        {
            ConfigKey &key = section.Keys[j];
            Free(key.name);

            for(k=0; k<key.ValueList.Num(); k++)
                Free(key.ValueList[k]);

            key.ValueList.Clear();
        }
        section.Keys.Clear();
    }
    Sections.Clear();

    if(lpFileData)
    {
        Free(lpFileData);
        lpFileData      = NULL;
    }

    bOpen = 0;
}

String ConfigFile::GetString(CTSTR lpSection, CTSTR lpKey, CTSTR def)
{
    assert(lpSection);
    assert(lpKey);

    DWORD i,j;

    for(i=0; i<Sections.Num(); i++)
    {
        ConfigSection &section = Sections[i];
        if(scmpi(lpSection, section.name) == 0)
        {
            for(j=0; j<section.Keys.Num(); j++)
            {
                ConfigKey &key = section.Keys[j];
                if(scmpi(lpKey, key.name) == 0)
                    return String(key.ValueList[0]);
            }
        }
    }

    if(def)
        return String(def);
    else
        return String();
}

int ConfigFile::GetInt(CTSTR lpSection, CTSTR lpKey, int def)
{
    assert(lpSection);
    assert(lpKey);

    DWORD i,j;

    for(i=0; i<Sections.Num(); i++)
    {
        ConfigSection &section = Sections[i];
        if(scmpi(lpSection, section.name) == 0)
        {
            for(j=0; j<section.Keys.Num(); j++)
            {
                ConfigKey &key = section.Keys[j];
                if(scmpi(lpKey, key.name) == 0)
                {
                    if(scmpi(key.ValueList[0], TEXT("true")) == 0)
                        return 1;
                    else if(scmpi(key.ValueList[0], TEXT("false")) == 0)
                        return 0;
                    else
                        return tstring_base_to_int(key.ValueList[0], NULL, 0);
                }
            }
        }
    }

    return def;
}

float ConfigFile::GetFloat(CTSTR lpSection, CTSTR lpKey, float def)
{
    assert(lpSection);
    assert(lpKey);

    DWORD i,j;

    for(i=0; i<Sections.Num(); i++)
    {
        ConfigSection &section = Sections[i];
        if(scmpi(lpSection, section.name) == 0)
        {
            for(j=0; j<section.Keys.Num(); j++)
            {
                ConfigKey &key = section.Keys[j];
                if(scmpi(lpKey, key.name) == 0)
                    return tstof(key.ValueList[0]);
            }
        }
    }

    return def;
}

Color4 ConfigFile::GetColor(CTSTR lpSection, CTSTR lpKey)
{
    assert(lpSection);
    assert(lpKey);

    DWORD i,j;

    for(i=0; i<Sections.Num(); i++)
    {
        ConfigSection &section = Sections[i];
        if(scmpi(lpSection, section.name) == 0)
        {
            for(j=0; j<section.Keys.Num(); j++)
            {
                ConfigKey &key = section.Keys[j];
                if(scmpi(lpKey, key.name) == 0)
                {
                    TSTR strValue = key.ValueList[0];
                    if(*strValue == '{')
                    {
                        Color4 ret;

                        ret.x = float(tstof(++strValue));

                        if(!(strValue = schr(strValue, ',')))
                            break;
                        ret.y = float(tstof(++strValue));

                        if(!(strValue = schr(strValue, ',')))
                            break;
                        ret.z = float(tstof(++strValue));

                        if(!(strValue = schr(strValue, ',')))
                        {
                            ret.w = 1.0f;
                            return ret;
                        }
                        ret.w = float(tstof(++strValue));

                        return ret;
                    }
                    else if(*strValue == '[')
                    {
                        Color4 ret;

                        ret.x = (float(tstoi(++strValue))/255.0f)+0.001f;

                        if(!(strValue = schr(strValue, ',')))
                            break;
                        ret.y = (float(tstoi(++strValue))/255.0f)+0.001f;

                        if(!(strValue = schr(strValue, ',')))
                            break;
                        ret.z = (float(tstoi(++strValue))/255.0f)+0.001f;

                        if(!(strValue = schr(strValue, ',')))
                        {
                            ret.w = 1.0f;
                            return ret;
                        }
                        ret.w = (float(tstoi(++strValue))/255.0f)+0.001f;

                        return ret;
                    }
                    else if( (*LPWORD(strValue) == 'x0') ||
                             (*LPWORD(strValue) == 'X0') )
                    {
                        return RGBA_to_Vect4(tstring_base_to_int(strValue+2, NULL, 16));
                    }
                }
            }
        }
    }

    return Color4(0.0f, 0.0f, 0.0f, 0.0f);
}

BOOL ConfigFile::GetStringList(CTSTR lpSection, CTSTR lpKey, StringList &StrList)
{
    assert(lpSection);
    assert(lpKey);

    BOOL bFoundKey = 0;

    DWORD i,j,k;

    for(i=0; i<Sections.Num(); i++)
    {
        ConfigSection &section = Sections[i];
        if(scmpi(lpSection, section.name) == 0)
        {
            for(j=0; j<section.Keys.Num(); j++)
            {
                ConfigKey &key = section.Keys[j];
                if(scmpi(lpKey, key.name) == 0)
                {
                    for(k=0; k<key.ValueList.Num(); k++)
                        StrList << key.ValueList[k];

                    bFoundKey = 1;
                }
            }
        }
    }

    return bFoundKey;
}

BOOL ConfigFile::GetIntList(CTSTR lpSection, CTSTR lpKey, List<int> &IntList)
{
    assert(lpSection);
    assert(lpKey);

    DWORD i,j,k;
    BOOL bFoundKey = 0;

    for(i=0; i<Sections.Num(); i++)
    {
        ConfigSection &section = Sections[i];
        if(scmpi(lpSection, section.name) == 0)
        {
            for(j=0; j<section.Keys.Num(); j++)
            {
                ConfigKey &key = section.Keys[j];
                if(scmpi(lpKey, key.name) == 0)
                {
                    for(k=0; k<key.ValueList.Num(); k++)
                    {
                        if(scmpi(key.ValueList[k], TEXT("true")) == 0)
                            IntList << 1;
                        else if(scmpi(key.ValueList[k], TEXT("false")) == 0)
                            IntList << 0;
                        else
                            IntList << tstring_base_to_int(key.ValueList[k], NULL, 0);
                    }

                    bFoundKey = 1;
                }
            }
        }
    }

    return bFoundKey;
}

BOOL ConfigFile::GetFloatList(CTSTR lpSection, CTSTR lpKey, List<float> &FloatList)
{
    assert(lpSection);
    assert(lpKey);

    DWORD i,j,k;
    BOOL bFoundKey = 0;

    for(i=0; i<Sections.Num(); i++)
    {
        ConfigSection &section = Sections[i];
        if(scmpi(lpSection, section.name) == 0)
        {
            for(j=0; j<section.Keys.Num(); j++)
            {
                ConfigKey &key = section.Keys[j];
                if(scmpi(lpKey, key.name) == 0)
                {
                    for(k=0; k<key.ValueList.Num(); k++)
                        FloatList << tstof(key.ValueList[k]);

                    bFoundKey = 1;
                }
            }
        }
    }

    return bFoundKey;
}

BOOL ConfigFile::GetColorList(CTSTR lpSection, CTSTR lpKey, List<Color4> &ColorList)
{
    assert(lpSection);
    assert(lpKey);

    DWORD i,j,k;
    BOOL bFoundKey = 0;

    for(i=0; i<Sections.Num(); i++)
    {
        ConfigSection &section = Sections[i];
        if(scmpi(lpSection, section.name) == 0)
        {
            for(j=0; j<section.Keys.Num(); j++)
            {
                ConfigKey &key = section.Keys[j];
                if(scmpi(lpKey, key.name) == 0)
                {
                    for(k=0; k<key.ValueList.Num(); k++)
                    {
                        TSTR strValue = key.ValueList[k];
                        if(*strValue == '{')
                        {
                            Color4 ret;

                            ret.x = float(tstof(++strValue));

                            if(!(strValue = schr(strValue, ',')))
                                break;
                            ret.y = float(tstof(++strValue));

                            if(!(strValue = schr(strValue, ',')))
                                break;
                            ret.z = float(tstof(++strValue));

                            if(!(strValue = schr(strValue, ',')))
                                ret.w = 0.0f;
                            else
                                ret.w = float(tstof(++strValue));
                            ColorList << ret;
                        }
                        else if(*strValue == '[')
                        {
                            Color4 ret;

                            ret.x = float(tstoi(++strValue))/255.0f;

                            if(!(strValue = schr(strValue, ',')))
                                break;
                            ret.y = float(tstoi(++strValue))/255.0f;

                            if(!(strValue = schr(strValue, ',')))
                                break;
                            ret.z = float(tstoi(++strValue))/255.0f;

                            if(!(strValue = schr(strValue, ',')))
                                ret.w = 0.0f;
                            else
                                ret.w = float(tstoi(++strValue))/255.0f;

                            ColorList << ret;
                        }
                        else if( (*LPWORD(strValue) == 'x0') ||
                                 (*LPWORD(strValue) == 'X0') )
                        {
                            ColorList << RGBA_to_Vect4(tstring_base_to_int(strValue+2, NULL, 16));
                        }
                    }

                    bFoundKey = 1;
                }
            }
        }
    }

    return bFoundKey;
}

void ConfigFile::SetString(CTSTR lpSection, CTSTR lpKey, CTSTR lpString)
{
    if(!bOpen)
        return;

    if(!lpString)
        return;

    SetKey(lpSection, lpKey, lpString);
}

void ConfigFile::SetInt(CTSTR lpSection, CTSTR lpKey, int number)
{
    if(!bOpen)
        return;

    TCHAR strNum[20];

    itots_s(number, strNum, 19, 10);
    SetKey(lpSection, lpKey, strNum);
}

void ConfigFile::SetFloat(CTSTR lpSection, CTSTR lpKey, float number)
{
    if(!bOpen)
        return;

    TCHAR strNum[20];

    tsprintf_s(strNum, 19, TEXT("%f"), number);
    SetKey(lpSection, lpKey, strNum);
}

void ConfigFile::SetColor(CTSTR lpSection, CTSTR lpKey, const Color4 &color)
{
    if(!bOpen)
        return;

    TCHAR strColor[50];

    tsprintf_s(strColor, 49, TEXT("{%.3f, %.3f, %.3f, %.3f}"), color.x, color.y, color.z, color.w);
    SetKey(lpSection, lpKey, strColor);
}

void ConfigFile::AddString(CTSTR lpSection, CTSTR lpKey, CTSTR lpString)
{
    if(!bOpen)
        return;

    if(!lpString)
        return;

    AddKey(lpSection, lpKey, lpString);
}

void ConfigFile::AddInt(CTSTR lpSection, CTSTR lpKey, int number)
{
    if(!bOpen)
        return;

    TCHAR strNum[20];

    itots_s(number, strNum, 19, 10);
    AddKey(lpSection, lpKey, strNum);
}

void ConfigFile::AddFloat(CTSTR lpSection, CTSTR lpKey, float number)
{
    if(!bOpen)
        return;

    TCHAR strNum[20];

    tsprintf_s(strNum, 19, TEXT("%f"), number);
    AddKey(lpSection, lpKey, strNum);
}

void ConfigFile::AddColor(CTSTR lpSection, CTSTR lpKey, const Color4 &color)
{
    if(!bOpen)
        return;

    TCHAR strColor[50];

    tsprintf_s(strColor, 49, TEXT("{%.3f, %.3f, %.3f, %.3f}"), color.x, color.y, color.z, color.w);
    AddKey(lpSection, lpKey, strColor);
}


void ConfigFile::SetStringList(CTSTR lpSection, CTSTR lpKey, StringList &StrList)
{
    while(HasKey(lpSection, lpKey))
        Remove(lpSection, lpKey);

    for(int i=0; i<StrList.Num(); i++)
        AddString(lpSection, lpKey, StrList[i]);
}

void ConfigFile::SetIntList(CTSTR lpSection, CTSTR lpKey, List<int> &IntList)
{
    while(HasKey(lpSection, lpKey))
        Remove(lpSection, lpKey);

    for(int i=0; i<IntList.Num(); i++)
        AddInt(lpSection, lpKey, IntList[i]);
}

void ConfigFile::SetFloatList(CTSTR lpSection, CTSTR lpKey, List<float> &FloatList)
{
    while(HasKey(lpSection, lpKey))
        Remove(lpSection, lpKey);

    for(int i=0; i<FloatList.Num(); i++)
        AddFloat(lpSection, lpKey, FloatList[i]);
}

void ConfigFile::SetColorList(CTSTR lpSection, CTSTR lpKey, List<Color4> &ColorList)
{
    while(HasKey(lpSection, lpKey))
        Remove(lpSection, lpKey);

    for(int i=0; i<ColorList.Num(); i++)
        AddColor(lpSection, lpKey, ColorList[i]);
}


BOOL  ConfigFile::HasKey(CTSTR lpSection, CTSTR lpKey)
{
    for(unsigned int i=0; i<Sections.Num(); i++)
    {
        ConfigSection &section = Sections[i];
        if(scmpi(section.name, lpSection) == 0)
        {
            for(unsigned int j=0; j<section.Keys.Num(); j++)
            {
                if(scmpi(section.Keys[j].name, lpKey) == 0)
                    return TRUE;
            }
        }
    }

    return FALSE;
}



void  ConfigFile::SetKey(CTSTR lpSection, CTSTR lpKey, CTSTR newvalue)
{
    assert(lpSection);
    assert(lpKey);
    TSTR lpTemp = lpFileData, lpEnd = &lpFileData[dwLength], lpSectionStart;
    DWORD dwSectionNameSize = slen(lpSection), dwKeyNameSize = slen(lpKey);
    BOOL  bInSection = 0;

    do
    {
        lpTemp = sstr(lpTemp, TEXT("\n["));
        if(!lpTemp)
            break;

        lpTemp += 2;
        if((scmpi_n(lpTemp, lpSection, dwSectionNameSize) == 0) && (lpTemp[dwSectionNameSize] == ']'))
        {
            bInSection = 1;
            lpSectionStart = lpTemp = schr(lpTemp, '\n')+1;
            break;
        }
    }while(lpTemp < lpEnd);

    if(!bInSection)
    {
        lpTemp -= 2;

        XFile file(strFileName, XFILE_WRITE, XFILE_CREATEALWAYS);
        file.Write("\xEF\xBB\xBF", 3);
        file.WriteAsUTF8(&lpFileData[2], dwLength-4);
        file.Write("\r\n[", 3);
        file.WriteAsUTF8(lpSection, dwSectionNameSize);
        file.Write("]\r\n", 3);
        file.WriteAsUTF8(lpKey, dwKeyNameSize);
        file.Write("=", 1);
        file.WriteAsUTF8(newvalue, slen(newvalue));
        file.Write("\r\n", 2);
        file.Close();

        if(LoadFile(XFILE_OPENEXISTING))
            LoadData();
        return;
    }

    do
    {
        if(*lpTemp == '[')
        {
            XFile file(strFileName, XFILE_WRITE, XFILE_CREATEALWAYS);
            file.Write("\xEF\xBB\xBF", 3);
            file.WriteAsUTF8(&lpFileData[2], lpSectionStart-lpFileData-2);
            file.WriteAsUTF8(lpKey, dwKeyNameSize);
            file.Write("=", 1);
            file.WriteAsUTF8(newvalue, slen(newvalue));
            file.Write("\r\n", 2);
            file.WriteAsUTF8(lpSectionStart, slen(lpSectionStart)-2);
            file.Close();

            if(LoadFile(XFILE_OPENEXISTING))
                LoadData();
            return;
        }
        else if(*(LPWORD)lpTemp == '//')
        {
            lpTemp = schr(lpTemp, '\n')+1;
            continue;
        }
        else if(bInSection)
        {
            if((scmpi_n(lpTemp, lpKey, dwKeyNameSize) == 0) && (lpTemp[dwKeyNameSize] == '='))
            {
                lpTemp = &lpTemp[dwKeyNameSize+1];
                TSTR lpNextLine = schr(lpTemp, '\r');
                XFile file(strFileName, XFILE_WRITE, XFILE_CREATEALWAYS);
                file.Write("\xEF\xBB\xBF", 3);
                file.WriteAsUTF8(&lpFileData[2], lpTemp-lpFileData-2);
                file.WriteAsUTF8(newvalue, slen(newvalue));
                file.WriteAsUTF8(lpNextLine, slen(lpNextLine)-2);
                file.Close();

                if(LoadFile(XFILE_OPENEXISTING))
                    LoadData();
                return;
            }
        }

        lpTemp = schr(lpTemp, '\n')+1;
    }while(lpTemp < lpEnd);

    XFile file(strFileName, XFILE_WRITE, XFILE_CREATEALWAYS);
    file.Write("\xEF\xBB\xBF", 3);
    file.WriteAsUTF8(&lpFileData[2], lpSectionStart-lpFileData-2);
    file.WriteAsUTF8(lpKey, dwKeyNameSize);
    file.Write("=", 1);
    file.WriteAsUTF8(newvalue, slen(newvalue));
    file.Write("\r\n", 2);
    file.WriteAsUTF8(lpSectionStart, slen(lpSectionStart)-2);
    file.Close();

    if(LoadFile(XFILE_OPENEXISTING))
        LoadData();
}

void  ConfigFile::Remove(CTSTR lpSection, CTSTR lpKey)
{
    assert(lpSection);
    assert(lpKey);
    TSTR lpTemp = lpFileData, lpEnd = &lpFileData[dwLength], lpSectionStart;
    DWORD dwSectionNameSize = slen(lpSection), dwKeyNameSize = slen(lpKey);
    BOOL  bInSection = 0;

    do
    {
        lpTemp = sstr(lpTemp, TEXT("\n["));
        if(!lpTemp)
            break;

        lpTemp += 2;
        if((scmpi_n(lpTemp, lpSection, dwSectionNameSize) == 0) && (lpTemp[dwSectionNameSize] == ']'))
        {
            bInSection = 1;
            lpSectionStart = lpTemp = schr(lpTemp, '\n')+1;
            break;
        }
    }while(lpTemp < lpEnd);

    if(!bInSection)
        return;  //not possible, usually.

    do
    {
        if(*lpTemp == '[')
            return;
        else if(bInSection && (*(LPWORD)lpTemp != '//'))
        {
            if((scmpi_n(lpTemp, lpKey, dwKeyNameSize) == 0) && (lpTemp[dwKeyNameSize] == '='))
            {
                TSTR lpNextLine = schr(lpTemp, '\n')+1;
                XFile file(strFileName, XFILE_WRITE, XFILE_CREATEALWAYS);
                file.Write("\xEF\xBB\xBF", 3);
                file.WriteAsUTF8(&lpFileData[2], lpTemp-lpFileData-2);
                file.WriteAsUTF8(lpNextLine, slen(lpNextLine)-2);
                file.Close();

                if(LoadFile(XFILE_OPENEXISTING))
                    LoadData();
                return;
            }
        }

        lpTemp = schr(lpTemp, '\n')+1;
    }while(lpTemp < lpEnd);
}

void  ConfigFile::AddKey(CTSTR lpSection, CTSTR lpKey, CTSTR newvalue)
{
    assert(lpSection);
    assert(lpKey);
    TSTR lpTemp = lpFileData, lpEnd = &lpFileData[dwLength], lpSectionStart;
    DWORD dwSectionNameSize = slen(lpSection), dwKeyNameSize = slen(lpKey);
    BOOL  bInSection = 0;

    do
    {
        lpTemp = sstr(lpTemp, TEXT("\n["));
        if(!lpTemp)
            break;

        lpTemp += 2;
        if((scmpi_n(lpTemp, lpSection, dwSectionNameSize) == 0) && (lpTemp[dwSectionNameSize] == ']'))
        {
            bInSection = 1;
            lpSectionStart = lpTemp = schr(lpTemp, '\n')+1;
            break;
        }
    }while(lpTemp < lpEnd);

    if(!bInSection)
    {
        XFile file(strFileName, XFILE_WRITE, XFILE_CREATEALWAYS);
        file.Write("\xEF\xBB\xBF", 3);
        file.WriteAsUTF8(&lpFileData[2], dwLength-4);
        file.Write("\r\n[", 3);
        file.WriteAsUTF8(lpSection, dwSectionNameSize);
        file.Write("]\r\n", 3);
        file.WriteAsUTF8(lpKey, dwKeyNameSize);
        file.Write("=", 1);
        file.WriteAsUTF8(newvalue, slen(newvalue));
        file.Write("\r\n", 2);
        file.Close();

        if(LoadFile(XFILE_OPENEXISTING))
            LoadData();
        return;
    }

    TSTR lpLastItem = NULL;

    do
    {
        if(*lpTemp == '[')
        {
            XFile file(strFileName, XFILE_WRITE, XFILE_CREATEALWAYS);
            file.Write("\xEF\xBB\xBF", 3);
            file.WriteAsUTF8(&lpFileData[2], lpSectionStart-lpFileData-2);
            file.WriteAsUTF8(lpKey, dwKeyNameSize);
            file.Write("=", 1);
            file.WriteAsUTF8(newvalue, slen(newvalue));
            file.Write("\r\n", 2);
            file.WriteAsUTF8(lpSectionStart, slen(lpSectionStart)-2);
            file.Close();

            if(LoadFile(XFILE_OPENEXISTING))
                LoadData();
            return;
        }
        else if(*(LPWORD)lpTemp == '//')
        {
            lpTemp = schr(lpTemp, '\n')+1;
            continue;
        }
        else if(bInSection)
        {
            if((scmpi_n(lpTemp, lpKey, dwKeyNameSize) == 0) && (lpTemp[dwKeyNameSize] == '='))
            {
                lpLastItem = schr(lpTemp, '\n')+1;
            }
            else if(lpLastItem)
            {
                lpTemp = lpLastItem;
                XFile file(strFileName, XFILE_WRITE, XFILE_CREATEALWAYS);
                file.Write("\xEF\xBB\xBF", 3);
                file.WriteAsUTF8(&lpFileData[2], lpTemp-lpFileData-2);
                file.WriteAsUTF8(lpKey, dwKeyNameSize);
                file.Write("=", 1);
                file.WriteAsUTF8(newvalue, slen(newvalue));
                file.Write("\r\n", 2);
                file.WriteAsUTF8(lpTemp, slen(lpTemp)-2);
                file.Close();

                if(LoadFile(XFILE_OPENEXISTING))
                    LoadData();
                return;
            }
        }

        lpTemp = schr(lpTemp, '\n')+1;
    }while(lpTemp < lpEnd);

    XFile file(strFileName, XFILE_WRITE, XFILE_CREATEALWAYS);
    file.Write("\xEF\xBB\xBF", 3);
    file.WriteAsUTF8(&lpFileData[2], lpSectionStart-lpFileData-2);
    file.WriteAsUTF8(lpKey, dwKeyNameSize);
    file.Write("=", 1);
    file.WriteAsUTF8(newvalue, slen(newvalue));
    file.Write("\r\n", 2);
    file.WriteAsUTF8(lpSectionStart, slen(lpSectionStart)-2);
    file.Close();

    if(LoadFile(XFILE_OPENEXISTING))
        LoadData();
}
