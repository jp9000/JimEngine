/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EngineString.h:  String class

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


#ifndef STRING_HEADER
#define STRING_HEADER


BASE_EXPORT int    ENGINEAPI   slen(const TCHAR *strSrc);
BASE_EXPORT int    ENGINEAPI   ssize(const TCHAR *strSrc);
BASE_EXPORT void   ENGINEAPI   scpy(TCHAR *strDest, const TCHAR *strSrc);
BASE_EXPORT void   ENGINEAPI   scpy_n(TCHAR *strDest, const TCHAR *strSrc, unsigned int n);
BASE_EXPORT void   ENGINEAPI   scat(TCHAR *strDest, const TCHAR *strSrc);
BASE_EXPORT void   ENGINEAPI   scat_n(TCHAR *strDest, const TCHAR *strSrc, unsigned int n);
BASE_EXPORT void   ENGINEAPI   slwr(TCHAR *str);
BASE_EXPORT void   ENGINEAPI   supr(TCHAR *str);
BASE_EXPORT TCHAR* ENGINEAPI   sdup(const TCHAR *str);
BASE_EXPORT int    ENGINEAPI   scmp(const TCHAR *str1, const TCHAR *str2);
BASE_EXPORT int    ENGINEAPI   scmpi(const TCHAR *str1, const TCHAR *str2);
BASE_EXPORT int    ENGINEAPI   scmp_n(const TCHAR *str1, const TCHAR *str2, unsigned int num);
BASE_EXPORT int    ENGINEAPI   scmpi_n(const TCHAR *str1, const TCHAR *str2, unsigned int num);
BASE_EXPORT TCHAR* ENGINEAPI   srchr(const TCHAR *strSrc, TCHAR chr);
BASE_EXPORT TCHAR* ENGINEAPI   schr(const TCHAR *strSrc, TCHAR chr);
BASE_EXPORT TCHAR* ENGINEAPI   sstr(const TCHAR *strSrc, const TCHAR *strSearch);
BASE_EXPORT TCHAR* ENGINEAPI   srchri(const TCHAR *strSrc, TCHAR chr);
BASE_EXPORT TCHAR* ENGINEAPI   schri(const TCHAR *strSrc, TCHAR chr);
BASE_EXPORT TCHAR* ENGINEAPI   sstri(const TCHAR *strSrc, const TCHAR *strSearch);
BASE_EXPORT TCHAR* ENGINEAPI   sfix(TCHAR *str);

BASE_EXPORT BOOL ENGINEAPI ValidFloatString(CTSTR lpStr);
BASE_EXPORT BOOL ENGINEAPI ValidIntString(CTSTR lpStr);
BASE_EXPORT BOOL DefinitelyFloatString(CTSTR lpStr);

BASE_EXPORT BOOL            UsingUnicode();
BASE_EXPORT int             tstr_to_utf8(const TCHAR *strSrc, char *strDest, size_t destLen);
BASE_EXPORT int             tstr_to_wide(const TCHAR *strSrc, wchar_t *strDest, size_t destLen);
BASE_EXPORT int             tstr_to_utf8_datalen(const TCHAR *strSrc);
BASE_EXPORT int             tstr_to_wide_datalen(const TCHAR *strSrc);
BASE_EXPORT int             utf8_to_tchar_datalen(const char *strSrc);
BASE_EXPORT LPSTR           tstr_createUTF8(const TCHAR *strSrc);
BASE_EXPORT WSTR            tstr_createWide(const TCHAR *strSrc);
BASE_EXPORT TSTR            utf8_createTstr(const char *strSrc);

BASE_EXPORT int             vtscprintf(const TCHAR *format, va_list args);
BASE_EXPORT int             vtsprintf_s(TCHAR *pDest, size_t destLen, const TCHAR *format, va_list args);
BASE_EXPORT int             tsprintf_s(TCHAR *pDest, size_t destLen, const TCHAR *format, ...);

