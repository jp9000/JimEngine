/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ScriptTypeFunctions

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


void ScriptSystem::AddTypeFunction(CTSTR lpType, CTSTR lpFuncDec, DEFFUNCPROC func)
{
    TypeInfo typeInfo;
    Compiler::GetTypeInfo(lpType, typeInfo);

    CodeTokenizer ct;
    ct.SetCodeStart(lpFuncDec);

    //--------------------------------------

    int i;
    ScriptTypeData *data;
    for(i=0; i<ScriptTypes.Num(); i++)
    {
        if(ScriptTypes[i].type == typeInfo)
        {
            data = ScriptTypes+i;
            break;
        }
    }

    if(i == ScriptTypes.Num())
    {
        data = ScriptTypes.CreateNew();
        data->type = typeInfo;
    }

    //--------------------------------------

    int funcID = data->funcs.Num();

    TypeFunction *funcDef;
    funcDef = data->funcs.CreateNew();

    //return value
    String curToken;
    ct.GetNextToken(curToken);
    Compiler::GetTypeInfo(curToken, funcDef->returnType);

    //name
    ct.GetNextToken(curToken);
    funcDef->name = curToken;
    funcDef->typeFunc = (TYPEFUNCPROC)func;
    funcDef->funcOffset = funcID;

    //(
    ct.GetNextToken(curToken);

    //vars - )
    int id = 0;

    ct.GetNextToken(curToken);
    while(curToken[0] != ')')
    {
        BOOL bOutVar = FALSE;

        if(curToken == TEXT("out"))
        {
            bOutVar = TRUE;
            ct.GetNextToken(curToken);
        }

        DefaultVariable *var;
        var = funcDef->Params.CreateNew();

        if(curToken == TEXT("subtype"))
        {
            var->typeInfo.type  = DataType_SubType;
            var->typeInfo.size  = 0;
            var->typeInfo.align = 0;
            var->typeInfo.name  = TEXT("subtype");
        }
        else
        {
            Compiler::GetTypeInfo(curToken, var->typeInfo);

            if(curToken == TEXT("list"))
            {
                ct.GetNextToken(curToken, TRUE);
                if(curToken == TEXT("<"))
                {
                    ct.GetNextToken(curToken);
                    ct.GetNextToken(curToken);
                    if(curToken != TEXT(">"))
                    {
                        Compiler::GetTypeInfo(curToken, var->subTypeInfo);
                        if(curToken == TEXT("subtype"))
                        {
                            var->subTypeInfo.type  = DataType_SubType;
                            var->subTypeInfo.size  = 0;
                            var->subTypeInfo.align = 0;
                            var->subTypeInfo.name  = TEXT("subtype");
                        }
                        ct.GetNextToken(curToken);
                    }
                    else
                        Compiler::GetTypeInfo(TEXT("int"), var->subTypeInfo);
                }
                else
                    Compiler::GetTypeInfo(TEXT("int"), var->subTypeInfo);
            }
        }

        var->offset = id;
        var->scope = VarScope_Param;
        var->numElements = 0;

        if(bOutVar)
            var->flags |= VAR_OUT;

        String strVarName;
        ct.GetNextToken(curToken);

        var->name = strVarName;

        ct.GetNextToken(curToken);

        if(curToken == TEXT("="))
        {
            ct.GetNextToken(curToken);

            if(curToken == TEXT("-"))
            {
                String nextToken;
                ct.GetNextToken(nextToken);
                curToken << nextToken;
            }

            var->flags |= VAR_DEFAULT;

            BufferOutputSerializer defOut(var->DefaultParamData);

            TokenType type = Compiler::GetTokenType(NULL, NULL, curToken);
            if(type == TokenType_Number)
            {
                if(Compiler::GetBaseFloat(curToken))
                {
                    float fChi = (float)tstof(curToken);

                    if(var->typeInfo.type == DataType_Float)
                        defOut << fChi;
                    else if(var->typeInfo.type == DataType_Integer)
                    {
                        int iChi = (int)fChi;
                        defOut << iChi;
                    }
                }
                else
                {
                    int iChi = tstoi(curToken);

                    if(var->typeInfo.type == DataType_Float)
                    {
                        float fChi = (float)iChi;
                        defOut << fChi;
                    }
                    else if(var->typeInfo.type == DataType_Integer)
                        defOut << iChi;
                }
            }
            else if(type == TokenType_String)
            {
                if(var->typeInfo.type == DataType_String)
                    defOut << Compiler::GetActualString(curToken);
            }
            else if(type == TokenType_Enum)
            {
                int val = Compiler::GetEnumVal(curToken);

                if(var->typeInfo.type == DataType_Integer)
                    defOut << val;
            }
            else if(curToken == TEXT("true"))
            {
                int val = 1;
                defOut << val;
            }
            else if(curToken == TEXT("false"))
            {
                int val = 0;
                defOut << val;
            }

            ct.GetNextToken(curToken);
        }

        if(curToken[0] != ')')
            ct.GetNextToken(curToken);

        ++id;
    }
}


