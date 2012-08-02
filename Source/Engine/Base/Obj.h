/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Obj.h:  Object Engine

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

#ifndef OBJ_HEADER
#define OBJ_HEADER



/*========================================================
  Class Macros
=========================================================*/

#define DeclareClass(cls, super_class) \
    friend class Class; \
    private: \
    static Class class_id; \
    public: \
    typedef super_class Super; \
    static  Object* ENGINEAPI CreateNew(Class *targetClass=cls::GetLocalClass()); \
    static  Class* GetLocalClass()      {return &class_id;} \
    virtual Class* _GetInteralClass()   {return &class_id;} \
    private:


#define DefineClass(cls)  \
    Class cls::class_id(TEXT(#cls), cls::Super::GetLocalClass(), FALSE, cls::CreateNew, sizeof(cls)); \
    Object* ENGINEAPI cls::CreateNew(Class *targetClass) {void* test1234 = Allocate(targetClass->GetClassSize()); zero(test1234, targetClass->GetClassSize()); return new(test1234) cls;}


#define DefineAbstractClass(cls)  \
    Class cls::class_id(TEXT(#cls), cls::Super::GetLocalClass(), TRUE, cls::CreateNew, sizeof(cls)); \
    Object* ENGINEAPI cls::CreateNew(Class *targetClass) {AppWarning(TEXT("Tried to create abstract class %s"), TEXT(#cls)); return NULL;}


#define CLASS_DECLARE           DeclareClass
#define CLASS_DEFINE            DefineClass
#define CLASS_DEFINE_ABSTRACT   DefineAbstractClass


/*=========================================================
    Object Class
==========================================================*/

struct ObjectScript;


class BASE_EXPORT Object
{
    friend class Class;
    friend class ObjectPropertiesEditor;
    friend class ScriptEditor;
    friend class FrameObject;
    friend struct FunctionDefinition;

    int refCounter;

    static Class class_id;
    Class *topClass;

    ObjectScript *objScript;

    static Object* pFirstObject;
    static Object* pLastObject;
    Object *pNextObject, *pPrevObject;

public:
    static Object* ENGINEAPI CreateNew(Class *targetClass=Object::GetLocalClass());

    Object();
    virtual ~Object();

    inline int AddReference()           {return (++refCounter)+1;}
    inline int GetRefCount()            {return refCounter+1;}
    int Release();

    //class functions
    static  Class* GetLocalClass()      {return &class_id;}
    virtual Class* _GetInteralClass()   {return &class_id;}

    Class* GetObjectClass()             {return topClass;}

    virtual BOOL IsOf(TSTR lpClass)     {Class *cls = FindClass(lpClass); return IsOf(cls);}
    BOOL IsOf(Class *cls)               {assert(topClass); if(!topClass) return FALSE; return topClass->_IsOf(cls);}

    virtual void Init()                 {scriptInit();}
    virtual void Destroy()              {scriptDestroy();}

    virtual void Reinitialize()         {scriptReinitialize();}  //used in editing

    virtual void Serialize(Serializer &s);

    inline BOOL GetScriptBool  (CTSTR lpName,   BOOL&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; val = *(BOOL*)ptr;    return TRUE;}
    inline BOOL GetScriptInt   (CTSTR lpName,    int&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; val = *(int*)ptr;     return TRUE;}
    inline BOOL GetScriptFloat (CTSTR lpName,  float&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; val = *(float*)ptr;   return TRUE;}
    inline BOOL GetScriptString(CTSTR lpName, String&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; val = *(String*)ptr;  return TRUE;}
    inline BOOL GetScriptObject(CTSTR lpName, Object*& val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; val = *(Object**)ptr; return TRUE;}
    inline BOOL GetScriptVect2 (CTSTR lpName,  Vect2&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; val = *(Vect2*)ptr;   return TRUE;}
    inline BOOL GetScriptVect  (CTSTR lpName,   Vect&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; val = *(Vect*)ptr;    return TRUE;}
    inline BOOL GetScriptQuat  (CTSTR lpName,   Quat&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; val = *(Quat*)ptr;    return TRUE;}
    inline BOOL GetScriptColor4(CTSTR lpName, Color4&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; val = *(Color4*)ptr;  return TRUE;}
    inline BOOL GetScriptBounds(CTSTR lpName, Bounds&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; val = *(Bounds*)ptr;  return TRUE;}
    inline BOOL GetScriptPlane (CTSTR lpName,  Plane&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; val = *(Plane*)ptr;   return TRUE;}
    inline BOOL GetScriptMatrix(CTSTR lpName, Matrix&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; val = *(Matrix*)ptr;  return TRUE;}

    inline BOOL SetScriptBool  (CTSTR lpName,   BOOL   val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; *(BOOL*)ptr    = val; return TRUE;}
    inline BOOL SetScriptInt   (CTSTR lpName,    int   val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; *(int*)ptr     = val; return TRUE;}
    inline BOOL SetScriptFloat (CTSTR lpName,  float   val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; *(float*)ptr   = val; return TRUE;}
    inline BOOL SetScriptString(CTSTR lpName, String&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; *(String*)ptr  = val; return TRUE;}
    inline BOOL SetScriptObject(CTSTR lpName, Object*  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; *(Object**)ptr = val; return TRUE;}
    inline BOOL SetScriptVect2 (CTSTR lpName,  Vect2&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; *(Vect2*)ptr   = val; return TRUE;}
    inline BOOL SetScriptVect  (CTSTR lpName,   Vect&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; *(Vect*)ptr    = val; return TRUE;}
    inline BOOL SetScriptQuat  (CTSTR lpName,   Quat&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; *(Quat*)ptr    = val; return TRUE;}
    inline BOOL SetScriptColor4(CTSTR lpName, Color4&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; *(Color4*)ptr  = val; return TRUE;}
    inline BOOL SetScriptBounds(CTSTR lpName, Bounds&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; *(Bounds*)ptr  = val; return TRUE;}
    inline BOOL SetScriptPlane (CTSTR lpName,  Plane&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; *(Plane*)ptr   = val; return TRUE;}
    inline BOOL SetScriptMatrix(CTSTR lpName, Matrix&  val)  {void* ptr = GetScriptPtr(lpName); if(!ptr) return FALSE; *(Matrix*)ptr  = val; return TRUE;}

    void* GetScriptPtr(CTSTR lpName);

    inline BOOL GetScriptColor(CTSTR lpName, DWORD& color) {return GetScriptInt(lpName, (int&)color);}
    inline BOOL SetScriptColor(CTSTR lpName, DWORD  color) {return SetScriptInt(lpName, (int)color);}

    //object list
    static void DestroyAll();
    static Object* FirstObject() {return pFirstObject;}
    static Object* LastObject()  {return pLastObject;}
    Object* PrevObject()         {return pPrevObject;}
    Object* NextObject()         {return pNextObject;}

    //-----------------------------------------
    static Object *CreateFactoryObject(CTSTR lpClass, BOOL bInitialize=TRUE);
    static void DestroyObject(Object* obj);
    Object *InitializeObject(BOOL bCallScriptConstructors=FALSE);

    //<Script module="Base" classdecs="Object">
    void scriptInit()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 0, cs);
    }

    void scriptDestroy()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 1, cs);
    }

    void scriptReinitialize()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 7, cs);
    }

    Declare_Internal_Member(native_AddReference);
    Declare_Internal_Member(native_GetRefCount);
    Declare_Internal_Member(native_Release);
    Declare_Internal_Member(native_IsOf);
    Declare_Internal_Member(native_InitializeObject);
    //</Script>
};

//<Script module="Base" globaldecs="Object.xscript">
Declare_Native_Global(NativeGlobal_CreateFactoryObject);
//</Script>


inline float MSToSeconds(DWORD dwMS)  {return (float(dwMS)*0.001);}


//-----------------------------------------
//C defs
inline void DestroyObject(Object* obj)                                                      {Object::DestroyObject(obj);}
inline Object *CreateFactoryObject(CTSTR lpClass, BOOL bInitialize=TRUE)                    {return Object::CreateFactoryObject(lpClass, bInitialize);}
inline Object *InitializeObjectData(Object* obj, BOOL bCallScriptConstructors=FALSE)        {return obj->InitializeObject(bCallScriptConstructors);}

#define CreateObject(t)                                 ((t*)InitializeObjectData((Object*)new t, TRUE))
#define CreateObjectParam(t, param)                     ((t*)InitializeObjectData((Object*)new t(param), TRUE))
#define CreateObjectParam2(t, param1, param2)           ((t*)InitializeObjectData((Object*)new t(param1, param2), TRUE))
#define CreateObjectParam3(t, param1, param2, param3)   ((t*)InitializeObjectData((Object*)new t(param1, param2, param3), TRUE))
#define CreateObjectParam4(t, param1, param2, param3, param4) ((t*)InitializeObjectData((Object*)new t(param1, param2, param3, param4), TRUE))
#define CreateObjectParam5(t, param1, param2, param3, param4, param5) ((t*)InitializeObjectData((Object*)new t(param1, param2, param3, param4, param5), TRUE))
#define CreateBare(t)                                   ((t*)InitializeObjectData((Object*)new t, FALSE))
#define GetClass(t)                                     t::GetLocalClass()
#define TerminateObjectEngine()                         Object::DestroyAll()

template<typename T> inline T* ObjectCast(Object *obj)
{
    return (obj && obj->IsOf(GetClass(T))) ? static_cast<T*>(obj) : NULL;
}



/*=========================================================
    SerializerObject
==========================================================*/

class SerializerObject : public Object
{
    DeclareClass(SerializerObject, Object);

    BOOL bDestroy;
    Serializer *s;

public:
    inline SerializerObject() {}
    SerializerObject(Serializer *sIn, BOOL bDestroyIn);
    ~SerializerObject();

    //<Script module="Base" classdecs="SerializerObject">
    Declare_Internal_Member(native_SerializeChar);
    Declare_Internal_Member(native_SerializeByte);
    Declare_Internal_Member(native_SerializeShort);
    Declare_Internal_Member(native_SerializeWord);
    Declare_Internal_Member(native_SerializeInt);
    Declare_Internal_Member(native_SerializeFloat);
    Declare_Internal_Member(native_SerializeDouble);
    Declare_Internal_Member(native_SerializeString);
    Declare_Internal_Member(native_SerializeObject);
    Declare_Internal_Member(native_IsLoading);
    Declare_Internal_Member(native_GetPos);
    Declare_Internal_Member(native_Seek);
};

//<Script module="Base" globaldecs="Serialization.xscript">
Declare_Native_Global(NativeGlobal_CreateFileInputSerializer);
Declare_Native_Global(NativeGlobal_CreateFileOutputSerializer);
//</Script>


/*=========================================================
    Renderable
==========================================================*/

class BASE_EXPORT FrameObject : public Object
{
    friend class Level;
    friend class FrameObject;
    friend class Engine;

    DeclareClass(FrameObject, Object);

    BOOL bMarkedForDestruction;

    static FrameObject* pFirstFrameObject;
    static FrameObject* pLastFrameObject;
    FrameObject *pNextFrameObject, *pPrevFrameObject;

public:
    FrameObject();
    virtual ~FrameObject();

    //render functions
    virtual void PreRender()                      {scriptPreRender();}
    virtual void Render()                         {scriptRender();}
    virtual void PostRender()                     {scriptPostRender();}

    //frame functions
    virtual void Tick(float fSeconds)             {scriptTick(fSeconds);}
    virtual void PreFrame()                       {scriptPreFrame();}

    //Safe deletion
    int  SafeRelease();
    void SafeDestroy();

    //frame object list
    static FrameObject* FirstFrameObject() {return pFirstFrameObject;}
    static FrameObject* LastFrameObject()  {return pLastFrameObject;}
    FrameObject* PrevFrameObject()         {return pPrevFrameObject;}
    FrameObject* NextFrameObject()         {return pNextFrameObject;}

    //<Script module="Base" classdecs="FrameObject">
    BOOL bRenderable;

    void scriptPreFrame()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 0, cs);
    }

    void scriptPreRender()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 1, cs);
    }

    void scriptRender()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 2, cs);
    }

    void scriptPostRender()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 3, cs);
    }

    void scriptTick(float fSeconds)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetFloat(0, fSeconds);

        GetLocalClass()->CallScriptMember(this, 4, cs);
    }

    Declare_Internal_Member(native_SafeRelease);
    Declare_Internal_Member(native_SafeDestroy);
    //</Script>
};



#endif
