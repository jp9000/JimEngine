/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  XConfig.h - a JSON-esque data storage thingamabopper

  (c) 2038 by Jip
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#pragma once


enum
{
    XConfig_Data,
    XConfig_Element
};

class BASE_EXPORT XBaseItem
{
    friend class XElement;
    friend class XConfig;

protected:
    XBaseItem(int type, CTSTR lpName) : type(type), strName(lpName) {}

    virtual ~XBaseItem() {}

    int type;

public:
    String strName;

    inline int GetType() const     {return type;}
    inline bool IsData() const     {return type == XConfig_Data;}
    inline bool IsElement() const  {return type == XConfig_Element;}
};


class BASE_EXPORT XDataItem : public XBaseItem
{
    friend class XElement;
    friend class XConfig;

    inline XDataItem(CTSTR lpName, CTSTR lpData)
        : XBaseItem(XConfig_Data, lpName), strData(lpData)
    {}

public:
    String strData;
};


class BASE_EXPORT XElement : public XBaseItem
{
    friend class XConfig;

    XConfig *file;

    XElement *parent;
    List<XBaseItem*> SubItems;

    inline XElement(XConfig *XConfig, XElement *parentElement, CTSTR lpName)
        : XBaseItem(XConfig_Element, lpName), parent(parentElement), file(XConfig)
    {}

protected:
    ~XElement();

public:

    CTSTR GetString(CTSTR lpName, CTSTR def=NULL) const;
    int   GetInt(CTSTR lpName, int def=0) const;
    float GetFloat(CTSTR lpName, float def=0.0f) const;
    inline bool  GetBool(CTSTR lpName, bool bDef=false) const
    {
        return scmpi(GetString(lpName, bDef ? TEXT("true") : TEXT("false")), TEXT("true")) == 0;
    }
    inline DWORD GetColor(CTSTR lpName, DWORD def=0) const
    {
        return (DWORD)GetInt(lpName, (int)def);
    }
    inline DWORD GetHex(CTSTR lpName, DWORD def=0) const
    {
        return (DWORD)GetInt(lpName, (int)def);
    }

    void  GetStringList(CTSTR lpName, StringList &stringList) const;
    void  GetIntList(CTSTR lpName, List<int> &IntList) const;
    void  GetFloatList(CTSTR lpName, List<float> &FloatList) const;
    inline void GetColorList(CTSTR lpName, List<DWORD> &ColorList) const
    {
        GetIntList(lpName, *(List<int>*)&ColorList);
    }
    inline void GetHexList(CTSTR lpName, List<DWORD> &HexList) const
    {
        GetIntList(lpName, *(List<int>*)&HexList);
    }

    void  SetString(CTSTR lpName, CTSTR lpString);
    void  SetInt(CTSTR lpName, int number);
    void  SetFloat(CTSTR lpName, float number);
    void  SetHex(CTSTR lpName, DWORD hex);
    inline void SetBool(CTSTR lpName, bool bVal)
    {
        SetString(lpName, bVal ? TEXT("true") : TEXT("false"));
    }
    inline void SetColor(CTSTR lpName, DWORD color)
    {
        SetHex(lpName, color);
    }

    void  SetStringList(CTSTR lpName, List<TSTR> &StringList);
    void  SetIntList(CTSTR lpName, List<int> &IntList);
    void  SetFloatList(CTSTR lpName, List<float> &FloatList);
    void  SetHexList(CTSTR lpName, List<DWORD> &HexList);
    inline void SetColorList(CTSTR lpName, List<DWORD> &ColorList)
    {
        SetHexList(lpName, ColorList);
    }

    void  AddString(CTSTR lpName, CTSTR lpString);
    void  AddInt(CTSTR lpName, int number);
    void  AddFloat(CTSTR lpName, float number);
    void  AddHex(CTSTR lpName, DWORD hex);
    inline void AddColor(CTSTR lpName, DWORD color)
    {
        AddHex(lpName, color);
    }

    void  AddStringList(CTSTR lpName, List<TSTR> &StringList);
    void  AddIntList(CTSTR lpName, List<int> &IntList);
    void  AddFloatList(CTSTR lpName, List<float> &FloatList);
    void  AddHexList(CTSTR lpName, List<DWORD> &HexList);
    inline void AddColorList(CTSTR lpName, List<DWORD> &ColorList)
    {
        AddHexList(lpName, ColorList);
    }

    void  RemoveItem(CTSTR lpName);

    //----------------

    XElement* GetElement(CTSTR lpName) const;
    XElement* GetElementByID(DWORD elementID) const;
    XElement* GetElementByItem(CTSTR lpName, CTSTR lpItemName, CTSTR lpItemValue) const;
    XElement* CreateElement(CTSTR lpName);
    void  GetElementList(CTSTR lpName, List<XElement*> &Elements) const;
    void  RemoveElement(XElement *element);
    void  RemoveElement(CTSTR lpName);

    XBaseItem* GetBaseItem(CTSTR lpName) const;
    XBaseItem* GetBaseItemByID(DWORD itemID) const;

    XDataItem* GetDataItem(CTSTR lpName) const;
    XDataItem* GetDataItemByID(DWORD itemID) const;

    DWORD NumElements(CTSTR lpName=NULL);
    DWORD NumBaseItems(CTSTR lpName=NULL);
    DWORD NumDataItems(CTSTR lpName=NULL);
};


class BASE_EXPORT XConfig
{
    XElement *RootElement;
    String strFileName;

    bool ReadFileData(XElement *curElement, TSTR &lpFileData);
    void WriteFileData(XFile &file, int indent, XElement *curElement);

    static String ConvertToTextString(String &string);
    static String ProcessString(TSTR &lpTemp);

public:
    inline XConfig() : RootElement(NULL) {}
    inline XConfig(TSTR lpFile) : RootElement(NULL) {Open(lpFile);}

    inline ~XConfig() {Close();}

    bool    Open(CTSTR lpFile, bool bNew=false);
    void    Close(bool bSave=false);

    inline XElement *GetRootElement() {return RootElement;}
    inline XElement *GetElement(CTSTR lpName) {return RootElement->GetElement(lpName);}
};