//--------------------------------------------------------------------------------------
// Script List stuff
// holy hell what a nightmare, but it pays off, if you're looking at this, or any of the scripting stuff,
// please forgive me for having to endure such painful code.

struct ScriptList : List<BYTE>
{
    void listClear(TypeInfo &type)
    {
        if(type.type == DataType_Struct)
        {
            for(int i=0; i<num; i++)
                Scripting->FreeTypeData(array+(i*type.size), type);
        }

        Clear();
    }

    void listRemove(unsigned int index, TypeInfo &type)
    {
        if(index >= num)
        {
            AppWarning(TEXT("ScriptList::Remove tried to remove an invalid index"));
            return;
        }

        if(num == 1)
        {
            listClear(type);
            return;
        }

        --num;
        int moveCount = num-index;
        LPBYTE itemPtr = array+(index*type.size);
        if(moveCount)
            mcpy(itemPtr, itemPtr+type.size, moveCount*type.size);

        if(type.type == DataType_Struct)
            Scripting->FreeTypeData(itemPtr, type);

        array = (LPBYTE)ReAllocate(array, num*type.size);
    }

    //---------------------------

    static void ENGINEAPI scriptAdd(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        unsigned int newID;

        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        if(type.type == DataType_String)
        {
            String str;
            Stack.PopString(str);

            StringList &list = *(StringList*)var;
            newID = list.Add(str);
        }
        else
        {
            ScriptList &list = *(ScriptList*)var;

            newID = list.num;

            list.array = (BYTE*)ReAllocate(list.array, type.size*++list.num);
            LPVOID popAddr = list.array+(type.size*newID);

            Stack.PopData(popAddr, type.size);

            nop();
        }

        TempStack.PushData(&newID, sizeof(int));
    }

    static void ENGINEAPI scriptSafeAddBase(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack, BOOL bCaseInsensative)
    {
        unsigned int newID;

        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        if(type.type == DataType_String)
        {
            String str;
            Stack.PopString(str);

            StringList &list = *(StringList*)var;

            if(bCaseInsensative)
                newID = list.SafeAddI(str);
            else
                newID = list.SafeAdd(str);
        }
        else
        {
            ScriptList &list = *(ScriptList*)var;

            LPBYTE value = (LPBYTE)Allocate(type.size);
            Stack.PopData(value, type.size);

            for(newID=0; newID<list.num; newID++)
            {
                LPVOID lpItem = list.array+(list.num*type.size);
                if(mcmp(lpItem, value, type.size))
                    break;
            }

            //value wasn't found
            if(newID == list.num)
            {
                list.array = (LPBYTE)ReAllocate(list.array, type.size*++list.num);
                LPVOID popAddr = list.array+(type.size*newID);
            }

            Free(value);
        }

        TempStack.PushData(&newID, sizeof(int));
    }

    static void ENGINEAPI scriptSafeAdd(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        scriptSafeAddBase(var, subTypeName, Stack, TempStack, FALSE);
    }

    static void ENGINEAPI scriptSafeAddI(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        scriptSafeAddBase(var, subTypeName, Stack, TempStack, TRUE);
    }