BASE_EXPORT int             itots_s(int val, TCHAR *buffer, size_t bufLen, int radix);
BASE_EXPORT int             uitots_s(unsigned int val, TCHAR *buffer, size_t bufLen, int radix);
BASE_EXPORT int             i64tots_s(INT64 val, TCHAR *buffer, size_t bufLen, int radix);
BASE_EXPORT int             ui64tots_s(UINT64 val, TCHAR *buffer, size_t bufLen, int radix);
BASE_EXPORT int             tstring_base_to_int(const TCHAR *nptr, TCHAR **endptr, int base);
BASE_EXPORT unsigned int    tstring_base_to_uint(const TCHAR *nptr, TCHAR **endptr, int base);
BASE_EXPORT INT64           tstring_base_to_int64(const TCHAR *nptr, TCHAR **endptr, int base);
BASE_EXPORT UINT64          tstring_base_to_uint64(const TCHAR *nptr, TCHAR **endptr, int base);
BASE_EXPORT double          tstof(TCHAR *lpFloat);
BASE_EXPORT int             tstoi(TCHAR *lpInt);
BASE_EXPORT unsigned int    tstoui(TCHAR *lpInt);
BASE_EXPORT INT64           tstoi64(TCHAR *lpInt);
BASE_EXPORT UINT64          tstoui64(TCHAR *lpInt);

class StringList;

class BASE_EXPORT String
{
    TSTR lpString;
    unsigned int curLength;

public:
    String();
    String(LPCSTR str);
    String(CWSTR str);
    String(const String &str);

    ~String();

    String& operator=(CTSTR str);
    String& operator+=(CTSTR str);
    String  operator+(CTSTR str) const;

    String& operator=(TCHAR ch);
    String& operator+=(TCHAR ch);
    String  operator+(TCHAR ch) const;

    String& operator=(int number);
    String& operator+=(int number);
    String  operator+(int number) const;

    String& operator=(unsigned int unumber);
    String& operator+=(unsigned int unumber);
    String  operator+(unsigned int unumber) const;

    String& operator=(const String &str);
    String& operator+=(const String &str);
    String  operator+(const String &str) const;

    String& operator<<(CTSTR str);
    String& operator<<(TCHAR ch);
    String& operator<<(int number);
    String& operator<<(unsigned int unumber);
    String& operator<<(const String &str);

    inline BOOL operator==(const String &str) const {return Compare(str);}
    inline BOOL operator!=(const String &str) const {return !Compare(str);}

    inline BOOL operator==(CTSTR lpStr) const {return Compare(lpStr);}
    inline BOOL operator!=(CTSTR lpStr) const {return !Compare(lpStr);}

    BOOL    Compare(CTSTR str)    const;
    BOOL    CompareI(CTSTR str)   const;

    LPSTR   CreateUTF8String();

    String& FindReplace(CTSTR strFind, CTSTR strReplace);

    void    InsertString(UINT dwPos, CTSTR str);

    String& AppendString(CTSTR str, UINT count=0);

    UINT    GetLinePos(UINT dwLine);

    void    RemoveRange(unsigned int from, unsigned int to);

    String& AppendChar(TCHAR chr);
    String& InsertChar(int pos, TCHAR chr);
    String& RemoveChar(int pos);

    String& Clear();

    String& GroupDigits();

    int     NumTokens(TCHAR token=' ') const;
    String  GetToken(int id, TCHAR token=' ') const;
    void    GetTokenList(StringList &strList, TCHAR token=' ', BOOL bIncludeEmpty=TRUE) const;
    CTSTR   GetTokenOffset(int token, TCHAR seperator=' ') const;

    String  Left(int iEnd);
    String  Mid(int iStart, int iEnd);
    String  Right(int iStart);

    inline int     ToInteger(int base=10) const {return tstring_base_to_int(lpString, NULL, base);}
    inline int     ToInt(int base=10) const     {return tstring_base_to_int(lpString, NULL, base);}
    inline float   ToFloat() const              {return (float)tstof(lpString);}

    inline BOOL    IsEmpty() const              {return !lpString || !*lpString || curLength == 0;}
    inline BOOL    IsValid() const              {return !IsEmpty();}

    inline String& KillSpaces()                 {curLength = slen(sfix(lpString)); return *this;}

    inline TSTR Array() const                   {return lpString;}

    inline operator TSTR() const                {return lpString;}

    String& SetLength(UINT length);

    inline UINT    Length() const               {return curLength;}
    inline UINT    DataLength() const           {return curLength ? ssize(lpString) : 0;}

