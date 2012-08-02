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

#ifndef CONFIG_HEADER
#define CONFIG_HEADER



/*=========================================================
    Config
===========================================================*/

enum {INT_VALUE, STRING_VALUE};

struct ConfigKey
{
    TSTR name;
    List<TSTR> ValueList;
};

struct ConfigSection
{
    TSTR name;
    List<ConfigKey> Keys;
};


class BASE_EXPORT ConfigFile
{
public:
    ConfigFile() : bOpen(0), strFileName(), lpFileData(NULL), dwLength(0) {}
    ~ConfigFile() {Close();}

    BOOL  Create(CTSTR lpConfigFile);
    BOOL  Open(CTSTR lpConfigFile, BOOL bOpenAlways=FALSE);
    void  Close();

    String GetString(CTSTR lpSection, CTSTR lpKey, CTSTR def=NULL);
    int   GetInt(CTSTR lpSection, CTSTR lpKey, int def=0);
    float GetFloat(CTSTR lpSection, CTSTR lpKey, float def=0.0f);
    Color4 GetColor(CTSTR lpSection, CTSTR lpKey);

    BOOL  GetStringList(CTSTR lpSection, CTSTR lpKey, StringList &StrList);
    BOOL  GetIntList(CTSTR lpSection, CTSTR lpKey, List<int> &IntList);
    BOOL  GetFloatList(CTSTR lpSection, CTSTR lpKey, List<float> &FloatList);
    BOOL  GetColorList(CTSTR lpSection, CTSTR lpKey, List<Color4> &ColorList);

    void  SetString(CTSTR lpSection, CTSTR lpKey, CTSTR lpString);
    void  SetInt(CTSTR lpSection, CTSTR lpKey, int number);
    void  SetFloat(CTSTR lpSection, CTSTR lpKey, float number);
    void  SetColor(CTSTR lpSection, CTSTR lpKey, const Color4 &color);

    void  SetStringList(CTSTR lpSection, CTSTR lpKey, StringList &StrList);
    void  SetIntList(CTSTR lpSection, CTSTR lpKey, List<int> &IntList);
    void  SetFloatList(CTSTR lpSection, CTSTR lpKey, List<float> &FloatList);
    void  SetColorList(CTSTR lpSection, CTSTR lpKey, List<Color4> &ColorList);

    void  AddString(CTSTR lpSection, CTSTR lpKey, CTSTR lpString);
    void  AddInt(CTSTR lpSection, CTSTR lpKey, int number);
    void  AddFloat(CTSTR lpSection, CTSTR lpKey, float number);
    void  AddColor(CTSTR lpSection, CTSTR lpKey, const Color4 &color);

    BOOL  HasKey(CTSTR lpSection, CTSTR lpKey);

    void  Remove(CTSTR lpSection, CTSTR lpKey);

    inline Color3 GetColor3(CTSTR lpSection, CTSTR lpKey)
    {
        return Color3(GetColor(lpSection, lpKey));
    }

    inline void SetColor3(CTSTR lpSection, CTSTR lpKey, const Color3 &color)
    {
        return SetColor(lpSection, lpKey, Color4(color));
    }

private:
    BOOL  LoadFile(DWORD dwOpenMode);
    void  LoadData();
    void  SetKey(CTSTR lpSection, CTSTR lpKey, CTSTR newvalue);
    void  AddKey(CTSTR lpSection, CTSTR lpKey, CTSTR newvalue);

    List<ConfigSection> Sections;

    BOOL  bOpen;
    String strFileName;
    TSTR lpFileData;
    DWORD dwLength;
};


//<Script module="Base" globaldecs="Config.xscript">
Declare_Native_Global(NativeGlobal_GetConfigString);
Declare_Native_Global(NativeGlobal_GetConfigInt);
Declare_Native_Global(NativeGlobal_GetConfigFloat);
Declare_Native_Global(NativeGlobal_GetConfigColor);
Declare_Native_Global(NativeGlobal_SetConfigString);
Declare_Native_Global(NativeGlobal_SetConfigInt);
Declare_Native_Global(NativeGlobal_SetConfigFloat);
Declare_Native_Global(NativeGlobal_SetConfigColor);
//</Script>


#endif