    static void ENGINEAPI scriptInsert(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        if(type.type == DataType_String)
        {
            String str;
            Stack.PopString(str);

            int index;
            Stack.PopData(&index, sizeof(int));

            StringList &list = *(StringList*)var;

            if(index > list.Num())
            {
                AppWarning(TEXT("ScriptList::Insert - tried to insert item to an invalid index"));
                return;
            }

            list.Insert(index, str);
        }
        else
        {
            LPVOID lpTemp = Allocate(type.size);
            Stack.PopData(lpTemp, type.size); //popAddr

            int index;
            Stack.PopData(&index, sizeof(int));

            ScriptList &list = *(ScriptList*)var;

            assert(index <= list.num);
            if(index > list.num)
            {
                Stack.Pop(type.size);
                return;
            }

            int moveCount = list.num-index;

            list.array = (BYTE*)ReAllocate(list.array, type.size*++list.num);
            LPBYTE popAddr = list.array+(type.size*index);
            if(moveCount)
                mcpyrev(popAddr+type.size, popAddr, moveCount*type.size);

            mcpy(popAddr, lpTemp, type.size);

            Free(lpTemp);
        }
    }

    static void ENGINEAPI scriptRemove(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        int index;
        Stack.PopData(&index, sizeof(int));

        if(type.type == DataType_String)
        {
            StringList &list = *(StringList*)var;

            if(index >= list.Num())
            {
                AppWarning(TEXT("ScriptList::Remove - tried to remove an invalid index"));
                return;
            }

            list.Remove(index);
        }
        else
        {
            ScriptList &list = *(ScriptList*)var;
            list.listRemove(index, type);
        }
    }

    static void ENGINEAPI scriptRemoveItemBase(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack, BOOL bCaseInsensative)
    {
        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        if(type.type == DataType_String)
        {
            StringList &list = *(StringList*)var;

            String str;
            Stack.PopString(str);

            if(bCaseInsensative)
                list.RemoveItemI(str);
            else
                list.RemoveItem(str);
        }
        else
        {
            ScriptList &list = *(ScriptList*)var;

            LPBYTE value = (LPBYTE)Allocate(type.size);
            Stack.PopData(value, type.size);

            for(int i=0; i<list.Num(); i++)
            {
                LPBYTE lpItem = list.array+(i*type.size);

                if(mcmp(lpItem, value, type.size))
                {
                    list.listRemove(i, type);
                    break;
                }
            }

            if(type.type == DataType_Struct)
                Scripting->FreeTypeData(value, type);

            Free(value);
        }
    }

    static void ENGINEAPI scriptRemoveItem(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        scriptRemoveItemBase(var, subTypeName, Stack, TempStack, FALSE);
    }

    static void ENGINEAPI scriptRemoveItemI(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        scriptRemoveItemBase(var, subTypeName, Stack, TempStack, TRUE);
    }

    static void ENGINEAPI scriptSetSize(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        unsigned int size;
        Stack.PopData(&size, sizeof(unsigned int));

        if(type.type == DataType_String)
        {
            StringList &list = *(StringList*)var;
            list.SetSize(size);
        }
        else
        {
            ScriptList &list = *(ScriptList*)var;

            if(size == 0)
            {
                list.listClear(type);
                return;
            }
            else if(size == list.num)
                return;
            else if(size < list.num)
            {
                if(type.type == DataType_Struct)
                {
                    for(int i=size; i<list.num; i++)
                        Scripting->FreeTypeData(list.array+(i*type.size), type);
                }
            }

            list.num = size;
            list.array = (LPBYTE)ReAllocate(list.array, list.num*type.size);
        }
    }

    static void ENGINEAPI scriptNum(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        ScriptList &list = *(ScriptList*)var;
        TempStack.PushData(&list.num, sizeof(unsigned int));
    }

    static void ENGINEAPI scriptClear(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        if(type.type == DataType_String)
        {
            StringList &list = *(StringList*)var;
            list.Clear();
        }
        else
        {
            ScriptList &list = *(ScriptList*)var;
            list.listClear(type);
        }
    }

    static void ENGINEAPI scriptCopyList(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        if(type.type == DataType_String)
        {
            StringList &list = *(StringList*)var;
            StringList copyList;
            Stack.PopData(&copyList, sizeof(StringList));

            list.CopyList(copyList);

            //hack-a-licious
            zero(&copyList, sizeof(ScriptList));
        }
        else
        {
            ScriptList &list = *(ScriptList*)var;
            ScriptList copyList;
            Stack.PopData(&copyList, sizeof(ScriptList));

            list.listClear(type);

            list.num = copyList.num;
            list.array = (LPBYTE)Allocate(copyList.num*type.size);
            mcpy(list.array, copyList.array, copyList.num*type.size);

            if(type.type == DataType_Struct)
            {
                for(int i=0; i<list.num; i++)
                    Scripting->DuplicateTypeData(list.array+(i*type.size), type);
            }

            //hack-a-licious
            zero(&copyList, sizeof(ScriptList));
        }
    }