    inline String& MakeLower()                  {if(lpString) slwr(lpString); return *this;}
    inline String& MakeUpper()                  {if(lpString) supr(lpString); return *this;}

    inline String GetLower() const              {return String(*this).MakeLower();}
    inline String GetUpper() const              {return String(*this).MakeUpper();}

    static String RepresentationToString(String &stringToken);
    static String StringToRepresentation(String &string);
    BASE_EXPORT friend Serializer& operator<<(Serializer &s, String &str);
};

BASE_EXPORT String FormattedStringva(CTSTR lpFormat, va_list arglist);
BASE_EXPORT String FormattedString(CTSTR lpFormat, ...);
WORD StringCRC16(CTSTR lpData);
WORD StringCRC16I(CTSTR lpData);

class BASE_EXPORT StringList : public List<String>
{
public:
    inline ~StringList()
    {
        for(int i=0; i<num; i++)
            array[i].Clear();
    }

    inline void Clear()
    {
        for(int i=0; i<num; i++)
            array[i].Clear();
        List<String>::Clear();
    }

    inline unsigned int Add(const String &val)
    {
        *CreateNew() = val;

        return num-1;
    }

    inline unsigned int Add(CTSTR lpStr)
    {
        *CreateNew() = lpStr;

        return num-1;
    }

    inline void Insert(unsigned int index, const String &val)
    {
        *InsertNew(index) = val;
    }

    inline void Insert(unsigned int index, CTSTR lpStr)
    {
        *InsertNew(index) = lpStr;
    }


    inline unsigned int AddSorted(CTSTR lpStr)
    {
        UINT i;
        for(i=0; i<num; i++)
        {
            if(scmpi(array[i], lpStr) >= 0)
                break;
        }

        Insert(i, lpStr);
        return i;
    }


    unsigned int SafeAdd(const String& val)
    {
        UINT i;
        for(i=0; i<num; i++)
        {
            if(array[i].Compare(val))
                break;
        }

        return (i != num) ? i : Add(val);
    }

    unsigned int SafeAdd(CTSTR lpStr)
    {
        UINT i;
        for(i=0; i<num; i++)
        {
            if(array[i].Compare(lpStr))
                break;
        }

        return (i != num) ? i : Add(lpStr);
    }

    unsigned int SafeAddI(const String& val)
    {
        UINT i;
        for(i=0; i<num; i++)
        {
            if(array[i].CompareI(val))
                break;
        }

        return (i != num) ? i : Add(val);
    }

    unsigned int SafeAddI(CTSTR lpStr)
    {
        UINT i;
        for(i=0; i<num; i++)
        {
            if(array[i].CompareI(lpStr))
                break;
        }

        return (i != num) ? i : Add(lpStr);
    }


    inline void Remove(unsigned int index)
    {
        array[index].Clear();
        List<String>::Remove(index);
    }

    inline void RemoveItem(const String& str)
    {
        for(UINT i=0; i<num; i++)
        {
            if(array[i].Compare(str))
            {
                Remove(i);
                break;
            }
        }
    }

    inline void RemoveItem(CTSTR lpStr)
    {
        for(UINT i=0; i<num; i++)
        {
            if(array[i].Compare(lpStr))
            {
                Remove(i);
                break;
            }
        }
    }

    inline void RemoveItemI(const String& str)
    {
        for(UINT i=0; i<num; i++)
        {
            if(array[i].CompareI(str))
            {
                Remove(i);
                break;
            }
        }
    }

    inline void RemoveItemI(CTSTR lpStr)
    {
        for(UINT i=0; i<num; i++)
        {
            if(array[i].CompareI(lpStr))
            {
                Remove(i);
                break;
            }
        }
    }

    inline void CopyArray(const String *new_array, unsigned int n)
    {
        if(!new_array && n)
        {
            AppWarning(TEXT("List::CopyArray:  NULL array with count above zero"));
            return;
        }

        Clear();

        if(n)
        {
            SetSize(n);

            for(int i=0; i<n; i++)
                array[i] = new_array[i];
        }
    }

