/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ScriptBytecodeProcessor

  Copyright (c) 2009, Hugh Bailey
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
#include "ScriptDefs.h"
#include "ScriptCompiler.h"
#include "ScriptBytecode.h"

struct TempListThingy
{
    BYTE *array;
    unsigned int num;
};

BOOL FunctionDefinition::Call(Object *curObj, CallStruct &callData)
{
#if defined(USE_TRACE) //&& defined(FULL_TRACE)
    {try{
#endif

    if(!ByteCode.Num())
    {
        if(curObj && ObjectFunc)
        {
            (curObj->*ObjectFunc)(callData);
            return TRUE;
        }
        else if(NativeFunc)
        {
            NativeFunc(callData);
            return TRUE;
        }
        else
            return FALSE;
    }

    DataStack Stack;
    DataStack TempStack;
    List<void*> AddressStack;

    List<BYTE> LocalData;
    LocalData.SetSize(LocalVariableStackSize);

    BufferInputSerializer data(ByteCode);

    DWORD lastZeroPos = 0;

    ByteCodeInstruction instruction = BCNop;

    while(data.DataPending())
    {
        if(!Stack.Num() && !TempStack.Num() && !AddressStack.Num())
            lastZeroPos = data.GetPos();

        data << (BYTE&)instruction;

        switch(instruction)
        {
            case BCNop:
                break;

            case BCPush:         //in: [int size][value], push (value)
                {
                    int size;

                    data << size;
                    Stack.Push(size);
                    data.Serialize(Stack.Array(), size);
                    break;
                }
            case BCPop:          //in: [int size], pop value from the stack and into thin air
                {
                    int size;

                    data << size;
                    Stack.Pop(size);
                    break;
                }

            case BCPushDup:      //in: [int size], push (duplicated stack val)
                {
                    int size;

                    data << size;
                    Stack.Push(size); //this will actually duplicate the value automatically
                    mcpy(Stack.Array(), Stack.Array()+size, size);
                    break;
                }
            case BCPushDupInit:  //in: [string type][string subType], push (duplicated stack val)
                {
                    String typeName, subTypeName;
                    data << typeName << subTypeName;

                    TypeInfo typeInfo;
                    if(subTypeName.IsValid()) //list
                    {
                        typeInfo.name = TEXT("list");
                        typeInfo.type = DataType_List;
                        typeInfo.size = sizeof(List<BYTE>);

                        TypeInfo subTypeInfo;
                        Compiler::GetTypeInfo(subTypeName, subTypeInfo);

                        Stack.Push(typeInfo.size);
                        mcpy(Stack.Array(), Stack.Array()+typeInfo.size, typeInfo.size);

                        Scripting->DuplicateTypeData(Stack.Array(), typeInfo, &subTypeInfo);
                    }
                    else //struct
                    {
                        StructDefinition *structDef = Scripting->GetStruct(typeName);

                        Stack.Push(structDef->size);
                        mcpy(Stack.Array(), Stack.Array()+structDef->size, structDef->size);

                        Scripting->DuplicateStructData(Stack.Array(), structDef);
                    }

                    break;
                }

            case BCAPushThis:    //apush: [curObj*]
                {
                    AddressStack.Insert(0, &curObj);
                    break;
                }

                //--------------------------------------------------------------------

            case BCAPushDup:     //apush: [address]
                {
                    AddressStack.Insert(0, AddressStack[0]);
                    break;
                }

            case BCAPush:        //in: [int scope][int pos][int arrayOffset], apush: [address]
                {
                    int scope, pos, arrayOffset=0;
                    BOOL bArrayOffset;

                    data << scope << pos << bArrayOffset;

                    if(bArrayOffset)
                        Stack.PopData(&arrayOffset, sizeof(int));

                    switch(scope)
                    {
                        case VarScope_Param:
                            AddressStack.Insert(0, ((LPBYTE)callData.Params[pos+1].lpData)+arrayOffset); break;
                        case VarScope_Local:
                            AddressStack.Insert(0, LocalData+(pos+arrayOffset)); break;
                        //case VarScope_Object:
                        //    AddressStack.Insert(0, curObj->objScript->Varaibles[pos].DefaultParamData.Array()+arrayOffset); break;
                        case VarScope_Struct:
                            AddressStack.Insert(0, ((LPBYTE)curObj)+(pos+arrayOffset)); break;
                        case VarScope_Class:
                            AddressStack.Insert(0, ((LPBYTE)curObj->GetObjectClass()->GetScriptVarAddress(curObj, pos))+arrayOffset); break;
                        case VarScope_Global:
                            AddressStack.Insert(0, ((LPBYTE)Scripting->GetGlobalVariableAddress(pos))+arrayOffset); break;
                    }
                    break;
                }

            case BCAPop:         //apop: [address]
                {
                    AddressStack.Remove(0);
                    break;
                }

            case BCAPushSVar:    //in: [int pos][bool istempstack][int arrayOffset], apop: [struct*], apush: [address]
                {
                    int pos, arrayOffset=0;
                    BOOL bArrayOffset, bIsTempStack;
                    data << pos << bIsTempStack << bArrayOffset;

                    if(bArrayOffset)
                        Stack.PopData(&arrayOffset, sizeof(int));

                    if(bIsTempStack)
                        AddressStack[0] = TempStack.Array()+pos+arrayOffset;
                    else
                        AddressStack[0] = (((BYTE*)AddressStack[0])+pos)+arrayOffset;
                    break;
                }

            case BCAPushCVar:    //in: [int pos][bool istempstack][int arrayOffset], apop: [obj**], apush: [address]
                {
                    int pos, arrayOffset=0;
                    BOOL bArrayOffset, bIsTempStack;
                    data << pos << bIsTempStack << bArrayOffset;

                    if(bArrayOffset)
                        Stack.PopData(&arrayOffset, sizeof(int));

                    Object *obj;
                    if(bIsTempStack)
                    {
                        TempStack.GetData(&obj, sizeof(Object*));
                        AddressStack.Insert(0, ((LPBYTE)obj->GetObjectClass()->GetScriptVarAddress(obj, pos))+arrayOffset);
                    }
                    else
                    {
                        obj = *(Object**)AddressStack[0];
                        AddressStack[0] = ((LPBYTE)obj->GetObjectClass()->GetScriptVarAddress(obj, pos))+arrayOffset;
                    }
                    break;
                }

            case BCAPushListItem://in: [string subType], apop: [list*], pop: [element], apush: [address]
                {
                    String subType;
                    data << subType;

                    TypeInfo type;
                    Compiler::GetTypeInfo(subType, type);

                    unsigned int element;
                    Stack.PopData(&element, sizeof(unsigned int));

                    TempListThingy &list = *(TempListThingy*)AddressStack[0];
                    AddressStack[0] = list.array+(element*type.size);
                    break;
                }

            case BCPushAData:    //in: [int size][bool noremove], apop: [address], push: [data]
                {
                    int size;
                    BOOL bNoRemove;

                    data << size << bNoRemove;

                    Stack.PushData(AddressStack[0], size);
                    if(!bNoRemove)
                        AddressStack.Remove(0);
                    break;
                }
            case BCPushADataInit://in: [string type][string subType][bool noremove], apop: [address], push: [data]
                {
                    String typeName, subTypeName;
                    BOOL bNoRemove;

                    data << typeName << subTypeName << bNoRemove;

                    TypeInfo typeInfo;
                    if(subTypeName.IsValid()) //list
                    {
                        typeInfo.name = TEXT("list");
                        typeInfo.type = DataType_List;
                        typeInfo.size = sizeof(List<BYTE>);

                        TypeInfo subTypeInfo;
                        Compiler::GetTypeInfo(subTypeName, subTypeInfo);

                        Stack.PushData(AddressStack[0], typeInfo.size);
                        if(!bNoRemove) AddressStack.Remove(0);

                        Scripting->DuplicateTypeData(Stack.Array(), typeInfo, &subTypeInfo);
                    }
                    else //struct
                    {
                        StructDefinition *structDef = Scripting->GetStruct(typeName);

                        Stack.PushData(AddressStack[0], structDef->size);
                        if(!bNoRemove) AddressStack.Remove(0);

                        Scripting->DuplicateStructData(Stack.Array(), structDef);
                    }

                    break;
                }

            case BCAPopVarInit:  //in: [string typeName][string subTypeName], apop: [address], pop: [data], out (data)
                {
                    String typeName, subTypeName;

                    data << typeName << subTypeName;

                    TypeInfo typeInfo;
                    if(subTypeName.IsValid()) //list
                    {
                        typeInfo.name = TEXT("list");
                        typeInfo.type = DataType_List;
                        typeInfo.size = sizeof(List<BYTE>);

                        TypeInfo subTypeInfo;
                        Compiler::GetTypeInfo(subTypeName, subTypeInfo);

                        Scripting->FreeTypeData(AddressStack[0], typeInfo, &subTypeInfo);
                        Stack.PopData(AddressStack[0], typeInfo.size);
                        //Scripting->DuplicateTypeData(AddressStack[0], typeInfo, &subTypeInfo); //changed it so the variable is duplicated at push
                    }
                    else //struct
                    {
                        StructDefinition *structDef = Scripting->GetStruct(typeName);

                        Scripting->FreeStructData(AddressStack[0], structDef);
                        Stack.PopData(AddressStack[0], structDef->size);
                        //Scripting->DuplicateStructData(AddressStack[0], structDef);
                    }

                    AddressStack.Remove(0);
                    break;
                }
            case BCAPopVar:      //in: [int size], apop: [address], pop: [data], out (data)
                {
                    int size;
                    data << size;

                    Stack.PopData(AddressStack[0], size);
                    AddressStack.Remove(0);
                    break;
                }

            case BCAPushStack:   //apush: [address to data stack]
                {
                    AddressStack.Insert(0, (void*)Stack.Array());
                    break;
                }

            case BCTPushStack:   //in: [int size], tpush [data], pop: [data]
                {
                    int size;
                    data << size;

                    TempStack.PushData(Stack.Array(), size);
                    Stack.Pop(size);
                    break;
                }
            case BCTPushZero:    //in: [int size], tpush (zeros)
                {
                    int size;
                    data << size;

                    TempStack.Push(size);
                    zero(TempStack.Array(), size);
                    break;
                }
            case BCTPop:         //in: [int size], tpop: [data]
                {
                    int size;
                    data << size;

                    TempStack.Pop(size);
                    break;
                }
            case BCTPopFree:     //in: [string type][string subType], tpop: [data]
                {
                    String typeName, subTypeName;
                    data << typeName << subTypeName;

                    if(subTypeName.IsValid()) //list
                    {
                        TypeInfo typeInfo;
                        typeInfo.name = TEXT("list");
                        typeInfo.type = DataType_List;
                        typeInfo.size = sizeof(List<BYTE>);

                        TypeInfo subTypeInfo;
                        Compiler::GetTypeInfo(subTypeName, subTypeInfo);

                        Scripting->FreeTypeData(TempStack.Array(), typeInfo, &subTypeInfo);

                        TempStack.Pop(typeInfo.size);
                    }
                    else //struct
                    {
                        StructDefinition *structDef = Scripting->GetStruct(typeName);
                        Scripting->FreeStructData(TempStack.Array(), structDef);

                        TempStack.Pop(structDef->size);
                    }
                    break;
                }
            case BCPushTData:    //in: [int size], tpop: [data], push [data]
                {
                    int size;

                    data << size;

                    Stack.PushData(TempStack.Array(), size);
                    TempStack.Pop(size);
                    break;
                }
            case BCAPushT:       //apush: [address to temp data stack]
                {
                    AddressStack.Insert(0, (void*)TempStack.Array());
                    break;
                }

                //--------------------------------------------------------------------

            case BCDynamicCast:  //in: [string classname], alter stack: [curObj* or null if uncastable]
                {
                    String strClass;
                    data << strClass;

                    Object *&obj = *(Object**)Stack.Array();
                    if(!obj->IsOf(strClass))
                        obj = NULL;
                    break;
                }
            case BCCastFloat:    //alter stack: [int val], (float)val;
                {
                    *(float*)Stack.Array() = (float)*(int*)Stack.Array();
                    break;
                }
            case BCCastInt:      //alter stack: [float val], (int)val;
                {
                    *(int*)Stack.Array() = (int)*(float*)Stack.Array();
                    break;
                }

                //--------------------------------------------------------------------

            case BCNegate:       //alter stack: [int val], -val
                {
                    *(int*)Stack.Array() = -(*(int*)Stack.Array());
                    break;
                }

            case BCNegateF:      //alter stack: [float val], -val
                {
                    *(float*)Stack.Array() = -(*(float*)Stack.Array());
                    break;
                }

                //--------------------------------------------------------------------

            case BCCallFunc:     //in: [int funcID], pop: [paramN-1, paramN-2, ...], tpush (return value)
                {
                    int funcID;
                    CallStruct newCS;

                    data << funcID;
                    FunctionDefinition *newFunc = Scripting->GetFunctionByID(funcID);
                    assert(newFunc);
                    
                    newCS.SetNumParams(newFunc->Params.Num());

                    for(int i=newFunc->Params.Num()-1; i>=0; i--)
                    {
                        Variable *var = &newFunc->Params[i];
                        if(var->typeInfo.type == DataType_String)
                        {
                            String str;
                            Stack.PopString(str);
                            newCS.SetString(i, str);
                        }
                        else
                        {
                            int varSize = var->typeInfo.size;
                            newCS.Params[i+1].lpData = Allocate(varSize);
                            Stack.PopData(newCS.Params[i+1].lpData, varSize);
                        }

                        newCS.Params[i+1].type = var->typeInfo.type;
                    }

                    if(newFunc->returnType.size != 0)
                    {
                        newCS.Params[0].lpData = Allocate(newFunc->returnType.size);
                        newCS.Params[0].type = newFunc->returnType.type;
                        zero(newCS.Params[0].lpData, newFunc->returnType.size);
                    }

                    newFunc->Call(NULL, newCS);

                    for(int i=0; i<newFunc->Params.Num(); i++)
                    {
                        DefaultVariable *var = &newFunc->Params[i];
                        if(var->flags & VAR_OUT)
                        {
                            if(var->typeInfo.type == DataType_String)
                            {
                                String str = newCS.GetString(i);
                                Stack.PushString(str);
                            }
                            else
                                Stack.PushData(newCS.Params[i+1].lpData, var->typeInfo.size);
                        }
                        else if(var->typeInfo.type == DataType_Struct)
                            Scripting->FreeStructData(newCS.Params[i+1].lpData, var->structDef);
                    }

                    if(newCS.IsValid(RETURNVAL))
                    {
                        if(newFunc->returnType.type == DataType_String)
                            TempStack.PushString(newCS.GetString(RETURNVAL));
                        else
                            TempStack.PushData(newCS.Params[0].lpData, newFunc->returnType.size);
                    }

                    break;
                }

            case BCCallStructOp: //in: [string struct][int pos], pop: [paramN-1, paramN-2, ...], apop: [struct*], tpush (return value)
            case BCCallStructFunc://in: [string struct][int pos], pop: [paramN-1, paramN-2, ...], apop: [struct*], tpush (return value)
                {
                    int pos;
                    CallStruct newCS;
                    String structName;

                    data << structName << pos;

                    StructDefinition *structDef = Scripting->GetStruct(structName);
                    FunctionDefinition *newFunc = (FunctionDefinition*)&structDef->Functions[pos];
                    assert(newFunc);

                    LPVOID structPtr = NULL;
                    if(instruction != BCCallStructOp && ~newFunc->flags & FUNC_STATIC)
                    {
                        structPtr = AddressStack[0];
                        AddressStack.Remove(0);
                    }

                    newCS.SetNumParams(newFunc->Params.Num());

                    for(int i=newFunc->Params.Num()-1; i>=0; i--)
                    {
                        DefaultVariable *var = &newFunc->Params[i];
                        if(var->typeInfo.type == DataType_String)
                        {
                            String str;
                            Stack.PopString(str);
                            newCS.SetString(i, str);
                        }
                        else
                        {
                            int varSize = var->typeInfo.size;
                            newCS.Params[i+1].lpData = Allocate(varSize);
                            Stack.PopData(newCS.Params[i+1].lpData, varSize);
                        }

                        newCS.Params[i+1].type = var->typeInfo.type;
                    }

                    //kind of a hack to be perfectly honest, an operator pushes both the source and target,
                    // but a structure function does not use the latter, so we just pop it right here
                    //if(instruction == BCCallStructOp)
                    //    Stack.Pop(structDef->size);

                    if(newFunc->returnType.size != 0)
                    {
                        newCS.Params[0].lpData = Allocate(newFunc->returnType.size);
                        newCS.Params[0].type = newFunc->returnType.type;
                        zero(newCS.Params[0].lpData, newFunc->returnType.size);
                    }

                    if(newFunc->flags & FUNC_STATIC)
                    {
                        newFunc->Call(NULL, newCS);
                    }
                    else
                    {
                        LPVOID thisVal = Allocate(structDef->size); //have to allocate in order to keep it aligned
                        if(instruction == BCCallStructOp)
                        {
                            Stack.PopData(thisVal, structDef->size);
                            newFunc->Call((Object*)thisVal, newCS);
                        }
                        else
                        {
                            mcpy(thisVal, structPtr, structDef->size);
                            newFunc->Call((Object*)thisVal, newCS);
                            mcpy(structPtr, thisVal, structDef->size);
                        }
                        Free(thisVal);
                    }

                    for(int i=0; i<newFunc->Params.Num(); i++)
                    {
                        DefaultVariable *var = &newFunc->Params[i];
                        if(var->flags & VAR_OUT)
                        {
                            if(var->typeInfo.type == DataType_String)
                            {
                                String str = newCS.GetString(i);
                                Stack.PushString(str);
                            }
                            else
                                Stack.PushData(newCS.Params[i+1].lpData, var->typeInfo.size);
                        }
                        else if(var->typeInfo.type == DataType_Struct)
                            Scripting->FreeStructData(newCS.Params[i+1].lpData, var->structDef);
                    }

                    if(newCS.IsValid(RETURNVAL))
                    {
                        if(newFunc->returnType.type == DataType_String)
                            TempStack.PushString(newCS.GetString(RETURNVAL));
                        else
                            TempStack.PushData(newCS.Params[0].lpData, newFunc->returnType.size);
                    }

                    break;
                }

            case BCCallTypeFunc:
                {
                    String subType;
                    int pos, type;

                    data << type << pos << subType;

                    LPVOID varAddr = AddressStack[0];
                    AddressStack.Remove(0);

                    TypeFunction *func = NULL;
                    for(int i=0; i<Scripting->ScriptTypes.Num(); i++)
                    {
                        ScriptTypeData &typeData = Scripting->ScriptTypes[i];

                        if(typeData.type.type == (VariableType)type)
                        {
                            func = &typeData.funcs[pos];
                            break;
                        }
                    }

                    assert(func);
                    if(!func) break;

                    func->typeFunc(varAddr, subType, Stack, TempStack);

                    break;
                }

            case BCCallClassStFn://in: [string class][int funcID], pop: [paramN-1, paramN-2, ...], tpush (return value)
            case BCCallClassFunc://in: [int pos][bool bSuper], pop: [paramN-1, paramN-2, ...], apop: [obj**], tpush (return value)
                {
                    FunctionDefinition *newFunc;
                    Object *obj = NULL;
                    CallStruct newCS;

                    if(IsLocation(curObj, TEXT("OrbitGame"), TEXT("Init")))
                        nop();

                    if(instruction == BCCallClassFunc)
                    {
                        int pos;
                        BOOL bSuper;

                        data << pos << bSuper;

                        obj = *(Object**)AddressStack[0];
                        AddressStack.Remove(0);

                        Class *callClass = (bSuper) ? classContext->Parent->classData : obj->GetObjectClass();
                        newFunc = callClass->GetTopmostFunction(pos);
                    }
                    else
                    {
                        int funcID;
                        String strClass;

                        data << strClass << funcID;

                        ClassDefinition *classDef = Scripting->GetClassDef(strClass);
                        newFunc = &classDef->Functions[funcID];
                    }

                    assert(newFunc);

                    newCS.SetNumParams(newFunc->Params.Num());

                    for(int i=newFunc->Params.Num()-1; i>=0; i--)
                    {
                        DefaultVariable *var = &newFunc->Params[i];
                        if(var->typeInfo.type == DataType_String)
                        {
                            String str;
                            Stack.PopString(str);
                            newCS.SetString(i, str);
                        }
                        else
                        {
                            int varSize = var->typeInfo.size;
                            newCS.Params[i+1].lpData = Allocate(varSize);
                            Stack.PopData(newCS.Params[i+1].lpData, varSize);
                        }

                        newCS.Params[i+1].type = var->typeInfo.type;
                    }

                    if(newFunc->returnType.size != 0)
                    {
                        newCS.Params[0].lpData = Allocate(newFunc->returnType.size);
                        newCS.Params[0].type = newFunc->returnType.type;
                        zero(newCS.Params[0].lpData, newFunc->returnType.size);
                    }

                    newFunc->Call(obj, newCS);

                    for(int i=0; i<newFunc->Params.Num(); i++)
                    {
                        DefaultVariable *var = &newFunc->Params[i];
                        if(var->flags & VAR_OUT)
                        {
                            if(var->typeInfo.type == DataType_String)
                            {
                                String str = newCS.GetString(i);
                                Stack.PushString(str);
                            }
                            else
                                Stack.PushData(newCS.Params[i+1].lpData, var->typeInfo.size);
                        }
                        else if(var->typeInfo.type == DataType_Struct)
                            Scripting->FreeStructData(newCS.Params[i+1].lpData, var->structDef);
                    }

                    if(newCS.IsValid(RETURNVAL))
                    {
                        if(newFunc->returnType.type == DataType_String)
                            TempStack.PushString(newCS.GetString(RETURNVAL));
                        else
                            TempStack.PushData(newCS.Params[0].lpData, newFunc->returnType.size);
                    }

                    break;
                }

                //--------------------------------------------------------------------

            case BCCreateObj:    //in: [string className], tpush: [obj]
                {
                    String className;
                    data << className;

                    Object *obj = CreateFactoryObject(className, FALSE);

                    TempStack.PushData(&obj, sizeof(Object*));
                    break;
                }

            case BCDestroyObj:   //pop: [Object *obj]
                {
                    Object *obj;
                    Stack.PopData(&obj, sizeof(Object*));
                    DestroyObject(obj);
                    break;
                }

                //--------------------------------------------------------------------

            case BCReturn:       //in: [int size], pop: [value]
                {
                    int size;

                    data << size;
                    if(size)
                    {
                        if(!callData.IsValid(RETURNVAL))
                            callData.Params[0].lpData = Allocate(size);

                        Stack.PopData(callData.Params[0].lpData, size);
                    }
                    data.Seek(0, SERIALIZE_SEEK_END);
                    break;
                }

                //--------------------------------------------------------------------

            case BCJump:         //in: [int pos]
                {
                    int pos;
                    data << pos;
                    data.Seek(pos, SERIALIZE_SEEK_CURRENT);
                    break;
                }
            case BCJumpIf:       //in: [int pos], pop: [bool value]
                {
                    int pos;
                    data << pos;

                    int bJump;
                    Stack.PopData(&bJump, sizeof(int));
                    if(bJump)
                        data.Seek(pos, SERIALIZE_SEEK_CURRENT);
                    break;
                }
            case BCJumpIfNot:    //in: [int pos], pop: [bool value]
                {
                    int pos;
                    data << pos;

                    int bJump;
                    Stack.PopData(&bJump, sizeof(int));
                    if(!bJump)
                        data.Seek(pos, SERIALIZE_SEEK_CURRENT);
                    break;
                }

                //--------------------------------------------------------------------

            case BCMultiply:     //pop: [int param2, int param1], push (param1*param2)
                {
                    int params[2];
                    Stack.PopData(params, sizeof(int)*2);
                    params[1] *= params[0];
                    Stack.PushData(&params[1], sizeof(int));
                    break;
                }
            case BCDivide:       //pop: [int param2, int param1], push (param1/param2)
                {
                    int params[2];
                    Stack.PopData(params, sizeof(int)*2);
                    params[1] /= params[0];
                    Stack.PushData(&params[1], sizeof(int));
                    break;
                }
            case BCAdd:          //pop: [int param2, int param1], push (param1+param2)
                {
                    int params[2];
                    Stack.PopData(params, sizeof(int)*2);
                    params[1] += params[0];
                    Stack.PushData(&params[1], sizeof(int));
                    break;
                }
            case BCSubtract:     //pop: [int param2, int param1], push (param1-param2)
                {
                    int params[2];
                    Stack.PopData(params, sizeof(int)*2);
                    params[1] -= params[0];
                    Stack.PushData(&params[1], sizeof(int));
                    break;
                }
            case BCMod:          //pop: [int param2, int param1], push (param1%param2)
                {
                    int params[2];
                    Stack.PopData(params, sizeof(int)*2);
                    params[1] %= params[0];
                    Stack.PushData(&params[1], sizeof(int));
                    break;
                }

                //--------------------------------------------------------------------

            case BCEqual:        //pop: [int param2, int param1], push (param1 == param2)
                {
                    int params[2];
                    Stack.PopData(params, sizeof(int)*2);
                    params[1] = (params[1] == params[0]);
                    Stack.PushData(&params[1], sizeof(int));
                    break;
                }
            case BCNotEqual:     //pop: [int param2, int param1], push (param1 != param2)
                {
                    int params[2];
                    Stack.PopData(params, sizeof(int)*2);
                    params[1] = params[1] != params[0];
                    Stack.PushData(&params[1], sizeof(int));
                    break;
                }
            case BCGreaterEqual: //pop: [int param2, int param1], push (param1 >= param2)
                {
                    int params[2];
                    Stack.PopData(params, sizeof(int)*2);
                    params[1] = params[1] >= params[0];
                    Stack.PushData(&params[1], sizeof(int));
                    break;
                }
            case BCLessEqual:    //pop: [int param2, int param1], push (param1 <= param2)
                {
                    int params[2];
                    Stack.PopData(params, sizeof(int)*2);
                    params[1] = params[1] <= params[0];
                    Stack.PushData(&params[1], sizeof(int));
                    break;
                }
            case BCGreater:      //pop: [int param2, int param1], push (param1 > param2)
                {
                    int params[2];
                    Stack.PopData(params, sizeof(int)*2);
                    params[1] = params[1] > params[0];
                    Stack.PushData(&params[1], sizeof(int));
                    break;
                }
            case BCLess:         //pop: [int param2, int param1], push (param1 < param2)
                {
                    int params[2];
                    Stack.PopData(params, sizeof(int)*2);
                    params[1] = params[1] < params[0];
                    Stack.PushData(&params[1], sizeof(int));
                    break;
                }

                //--------------------------------------------------------------------

            case BCMultiplyF:    //pop: [float param2, float param1], push (param1*param2)
                {
                    float params[2];
                    Stack.PopData(params, sizeof(float)*2);
                    params[1] *= params[0];
                    Stack.PushData(&params[1], sizeof(float));
                    break;
                }
            case BCDivideF:      //pop: [float param2, float param1], push (param1/param2)
                {
                    float params[2];
                    Stack.PopData(params, sizeof(float)*2);
                    params[1] /= params[0];
                    Stack.PushData(&params[1], sizeof(float));
                    break;
                }
            case BCAddF:         //pop: [float param2, float param1], push (param1+param2)
                {
                    float params[2];
                    Stack.PopData(params, sizeof(float)*2);
                    params[1] += params[0];
                    Stack.PushData(&params[1], sizeof(float));
                    break;
                }
            case BCSubtractF:    //pop: [float param2, float param1], push (param1-param2)
                {
                    float params[2];
                    Stack.PopData(params, sizeof(float)*2);
                    params[1] -= params[0];
                    Stack.PushData(&params[1], sizeof(float));
                    break;
                }
            case BCModF:         //pop: [float param2, float param1], push (param1%param2)
                {
                    float params[2];
                    Stack.PopData(params, sizeof(float)*2);
                    params[1] = modff(params[1], params);
                    Stack.PushData(&params[1], sizeof(float));
                    break;
                }

                //--------------------------------------------------------------------

            case BCEqualF:       //pop: [float param2, float param1], push (param1 == param2)
                {
                    float params[2];
                    Stack.PopData(params, sizeof(float)*2);
                    int bReturnVal = params[1] == params[0];
                    Stack.PushData(&bReturnVal, sizeof(int));
                    break;
                }
            case BCNotEqualF:    //pop: [float param2, float param1], push (param1 != param2)
                {
                    float params[2];
                    Stack.PopData(params, sizeof(float)*2);
                    int bReturnVal = params[1] != params[0];
                    Stack.PushData(&bReturnVal, sizeof(int));
                    break;
                }
            case BCGreaterEqualF://pop: [float param2, float param1], push (param1 >= param2)
                {
                    float params[2];
                    Stack.PopData(params, sizeof(float)*2);
                    int bReturnVal = params[1] >= params[0];
                    Stack.PushData(&bReturnVal, sizeof(int));
                    break;
                }
            case BCLessEqualF:   //pop: [float param2, float param1], push (param1 <= param2)
                {
                    float params[2];
                    Stack.PopData(params, sizeof(float)*2);
                    int bReturnVal = params[1] <= params[0];
                    Stack.PushData(&bReturnVal, sizeof(int));
                    break;
                }
            case BCGreaterF:     //pop: [float param2, float param1], push (param1 > param2)
                {
                    float params[2];
                    Stack.PopData(params, sizeof(float)*2);
                    int bReturnVal = params[1] > params[0];
                    Stack.PushData(&bReturnVal, sizeof(int));
                    break;
                }
            case BCLessF:        //pop: [float param2, float param1], push (param1 < param2)
                {
                    float params[2];
                    Stack.PopData(params, sizeof(float)*2);
                    int bReturnVal = params[1] < params[0];
                    Stack.PushData(&bReturnVal, sizeof(int));
                    break;
                }

                //--------------------------------------------------------------------

            case BCPushNullObj:  //push (null obj)
                {
                    Object *nullObj = NULL;
                    Stack.PushData(&nullObj, sizeof(Object*));
                    break;
                }
            case BCPushNullStr:  //push (null str)
                {
                    DWORD zerolenStr = 0;
                    Stack.PushData(&zerolenStr, sizeof(DWORD));
                    break;
                }

                //--------------------------------------------------------------------

            case BCEqualO:       //pop: [Object* param2, Object* param1], push (param1 == param2)
                {
                    Object* params[2];
                    Stack.PopData(params, sizeof(Object*)*2);
                    int bReturnVal = params[1] == params[0];
                    Stack.PushData(&bReturnVal, sizeof(int));
                    break;
                }
            case BCNotEqualO:    //pop: [Object* param2, Object* param1], push (param1 != param2)
                {
                    Object* params[2];
                    Stack.PopData(params, sizeof(Object*)*2);
                    int bReturnVal = params[1] != params[0];
                    Stack.PushData(&bReturnVal, sizeof(int));
                    break;
                }

                //--------------------------------------------------------------------

            case BCBitwiseNot:   //alter stack: [int val], ~val
                {
                    *(unsigned int*)Stack.Array() = ~(*(unsigned int*)Stack.Array());
                    break;
                }
            case BCBitwiseOr:    //pop: [int param2, int param1], push (param1 | param2)
                {
                    unsigned int params[2];
                    Stack.PopData(params, sizeof(unsigned int)*2);
                    params[0] = params[1] | params[0];
                    Stack.PushData(params, sizeof(unsigned int));
                    break;
                }
            case BCBitwiseAnd:   //pop: [int param2, int param1], push (param1 & param2)
                {
                    unsigned int params[2];
                    Stack.PopData(params, sizeof(unsigned int)*2);
                    params[0] = params[1] & params[0];
                    Stack.PushData(params, sizeof(unsigned int));
                    break;
                }
            case BCShiftLeft:    //pop: [int param2, int param1], push (param1 << param2)
                {
                    unsigned int params[2];
                    Stack.PopData(params, sizeof(unsigned int)*2);
                    params[0] = params[1] << params[0];
                    Stack.PushData(params, sizeof(unsigned int));
                    break;
                }
            case BCShiftRight:   //pop: [int param2, int param1], push (param1 >> param2)
                {
                    unsigned int params[2];
                    Stack.PopData(params, sizeof(unsigned int)*2);
                    params[0] = params[1] >> params[0];
                    Stack.PushData(params, sizeof(unsigned int));
                    break;
                }

                //--------------------------------------------------------------------

            case BCLogicalNot:   //alter stack: [int val], !val
                {
                    *(unsigned int*)Stack.Array() = !(*(unsigned int*)Stack.Array());
                    break;
                }

                //--------------------------------------------------------------------

            case BCReturnS:      //pop: [String str]
                {
                    String str;
                    Stack.PopString(str);

                    callData.SetString(RETURNVAL, str);
                    data.Seek(0, SERIALIZE_SEEK_END);
                    break;
                }

            case BCPushS:        //in: [String str], push (value)
                {
                    String str;

                    data << str;

                    Stack.PushString(str);
                    break;
                }
            case BCPopS:         //pop: [String str]
                {
                    String str;
                    Stack.PopString(str);
                    break;
                }
            case BCPushDupS:     //push (dup string)
                {
                    String strVal;
                    UINT len = *(DWORD*)Stack.Array();
                    strVal.SetLength(len);
                    scpy_n(strVal.Array(), (TCHAR*)(Stack.Array()+4), len);
                    Stack.PushString(strVal);
                    break;
                }

            case BCPushADataS:   //in: [bool noremove], apop: [address], push: [data]
                {
                    BOOL bNoRemove;

                    data << bNoRemove;

                    Stack.PushString(*(String*)AddressStack[0]);
                    if(!bNoRemove)
                        AddressStack.Remove(0);
                    break;
                }

            case BCAPopVarS:     //apop: [address], pop: [data], out (data)
                {
                    String str;
                    Stack.PopString(str);

                    *(String*)AddressStack[0] = str;
                    AddressStack.Remove(0);
                    break;
                }

            case BCPushTDataS:   //tpop: [data], push [data]
                {
                    String str;
                    TempStack.PopString(str);
                    Stack.PushString(str);
                    break;
                }

            case BCTPopS:        //tpop: [string]
                {
                    String str;
                    TempStack.PopString(str);
                    break;
                }


            case BCAddS:         //pop: [String string2, String string1], push (string1+string2)
                {
                    String string1, string2;
                    Stack.PopString(string2);
                    Stack.PopString(string1);
                    string1 += string2;
                    Stack.PushString(string1);
                    break;
                }
            case BCEqualS:       //pop: [String string2, String string1], push (string1.Compare(string2))
                {
                    String string1, string2;
                    Stack.PopString(string2);
                    Stack.PopString(string1);
                    int bCompare = string1.Compare(string2);
                    Stack.PushData(&bCompare, sizeof(int));
                    break;
                }
            case BCNotEqualS:    //pop: [String string2, String string1], push (string1.CompareI(string2))
                {
                    String string1, string2;
                    Stack.PopString(string2);
                    Stack.PopString(string1);
                    int bCompare = !string1.Compare(string2);
                    Stack.PushData(&bCompare, sizeof(int));
                    break;
                }

            default:
                nop();
        }
    }

    if(IsLocation(curObj, TEXT("OrbitGame"), TEXT("Init")))
        nop();

    //really have to do retarded stuff for local arrays.
    for(int i=0; i<LocalVars.Num(); i++)
        Scripting->FreeVarData(LocalData.Array()+LocalVars[i].offset, &LocalVars[i]);

    if(Stack.Num() || TempStack.Num() || AddressStack.Num())
        nop();

    return TRUE;

#if defined(USE_TRACE) //&& defined(FULL_TRACE)
    }catch(...)
    {
        String funcName;
        funcName << TEXT("[s]");
        if(this->classContext)
            funcName << this->classContext->classData->GetName() << TEXT("::");
        funcName << this->name;
        TraceCrash(funcName);
        funcName.Clear();
        throw;
    }}
#endif
}