    static void ENGINEAPI scriptAppendList(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        if(type.type == DataType_String)
        {
            StringList &list = *(StringList*)var;
            StringList appendList;
            Stack.PopData(&appendList, sizeof(StringList));

            list.AppendList(appendList);

            //hack-a-licious
            zero(&appendList, sizeof(ScriptList));
        }
        else
        {
            ScriptList &list = *(ScriptList*)var;
            ScriptList appendList;
            Stack.PopData(&appendList, sizeof(ScriptList));

            unsigned int oldNum = list.num;
            list.num += appendList.num;
            list.array = (LPBYTE)ReAllocate(list.array, list.num*type.size);
            LPBYTE insertAddr = list.array+(oldNum*type.size);

            mcpy(insertAddr, appendList.array, appendList.num*type.size);

            if(type.type == DataType_Struct)
            {
                for(int i=0; i<appendList.num; i++)
                    Scripting->DuplicateTypeData(insertAddr+(i*type.size), type);
            }

            //hack-a-licious
            zero(&appendList, sizeof(ScriptList));
        }
    }

    static void ENGINEAPI scriptInsertList(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        if(type.type == DataType_String)
        {
            StringList &list = *(StringList*)var;
            StringList appendList;
            Stack.PopData(&appendList, sizeof(StringList));

            int index;
            Stack.PopData(&index, sizeof(int));

            list.InsertList(index, appendList);

            //hack-a-licious
            zero(&appendList, sizeof(ScriptList));
        }
        else
        {
            ScriptList &list = *(ScriptList*)var;
            ScriptList appendList;
            Stack.PopData(&appendList, sizeof(ScriptList));

            int index;
            Stack.PopData(&index, sizeof(int));

            if(index > list.num)
            {
                AppWarning(TEXT("ScriptList::InsertList - tried to insert array to invalid index"));
                zero(&appendList, sizeof(ScriptList));
                return;
            }

            unsigned int oldNum = list.num;
            list.num += appendList.num;
            list.array = (LPBYTE)ReAllocate(list.array, list.num*type.size);
            LPBYTE insertAddr = list.array+(index*type.size);
            mcpyrev(insertAddr+(appendList.num*type.size), insertAddr, (oldNum-index)*type.size);

            mcpy(insertAddr, appendList.array, appendList.num*type.size);

            if(type.type == DataType_Struct)
            {
                for(int i=0; i<appendList.num; i++)
                    Scripting->DuplicateTypeData(insertAddr+(i*type.size), type);
            }

            //hack-a-licious
            zero(&appendList, sizeof(ScriptList));
        }
    }

    static void ENGINEAPI scriptHasValueBase(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack, BOOL bCaseInsensative)
    {
        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        BOOL bHasValue = FALSE;

        if(type.type == DataType_String)
        {
            StringList &list = *(StringList*)var;
            StringList appendList;

            String value;
            Stack.PopString(value);

            if(bCaseInsensative)
                bHasValue = list.HasValueI(value);
            else
                bHasValue = list.HasValue(value);
        }
        else
        {
            ScriptList &list = *(ScriptList*)var;

            LPBYTE value = (LPBYTE)Allocate(type.size);
            Stack.PopData(value, type.size);

            for(unsigned int i=0; i<list.num; i++)
            {
                LPBYTE lpItem = list.array+(i*type.size);

                if(mcmp(lpItem, value, type.size))
                {
                    bHasValue = TRUE;
                    break;
                }
            }

            Free(value);
        }

        TempStack.PushData(&bHasValue, sizeof(BOOL));
    }

    static void ENGINEAPI scriptHasValue(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        scriptHasValueBase(var, subTypeName, Stack, TempStack, FALSE);
    }

    static void ENGINEAPI scriptHasValueI(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        scriptHasValueBase(var, subTypeName, Stack, TempStack, TRUE);
    }

