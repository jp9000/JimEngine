#include "..\Base.h"
#include "GlobalNatives.h"


//--------------------------------------

void SerializerObject::native_SerializeChar(CallStruct &cs)
{
    assert(s);
    if(!s) return;

    int& val = cs.GetIntOut(0);

    char ch = (char)val;
    *s << ch;
    val = (int)ch;
}

void SerializerObject::native_SerializeByte(CallStruct &cs)
{
    assert(s);
    if(!s) return;

    int& val = cs.GetIntOut(0);

    BYTE b = (BYTE)val;
    *s << b;
    val = (int)b;
}

void SerializerObject::native_SerializeShort(CallStruct &cs)
{
    assert(s);
    if(!s) return;

    int& val = cs.GetIntOut(0);

    short sVal = (short)val;
    *s << sVal;
    val = (int)sVal;
}

void SerializerObject::native_SerializeWord(CallStruct &cs)
{
    assert(s);
    if(!s) return;

    int& val = cs.GetIntOut(0);

    WORD wVal = (WORD)val;
    *s << wVal;
    val = (int)wVal;
}

void SerializerObject::native_SerializeInt(CallStruct &cs)
{
    assert(s);
    if(!s) return;

    int& val = cs.GetIntOut(0);
    *s << val;
}

void SerializerObject::native_SerializeFloat(CallStruct &cs)
{
    assert(s);
    if(!s) return;

    float& val = cs.GetFloatOut(0);
    *s << val;
}

void SerializerObject::native_SerializeDouble(CallStruct &cs)
{
    assert(s);
    if(!s) return;

    float& val = cs.GetFloatOut(0);

    double dVal = (double)val;
    *s << dVal;
    val = (float)dVal;
}

void SerializerObject::native_SerializeString(CallStruct &cs)
{
    assert(s);
    if(!s) return;

    String& val = cs.GetStringOut(0);
    *s << val;
}

void SerializerObject::native_SerializeObject(CallStruct &cs)
{
    Object* obj = (Object*)cs.GetObject(0);
    obj->Serialize(*s);
}

void SerializerObject::native_IsLoading(CallStruct &cs)
{
    assert(s);
    if(!s) return;

    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    returnVal = s->IsLoading();
}

void SerializerObject::native_GetPos(CallStruct &cs)
{
    assert(s);
    if(!s) return;

    int& returnVal = cs.GetIntOut(RETURNVAL);
    returnVal = s->GetPos();
}

void SerializerObject::native_Seek(CallStruct &cs)
{
    assert(s);
    if(!s) return;

    int& returnVal = cs.GetIntOut(RETURNVAL);
    int offset = cs.GetInt(0);
    int seekType = cs.GetInt(1);

    returnVal = s->Seek(offset, seekType);
}

void ENGINEAPI NativeGlobal_CreateFileInputSerializer(CallStruct &cs)
{
    SerializerObject*& returnVal = (SerializerObject*&)cs.GetObjectOut(RETURNVAL);
    String fileName = cs.GetString(0);

    XFileInputSerializer *s = new XFileInputSerializer;
    if(!s->Open(fileName))
    {
        delete s;
        returnVal = NULL;
        return;
    }

    returnVal = CreateObjectParam2(SerializerObject, s, TRUE);
}

void ENGINEAPI NativeGlobal_CreateFileOutputSerializer(CallStruct &cs)
{
    SerializerObject*& returnVal = (SerializerObject*&)cs.GetObjectOut(RETURNVAL);
    String fileName = cs.GetString(0);
    int disposition = cs.GetInt(1);

    XFileOutputSerializer *s = new XFileOutputSerializer;
    if(!s->Open(fileName, (DWORD)disposition))
    {
        delete s;
        returnVal = NULL;
        return;
    }

    returnVal = CreateObjectParam2(SerializerObject, s, TRUE);
}