    inline void InsertArray(unsigned int index, const String *new_array, unsigned int n)
    {
        assert(index <= num);

        if(index > num)
            return;

        if(!new_array && n)
        {
            AppWarning(TEXT("List::AppendArray:  NULL array with count above zero"));
            return;
        }

        if(!n)
            return;

        int oldnum = num;

        SetSize(n+num);

        mcpyrev(array+index+n, array+index, sizeof(String)*(oldnum-index));
        zero(array+index, sizeof(String)*n);

        for(int i=0; i<n; i++)
            array[index+i] = new_array[i];
    }

    inline void AppendArray(const String *new_array, unsigned int n)
    {
        if(!new_array && n)
        {
            AppWarning(TEXT("List::AppendArray:  NULL array with count above zero"));
            return;
        }

        if(!n)
            return;

        int oldnum = num;

        SetSize(n+num);

        for(int i=0; i<n; i++)
            array[oldnum+i] = new_array[i];
    }

    inline BOOL SetSize(unsigned int n)
    {
        if(n < num)
        {
            for(int i=n; i<num; i++)
                array[i].Clear();
        }

        return List<String>::SetSize(n);
    }

    inline void RemoveDupes()
    {
        for(UINT i=0; i<num; i++)
        {
            String val1 = array[i];
            for(UINT j=i+1; j<num; j++)
            {
                String val2 = array[j];
                if(val1.Compare(val2))
                {
                    Remove(j);
                    --j;
                }
            }
        }
    }

    inline void RemoveDupesI()
    {
        for(UINT i=0; i<num; i++)
        {
            String val1 = array[i];
            for(UINT j=i+1; j<num; j++)
            {
                String val2 = array[j];
                if(val1.CompareI(val2))
                {
                    Remove(j);
                    --j;
                }
            }
        }
    }

    inline void CopyList(const StringList& list)
    {
        CopyArray(list.Array(), list.Num());
    }

    inline void InsertList(unsigned int index, const StringList& list)
    {
        InsertArray(index, list.Array(), list.Num());
    }

    inline void AppendList(const StringList& list)
    {
        AppendArray(list.Array(), list.Num());
    }

    inline StringList& operator<<(const String& val)
    {
        Add(val);
        return *this;
    }

    inline StringList& operator<<(CTSTR lpStr)
    {
        Add(lpStr);
        return *this;
    }

    inline BOOL HasValueI(const String& val) const
    {
        for(UINT i=0; i<num; i++)
            if(array[i].CompareI(val)) return 1;

        return 0;
    }

    inline BOOL HasValueI(CTSTR lpStr) const
    {
        for(UINT i=0; i<num; i++)
            if(array[i].CompareI(lpStr)) return 1;

        return 0;
    }

    inline UINT FindValueIndexI(const String& val) const
    {
        for(UINT i=0; i<num; i++)
            if(array[i].CompareI(val)) return i;

        return INVALID;
    }

    inline UINT FindValueIndexI(CTSTR lpStr) const
    {
        for(UINT i=0; i<num; i++)
            if(array[i].CompareI(lpStr)) return i;

        return INVALID;
    }

    inline UINT FindNextValueIndexI(const String& val, UINT lastIndex=INVALID) const
    {
        for(UINT i=lastIndex+1; i<num; i++)
            if(array[i].CompareI(val)) return i;

        return INVALID;
    }

    inline UINT FindNextValueIndexI(CTSTR lpStr, UINT lastIndex=INVALID) const
    {
        for(UINT i=lastIndex+1; i<num; i++)
            if(array[i].CompareI(lpStr)) return i;

        return INVALID;
    }

    inline friend Serializer& operator<<(Serializer &s, StringList &list)
    {
        if(s.IsLoading())
        {
            UINT num;
            s << num;
            list.SetSize(num);
            for(int i=0; i<num; i++)
                s << list[i];
        }
        else
        {
            UINT num = list.Num();
            s << num;
            for(int i=0; i<num; i++)
                s << list[i];
        }

        return s;
    }
};

BASE_EXPORT String UIntString(unsigned int ui, int radix=10);
BASE_EXPORT String IntString(int i, int radix=10);
BASE_EXPORT String Int64String(INT64 i, int radix);
BASE_EXPORT String FloatString(double f);

BASE_EXPORT int ENGINEAPI GetStringLine(const TCHAR *lpStart, const TCHAR *lpOffset);

BASE_EXPORT String ENGINEAPI GetStringSection(const TCHAR *lpStart, const TCHAR *lpOffset);



#endif