    static void ENGINEAPI scriptFindValueIndexBase(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack, BOOL bCaseInsensative)
    {
        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        unsigned int index = INVALID;

        if(type.type == DataType_String)
        {
            StringList &list = *(StringList*)var;
            StringList appendList;

            String value;
            Stack.PopString(value);

            if(bCaseInsensative)
                index = list.FindValueIndexI(value);
            else
                index = list.FindValueIndex(value);
        }
        else
        {
            ScriptList &list = *(ScriptList*)var;

            LPBYTE value = (LPBYTE)Allocate(type.size);
            Stack.PopData(value, type.size);

            for(unsigned int i=0; i<list.num; i++)
            {
                LPBYTE lpItem = list.array+(i*type.size);

                if(mcmp(lpItem, value, type.size))
                {
                    index = i;
                    break;
                }
            }

            Free(value);
        }

        TempStack.PushData(&index, sizeof(unsigned int));
    }

    static void ENGINEAPI scriptFindValueIndex(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        scriptFindValueIndexBase(var, subTypeName, Stack, TempStack, FALSE);
    }

    static void ENGINEAPI scriptFindValueIndexI(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        scriptFindValueIndexBase(var, subTypeName, Stack, TempStack, TRUE);
    }

    static void ENGINEAPI scriptFindNextValueIndexBase(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack, BOOL bCaseInsensative)
    {
        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        unsigned int index;
        Stack.PopData(&index, sizeof(unsigned int));

        if(type.type == DataType_String)
        {
            StringList &list = *(StringList*)var;
            StringList appendList;

            String value;
            Stack.PopString(value);

            if(bCaseInsensative)
                index = list.FindNextValueIndexI(value, index);
            else
                index = list.FindNextValueIndex(value, index);
        }
        else
        {
            ScriptList &list = *(ScriptList*)var;

            LPBYTE value = (LPBYTE)Allocate(type.size);
            Stack.PopData(value, type.size);

            unsigned int i;
            for(i=(index+1); i<list.num; i++)
            {
                LPBYTE lpItem = list.array+(i*type.size);

                if(mcmp(lpItem, value, type.size))
                {
                    index = i;
                    break;
                }
            }

            if(i == list.num)
                index = INVALID;

            Free(value);
        }

        TempStack.PushData(&index, sizeof(unsigned int));
    }

    static void ENGINEAPI scriptFindNextValueIndex(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        scriptFindNextValueIndexBase(var, subTypeName, Stack, TempStack, FALSE);
    }

    static void ENGINEAPI scriptFindNextValueIndexI(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        scriptFindNextValueIndexBase(var, subTypeName, Stack, TempStack, TRUE);
    }

