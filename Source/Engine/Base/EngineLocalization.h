
#ifndef LOCALIZATION_HEADER
#define LOCALIZATION_HEADER


//------------------------------------------------------------------
// Localization structures
//------------------------------------------------------------------

struct LocaleStringItem
{
    String      lookup;
    StringList  strings;
};

struct LocaleStringCache : public List<LocaleStringItem*>
{
    inline ~LocaleStringCache()
    {
        for(int i=0; i<Num(); i++)
            delete array[i];
    }

    inline void Clear()
    {
        for(int i=0; i<Num(); i++)
            delete array[i];
        List<LocaleStringItem*>::Clear();
    }

    inline void Remove(UINT i)
    {
        if(i < Num())
        {
            delete array[i];
            List<LocaleStringItem*>::Remove(i);
        }
    }

    inline void SetSize(UINT newSize)
    {
        if(newSize < Num())
        {
            for(int i=newSize; i<Num(); i++)
                delete array[i];
        }

        List<LocaleStringItem*>::SetSize(newSize);
    }
};


struct StringLookupNode;

struct LocaleStringFile
{
    String name;
    LocaleStringCache cache;

    inline void FreeData() {name.Clear(); cache.Clear();}
};


//------------------------------------------------------------------
// Localization String Lookup Class
//------------------------------------------------------------------

class BASE_EXPORT LocaleStringLookup
{
    String curLanguage;
    StringLookupNode *top;
    List<LocaleStringFile> loadedFiles;

    BOOL AddLookup(CTSTR lookupVal, LocaleStringItem *item, StringLookupNode *node=NULL);
    void RemoveLookup(CTSTR lookupVal, StringLookupNode *node=NULL);

    UINT GetLocaleFileID(CTSTR lpResource);

public:
    LocaleStringLookup();
    ~LocaleStringLookup();

    inline CTSTR GetLanguage() const {return curLanguage;}

    BOOL LoadStringFile(CTSTR lpResource);
    void UnloadStringFile(CTSTR lpResource);

    CTSTR LookupString(CTSTR lookupVal, StringLookupNode *node=NULL);
    BOOL  LookupStringSequence(CTSTR lookupVal, List<CTSTR> &strSequence, StringLookupNode *node=NULL);
};


BASE_EXPORT extern LocaleStringLookup *locale;

#define Str(text) locale->LookupString(TEXT2(text))
inline BOOL LookupStringSequence(CTSTR lookupVal, List<CTSTR> &strSequence) {return locale->LookupStringSequence(lookupVal, strSequence);}

inline BOOL  LoadStringFile(CTSTR lpResource)   {return locale->LoadStringFile(lpResource);}
inline void  UnloadStringFile(CTSTR lpResource) {locale->UnloadStringFile(lpResource);}
inline CTSTR GetLanguage()                      {return locale->GetLanguage();}



#endif

