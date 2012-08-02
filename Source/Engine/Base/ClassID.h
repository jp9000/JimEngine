/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ClassID.h:  Class Identification System

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

#ifndef CLASSID_HEADER
#define CLASSID_HEADER



/*=========================================================
    Class
==========================================================*/

struct ClassDefinition;
struct FunctionDefinition;
class CallStruct;
class Class;
class GameModule;

typedef Object* (ENGINEAPI *CREATENEWPROC)(Class*);
typedef void (ENGINEAPI *DEFFUNCPROC)();
typedef void (ENGINEAPI *NATIVECALLBACK)(CallStruct&);
typedef void (Object::*OBJECTCALLBACK)(CallStruct&);

typedef void (ENGINEAPI *NATIVELOADERCALLBACK)();

class BASE_EXPORT Class
{
    friend class Object;
    friend class ScriptSystem;
    friend class ObjectPropertiesEditor;
    friend class ScriptEditor;
    friend class Engine;
    friend class EditorEngine;
    friend struct Compiler;
    friend struct ObjectScript;
    friend struct ClassDefinition;
    friend struct FunctionDefinition;

public:
    Class(const TCHAR *lpName, Class *parent_class, BOOL bAbstract, CREATENEWPROC createproc, DWORD dwClassSize);
    ~Class();

    BOOL _IsOf(Class *cls) const;

    static Class *_FindClass(const TCHAR *lpName, const TCHAR *lpModule=NULL);
    static void _FindClassesOf(const TCHAR *lpName, List<Class*> &classes);
    static void GetClassChildren(Class* cls, List<Class*> &classes);

    inline Class* GetParent() const {return Parent;}

    inline BOOL IsAbstract() const {return isAbstract;}

    void DefineNativeMember(OBJECTCALLBACK func, int id);
    void DefineNativeStaticMember(NATIVECALLBACK func, int id);
    void DefineNativeVariable(size_t offset, int id);

    void CallScriptMember(Object *obj, unsigned int relativeID, CallStruct &cs);

    inline CTSTR GetName() {return name;}
    inline CTSTR GetModule() {return module;}

    inline DWORD GetClassSize() {return classSize;}

    inline BOOL IsPureScriptClass() const {return bPureScriptClass;}

    Object *Create(BOOL bInitialize=TRUE);

    inline static Class* FirstClass() {return lpFirst;}
    inline static Class* LastClass() {return lpLast;}
    inline Class* NextClass() {return lpNext;}
    inline Class* PrevClass() {return lpPrev;}

private:
    CREATENEWPROC pCreate;

    const TCHAR *name;
    TCHAR module[64];

    BOOL bFreeManually;

    BOOL isAbstract;

    Class *Parent;


    //script stuff
    BOOL bPureScriptClass;
    UINT initializationLevel;
    ClassDefinition *scriptClass;
    unsigned int classSize;

    void* GetScriptVarAddress(Object *obj, int pos);
    FunctionDefinition* GetTopmostFunction(int pos);
    void CallScriptConstructors(Object *obj);
    void CallClassConstructor(Object *obj, ClassDefinition *classDef);

    ClassDefinition* GetScriptClass();

    //linked list
    Class *lpNext;
    Class *lpPrev;
    static Class *lpFirst;
    static Class *lpLast;
};

//////////////
//defines
#define FindClass                Class::_FindClass
#define FindClassesOf            Class::_FindClassesOf



#endif