    static void ENGINEAPI scriptSwapValues(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
    {
        TypeInfo type;
        Compiler::GetTypeInfo(subTypeName, type);

        ScriptList &list = *(ScriptList*)var;

        unsigned int index1;
        Stack.PopData(&index1, sizeof(unsigned int));

        unsigned int index2;
        Stack.PopData(&index2, sizeof(unsigned int));

        BOOL bError = FALSE;
        if(index1 >= list.num)
        {
            AppWarning(TEXT("ScriptList::SwapValues - Invalid index specified for index 1"));
            bError = TRUE;
        }
        if(index2 >= list.num)
        {
            AppWarning(TEXT("ScriptList::SwapValues - Invalid index specified for index 2"));
            bError = TRUE;
        }
        if(bError) return;
        if(index1 == index2) return;

        LPBYTE value = (LPBYTE)Allocate(type.size);
        mcpy(value, list.array+(index1*type.size), type.size);
        mcpy(list.array+(index1*type.size), list.array+(index2*type.size), type.size);
        mcpy(list.array+(index2*type.size), value, type.size);
        Free(value);
    }

};

//------------------------------------------------------------------------

static void ENGINEAPI scriptString_ToInt(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    int base;
    Stack.PopData(&base, sizeof(int));

    int retVal = str.ToInt(base);
    TempStack.PushData(&retVal, sizeof(int));
}

static void ENGINEAPI scriptString_ToFloat(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    float retVal = str.ToFloat();
    TempStack.PushData(&retVal, sizeof(float));
}

static void ENGINEAPI scriptString_IsValid(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    BOOL retVal = str.IsValid();
    TempStack.PushData(&retVal, sizeof(BOOL));
}

static void ENGINEAPI scriptString_IsEmpty(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    BOOL retVal = str.IsEmpty();
    TempStack.PushData(&retVal, sizeof(BOOL));
}

static void ENGINEAPI scriptString_Left(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    int iEnd;
    Stack.PopData(&iEnd, sizeof(int));

    String retVal = str.Left(iEnd);
    TempStack.PushString(retVal);
}

static void ENGINEAPI scriptString_Mid(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    int iStart, iEnd;
    Stack.PopData(&iEnd,   sizeof(int));
    Stack.PopData(&iStart, sizeof(int));

    String retVal = str.Mid(iStart, iEnd);
    TempStack.PushString(retVal);
}

static void ENGINEAPI scriptString_Right(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    int iStart;
    Stack.PopData(&iStart, sizeof(int));

    String retVal = str.Right(iStart);
    TempStack.PushString(retVal);
}

static void ENGINEAPI scriptString_Clear(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;
    str.Clear();
}

static void ENGINEAPI scriptString_InsertString(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    String insertStr;
    int pos;
    Stack.PopString(insertStr);
    Stack.PopData(&pos, sizeof(int));

    str.InsertString(pos, insertStr);
}

static void ENGINEAPI scriptString_FindReplace(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    String strFind, strReplace;
    Stack.PopString(strReplace);
    Stack.PopString(strFind);

    str.FindReplace(strFind, strReplace);
}

static void ENGINEAPI scriptString_Compare(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    String strCmp;
    Stack.PopString(strCmp);

    BOOL retVal = str.Compare(strCmp);
    TempStack.PushData(&retVal, sizeof(BOOL));
}

static void ENGINEAPI scriptString_CompareI(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    String strCmp;
    Stack.PopString(strCmp);

    BOOL retVal = str.CompareI(strCmp);
    TempStack.PushData(&retVal, sizeof(BOOL));
}

static void ENGINEAPI scriptString_AppendChar(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    int ch;
    Stack.PopData(&ch, sizeof(int));

    str.AppendChar(ch);
}

static void ENGINEAPI scriptString_InsertChar(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    int ch, pos;
    Stack.PopData(&ch, sizeof(int));
    Stack.PopData(&pos, sizeof(int));

    str.InsertChar(pos, ch);
}

static void ENGINEAPI scriptString_RemoveChar(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    int pos;
    Stack.PopData(&pos, sizeof(int));

    str.RemoveChar(pos);
}

static void ENGINEAPI scriptString_Length(LPVOID var, CTSTR subTypeName, DataStack &Stack, DataStack &TempStack)
{
    String &str = *(String*)var;

    int len = str.Length();
    TempStack.PushData(&len, sizeof(int));
}

//------------------------------------------------------------------------

void ScriptSystem::AddTypeFunctions()
{
    AddTypeFunction(TEXT("string"),   TEXT("int    ToInt(int base=10)"),                            (DEFFUNCPROC)scriptString_ToInt);
    AddTypeFunction(TEXT("string"),   TEXT("int    ToInteger(int base=10)"),                        (DEFFUNCPROC)scriptString_ToInt);
    AddTypeFunction(TEXT("string"),   TEXT("float  ToFloat()"),                                     (DEFFUNCPROC)scriptString_ToFloat);
    AddTypeFunction(TEXT("string"),   TEXT("bool   IsValid()"),                                     (DEFFUNCPROC)scriptString_IsValid);
    AddTypeFunction(TEXT("string"),   TEXT("bool   IsEmpty()"),                                     (DEFFUNCPROC)scriptString_IsEmpty);
    AddTypeFunction(TEXT("string"),   TEXT("string Left(int iEnd)"),                                (DEFFUNCPROC)scriptString_Left);
    AddTypeFunction(TEXT("string"),   TEXT("string Mid(int iStart, int iEnd)"),                     (DEFFUNCPROC)scriptString_Mid);
    AddTypeFunction(TEXT("string"),   TEXT("string Right(int iStart)"),                             (DEFFUNCPROC)scriptString_Right);
    AddTypeFunction(TEXT("string"),   TEXT("void   Clear()"),                                       (DEFFUNCPROC)scriptString_Clear);
    AddTypeFunction(TEXT("string"),   TEXT("void   InsertString(int pos, string str)"),             (DEFFUNCPROC)scriptString_InsertString);
    AddTypeFunction(TEXT("string"),   TEXT("void   FindReplace(string strFind, string strReplace)"),(DEFFUNCPROC)scriptString_FindReplace);
    AddTypeFunction(TEXT("string"),   TEXT("bool   Compare(string str2)"),                          (DEFFUNCPROC)scriptString_Compare);
    AddTypeFunction(TEXT("string"),   TEXT("bool   CompareI(string str2)"),                         (DEFFUNCPROC)scriptString_CompareI);
    AddTypeFunction(TEXT("string"),   TEXT("void   AppendChar(char ch)"),                           (DEFFUNCPROC)scriptString_AppendChar);
    AddTypeFunction(TEXT("string"),   TEXT("void   InsertChar(int pos, char ch)"),                  (DEFFUNCPROC)scriptString_InsertChar);
    AddTypeFunction(TEXT("string"),   TEXT("void   RemoveChar(int pos)"),                           (DEFFUNCPROC)scriptString_RemoveChar);
    AddTypeFunction(TEXT("string"),   TEXT("int    Length()"),                                      (DEFFUNCPROC)scriptString_Length);

    AddTypeFunction(TEXT("list"), TEXT("int  Add(subtype item)"),                               (DEFFUNCPROC)ScriptList::scriptAdd);
    AddTypeFunction(TEXT("list"), TEXT("int  SafeAdd(subtype item)"),                           (DEFFUNCPROC)ScriptList::scriptSafeAdd);
    AddTypeFunction(TEXT("list"), TEXT("int  SafeAddI(subtype item)"),                          (DEFFUNCPROC)ScriptList::scriptSafeAddI);
    AddTypeFunction(TEXT("list"), TEXT("void Insert(int index, subtype item)"),                 (DEFFUNCPROC)ScriptList::scriptInsert);
    AddTypeFunction(TEXT("list"), TEXT("void Remove(int index)"),                               (DEFFUNCPROC)ScriptList::scriptRemove);
    AddTypeFunction(TEXT("list"), TEXT("void RemoveItem(subtype item)"),                        (DEFFUNCPROC)ScriptList::scriptRemoveItem);
    AddTypeFunction(TEXT("list"), TEXT("void RemoveItemI(subtype item)"),                       (DEFFUNCPROC)ScriptList::scriptRemoveItemI);
    AddTypeFunction(TEXT("list"), TEXT("void SetSize(int newSize)"),                            (DEFFUNCPROC)ScriptList::scriptSetSize);
    AddTypeFunction(TEXT("list"), TEXT("int  Num()"),                                           (DEFFUNCPROC)ScriptList::scriptNum);
    AddTypeFunction(TEXT("list"), TEXT("void Clear()"),                                         (DEFFUNCPROC)ScriptList::scriptClear);
    AddTypeFunction(TEXT("list"), TEXT("void CopyList(list<subtype>)"),                         (DEFFUNCPROC)ScriptList::scriptCopyList);
    AddTypeFunction(TEXT("list"), TEXT("void AppendList(list<subtype>)"),                       (DEFFUNCPROC)ScriptList::scriptAppendList);
    AddTypeFunction(TEXT("list"), TEXT("void InsertList(int index, list<subtype>)"),            (DEFFUNCPROC)ScriptList::scriptInsertList);
    AddTypeFunction(TEXT("list"), TEXT("bool HasValue(subtype item)"),                          (DEFFUNCPROC)ScriptList::scriptHasValue);
    AddTypeFunction(TEXT("list"), TEXT("bool HasValueI(subtype item)"),                         (DEFFUNCPROC)ScriptList::scriptHasValueI);
    AddTypeFunction(TEXT("list"), TEXT("int  FindValueIndex(subtype item)"),                    (DEFFUNCPROC)ScriptList::scriptFindValueIndex);
    AddTypeFunction(TEXT("list"), TEXT("int  FindValueIndexI(subtype item)"),                   (DEFFUNCPROC)ScriptList::scriptFindValueIndexI);
    AddTypeFunction(TEXT("list"), TEXT("int  FindNextValueIndex(subtype item, int index=-1)"),  (DEFFUNCPROC)ScriptList::scriptFindNextValueIndex);
    AddTypeFunction(TEXT("list"), TEXT("int  FindNextValueIndexI(subtype item, int index=-1)"), (DEFFUNCPROC)ScriptList::scriptFindNextValueIndexI);
}
