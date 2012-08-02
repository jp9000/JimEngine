/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Script

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
#include "ScriptBytecode.h"
#include "ScriptDefs.h"
#include "ScriptCompiler.h"


bool Compiler::WriteCurrentCodeOffset(FunctionDefinition *func)
{
    int size = lpTemp-lpLastTextOffset;

    if(func)
    {
        if(func->ByteCode.Num() < curCodeOffset)
            curCodeOffset = 0;
        else
        {
            if(curCodeOffset < func->ByteCode.Num())
            {
                curBytecodeOutput.AppendString(lpLastTextOffset, size);
                if(curBytecodeOutput.Right(1) != TEXT("\n"))
                    curBytecodeOutput << TEXT("\r\n");
                curBytecodeOutput << TEXT("-------------------------\r\n");
                WriteByteCode(func);
                curBytecodeOutput << TEXT("-------------------------\r\n");
                lpLastTextOffset = lpTemp;
                curCodeOffset = func->ByteCode.Num();
            }
        }
    }
    else
        curCodeOffset = 0;

    if(!func && !curBytecodeOutput.IsEmpty() && size)
    {
        curBytecodeOutput.AppendString(lpLastTextOffset, size);

        XFile bytecodeFile;

        String fileName;
        fileName << TEXT("ScriptOutput/") << curModule << TEXT("/Bytecode/") << GetPathFileName(curFile) << TEXT(".txt");
        if(bytecodeFile.Open(fileName, XFILE_WRITE, XFILE_CREATEALWAYS))
        {
            bytecodeFile.WriteAsUTF8(curBytecodeOutput);
            bytecodeFile.Close();
        }

        curBytecodeOutput.Clear();
    }

    return false;
}

static CTSTR ENGINEAPI TypeToString(VariableType type)
{
    switch(type)
    {
        case DataType_Integer:
            return TEXT("Integer");
        case DataType_Float:
            return TEXT("Float");

        case DataType_List:
            return TEXT("List");
        case DataType_String:
            return TEXT("String");

        case DataType_SubType:
            return TEXT("SubType");
        case DataType_Null:
            return TEXT("Null");
        case DataType_Any:
            return TEXT("Any");

        case DataType_Type:
            return TEXT("Type");

        case DataType_Void:
            return TEXT("Void");

        case DataType_Object:
            return TEXT("Object");
        case DataType_Struct:
            return TEXT("Struct");
    }

    return TEXT("");
};

void Compiler::WriteByteCode(FunctionDefinition *func)
{
    if(!func->ByteCode.Num())
        return;

    BufferInputSerializer data(func->ByteCode);
    data.Seek(curCodeOffset);

    List<BYTE> bulogna;

    while(data.DataPending())
    {
        UINT curCodePos = data.GetPos();

        BYTE instruction;
        data << instruction;

        switch(instruction)
        {
            case BCNop:
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ") << TEXT("BCNop\r\n");
                    break;
                }

            case BCPush:         //in: [int size][value]: push (value)
                {
                    int size;

                    data << size;

                    bulogna.SetSize(size);
                    data.Serialize(bulogna.Array(), size);

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPush ");
                    curBytecodeOutput << TEXT("[size: ") << IntString(size) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCPop:          //in: [int size]: pop value from the stack and into thin air
                {
                    int size;

                    data << size;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPop ");
                    curBytecodeOutput << TEXT("[size: ") << IntString(size) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            case BCPushDup:      //in: [int size]: push (duplicated stack val)
                {
                    int size;

                    data << size;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPushDup ");
                    curBytecodeOutput << TEXT("[size: ") << IntString(size) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCPushDupInit:  //in: [string type][string subType], push (duplicated stack val)
                {
                    String typeName, subTypeName;
                    data << typeName << subTypeName;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPushDupInit ");
                    curBytecodeOutput << TEXT("[type: ") << typeName << TEXT("]");
                    if(subTypeName.IsValid())
                        curBytecodeOutput << TEXT("[subType: ") << subTypeName << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            case BCAPushThis:    //apush: [curObj*]
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAPushThis");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            case BCAPushDup:     //apush: [address]
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAPushDup");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCAPush:        //in: [int scope][int pos][int arrayOffset]: apush: [address]
                {
                    int scope, pos;
                    BOOL arrayOffset;

                    data << scope << pos << arrayOffset;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAPush ");
                    curBytecodeOutput << TEXT("[scope: ") << IntString(scope) << TEXT("]");
                    curBytecodeOutput << TEXT("[pos: ") << IntString(pos) << TEXT("]");
                    curBytecodeOutput << TEXT("[arrayOffset: ") << (arrayOffset ? TEXT("true") : TEXT("false")) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCAPop:
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ") << TEXT("BCAPop\r\n");
                    break;
                }
            case BCAPushSVar:    //in: [int pos][bool istempstack][int arrayOffset]: apop: [struct*]: apush: [address]
                {
                    int pos;
                    BOOL tempstack, arrayOffset;

                    data << pos << tempstack << arrayOffset;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAPushSVar ");
                    curBytecodeOutput << TEXT("[pos: ") << IntString(pos) << TEXT("]");
                    curBytecodeOutput << TEXT("[tempstack: ") << (tempstack ? TEXT("true") : TEXT("false")) << TEXT("]");
                    curBytecodeOutput << TEXT("[arrayOffset: ") << (arrayOffset ? TEXT("true") : TEXT("false")) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCAPushCVar:    //in: [int pos][bool istempstack][int arrayOffset]: apop: [obj**]: apush: [address]
                {
                    int pos;
                    BOOL tempstack, arrayOffset;

                    data << pos << tempstack << arrayOffset;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAPushCVar ");
                    curBytecodeOutput << TEXT("[pos: ") << IntString(pos) << TEXT("]");
                    curBytecodeOutput << TEXT("[tempstack: ") << (tempstack ? TEXT("true") : TEXT("false")) << TEXT("]");
                    curBytecodeOutput << TEXT("[arrayOffset: ") << (arrayOffset ? TEXT("true") : TEXT("false")) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCAPushListItem://in: [string subType], apop: [list*], pop: [element], apush: [address]
                {
                    String sybType;
                    data << sybType;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAPushListItem ");
                    curBytecodeOutput << TEXT("[subType: ") << sybType << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCPushAData:    //in: [int size][bool noremove]: apop: [address]: push: [data]
                {
                    int size, noremove;

                    data << size << noremove;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPushAData ");
                    curBytecodeOutput << TEXT("[size: ") << IntString(size) << TEXT("]");
                    curBytecodeOutput << TEXT("[noremove: ") << IntString(noremove) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCPushADataInit://in: [string type][string subType][bool noremove]: apop: [address]: push: [data]
                {
                    String typeName, subTypeName;
                    int noremove;

                    data << typeName << subTypeName << noremove;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPushADataInit ");
                    curBytecodeOutput << TEXT("[type: ") << typeName << TEXT("]");
                    if(subTypeName.IsValid())
                        curBytecodeOutput << TEXT("[subType: ") << subTypeName << TEXT("]");
                    curBytecodeOutput << TEXT("[noremove: ") << IntString(noremove) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCAPopVarInit:      //in: [string typeName][string subTypeName], apop: [address], pop: [data], out (data)
                {
                    String typeName, subTypeName;
                    data << typeName << subTypeName;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAPopVarInit ");
                    curBytecodeOutput << TEXT("[typeName: ") << typeName << TEXT("]");
                    if(subTypeName.IsValid())
                        curBytecodeOutput << TEXT("[subTypeName: ") << subTypeName << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCAPopVar:      //in: [int size], apop: [address], pop: [data], out (data)
                {
                    int size;
                    data << size;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAPopVar ");
                    curBytecodeOutput << TEXT("[size: ") << IntString(size) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCAPushStack:   //apush: [address to data stack]
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAPushStack");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            case BCTPushStack:   //in: [int size], tpush [data], pop: [data]
                {
                    int size;
                    data << size;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCTPushStack ");
                    curBytecodeOutput << TEXT("[size: ") << IntString(size) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCTPushZero:    //in: [int size], tpush (zeroes)
                {
                    int size;
                    data << size;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCTPushZero ");
                    curBytecodeOutput << TEXT("[size: ") << IntString(size) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCTPop:         //in: [int size]: tpop: [data]
                {
                    int size;
                    data << size;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCTPop ");
                    curBytecodeOutput << TEXT("[size: ") << IntString(size) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCTPopFree:     //in: [string type][string subType], tpop: [data]
                {
                    String typeName, subTypeName;
                    data << typeName << subTypeName;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCTPopFree ");
                    curBytecodeOutput << TEXT("[type: ") << typeName << TEXT("]");
                    if(subTypeName.IsValid())
                        curBytecodeOutput << TEXT("[subType: ") << subTypeName << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCPushTData:    //in: [int size]: tpop: [data]: push [data]
                {
                    int size;

                    data << size;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPushTData ");
                    curBytecodeOutput << TEXT("[size: ") << IntString(size) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCAPushT:       //apush: [address to temp data stack]
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAPushT");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            case BCDynamicCast:  //in: [string classname], alter stack: [curObj* or null if uncastable]
                {
                    String strClass;
                    data << strClass;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCDynamicCast ");
                    curBytecodeOutput << TEXT("[strClass: \"") << strClass << TEXT("\"]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCCastFloat:    //alter stack: [int val]: (float)val;
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCCastFloat");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCCastInt:      //alter stack: [float val]: (int)val;
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCCastInt");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            case BCNegate:       //alter stack: [int val]: -val
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCNegate");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCNegateF:      //alter stack: [float val]: -val
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCNegateF");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            case BCCallFunc:     //in: [int funcID]: pop: [paramN-1: paramN-2: ...]: tpush (return value)
                {
                    int funcID;

                    data << funcID;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCCallFunc ");
                    curBytecodeOutput << TEXT("[funcID: 0x") << UIntString(funcID, 16) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCCallStructOp://in: [struct structName][int pos]: pop: [paramN-1: paramN-2: ...]: apop: [struct*]: tpush (return value)
            case BCCallStructFunc://in: [struct structName][int pos]: pop: [paramN-1: paramN-2: ...]: apop: [struct*]: tpush (return value)
                {
                    String name;
                    int pos;

                    data << name << pos;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    if(instruction == BCCallStructOp)
                        curBytecodeOutput << TEXT("BCCallStructOp ");
                    else
                        curBytecodeOutput << TEXT("BCCallStructFunc ");
                    curBytecodeOutput << TEXT("[structName: ") << name << TEXT("]");
                    curBytecodeOutput << TEXT("[pos: 0x") << UIntString(pos, 16) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCCallTypeFunc:
                {
                    String subType;
                    int type, pos;

                    data << type << pos << subType;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCCallTypeFunc ");
                    curBytecodeOutput << TEXT("[type: ") << TypeToString((VariableType)type) << TEXT("]");
                    curBytecodeOutput << TEXT("[pos: 0x") << UIntString(pos, 16) << TEXT("]");
                    if(subType.IsValid())
                        curBytecodeOutput << TEXT("[subType: \"") << subType << TEXT("\"]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCCallClassStFn://in: [string class][int funcID], pop: [paramN-1, paramN-2, ...], tpush (return value)
                {
                    String className;
                    int funcID;

                    data << className << funcID;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCCallClassStFn ");
                    curBytecodeOutput << TEXT("[className: ") << className << TEXT("]");
                    curBytecodeOutput << TEXT("[funcID: 0x") << UIntString(funcID, 16) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCCallClassFunc://in: [int pos]: pop: [paramN-1: paramN-2: ...]: apop: [class**]: tpush (return value)
                {
                    int pos, bsuper;

                    data << pos << bsuper;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCCallClassFunc ");
                    curBytecodeOutput << TEXT("[pos: 0x") << UIntString(pos, 16) << TEXT("]");
                    curBytecodeOutput << TEXT("[bSuper: ") << IntString(bsuper) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            case BCCreateObj:    //in: [string className], tpush: [obj]
                {
                    String className;
                    data << className;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCCreateObj ");
                    curBytecodeOutput << TEXT("[class: ") << className << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCDestroyObj:   //pop: [Object *obj]
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCDestroyObj\r\n");
                    break;
                }

            case BCReturn:       //in: [int size]: pop: [value]
                {
                    int size;

                    data << size;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCReturn ");
                    curBytecodeOutput << TEXT("[size: ") << IntString(size) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            case BCJump:         //in: [int offset]
                {
                    int offset;

                    data << offset;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCJump ");
                    curBytecodeOutput << TEXT("[offset: 0x") << IntString(offset, 16) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCJumpIf:       //in: [int offset]: pop: [bool value]
                {
                    int offset;

                    data << offset;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCJumpIf ");
                    curBytecodeOutput << TEXT("[offset: 0x") << IntString(offset, 16) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCJumpIfNot:    //in: [int offset]: pop: [bool value]
                {
                    int offset;

                    data << offset;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCJumpIfNot ");
                    curBytecodeOutput << TEXT("[offset: 0x") << IntString(offset, 16) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            case BCMultiply:     //pop: [int param2: int param1]: push (param1*param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCMultiply");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCDivide:       //pop: [int param2: int param1]: push (param1/param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCDivide");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCAdd:          //pop: [int param2: int param1]: push (param1+param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAdd");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCSubtract:     //pop: [int param2: int param1]: push (param1-param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCSubtract");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCMod:          //pop: [int param2: int param1]: push (param1%param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCMod");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            //--------------------------------------------------------
                                                         
            case BCEqual:        //pop: [int param2: int param1]: push (param1 == param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCEqual");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCNotEqual:     //pop: [int param2: int param1]: push (param1 != param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCNotEqual");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCGreaterEqual: //pop: [int param2: int param1]: push (param1 >= param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCGreaterEqual");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCLessEqual:    //pop: [int param2: int param1]: push (param1 <= param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCLessEqual");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCGreater:      //pop: [int param2: int param1]: push (param1 > param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCGreater");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCLess:         //pop: [int param2: int param1]: push (param1 < param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCLess");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            //--------------------------------------------------------

            case BCMultiplyF:    //pop: [float param2: float param1]: push (param1*param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCMultiplyF");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCDivideF:      //pop: [float param2: float param1]: push (param1/param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCDivideF");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCAddF:         //pop: [float param2: float param1]: push (param1+param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAddF");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCSubtractF:    //pop: [float param2: float param1]: push (param1-param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCSubtractF");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCModF:         //pop: [float param2: float param1]: push (param1%param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCModF");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            //--------------------------------------------------------

            case BCEqualF:       //pop: [float param2: float param1]: push (param1 == param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCEqualF");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCNotEqualF:    //pop: [float param2: float param1]: push (param1 != param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCNotEqualF");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCGreaterEqualF://pop: [float param2: float param1]: push (param1 >= param2)
                 {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCGreaterEqualF");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
           case BCLessEqualF:   //pop: [float param2: float param1]: push (param1 <= param2)
                 {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCLessEqualF");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
           case BCGreaterF:     //pop: [float param2: float param1]: push (param1 > param2)
                 {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCGreaterF");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
           case BCLessF:        //pop: [float param2: float param1]: push (param1 < param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCLessF");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            //--------------------------------------------------------

            case BCPushNullObj:  //push (null obj)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPushNullObj");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCPushNullStr:  //push (null str)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPushNullStr");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            //--------------------------------------------------------

            case BCEqualO:       //pop: [Object* param2: Object* param1]: push (param1 == param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCEqualO");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCNotEqualO:    //pop: [Object* param2: Object* param1]: push (param1 != param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCNotEqualO");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            //--------------------------------------------------------

            case BCBitwiseNot:   //alter stack: [int val]: ~val
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCBitwiseNot");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCBitwiseOr:    //pop: [int param2: int param1]: push (param1 | param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCBitwiseOr");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCBitwiseAnd:   //pop: [int param2: int param1]: push (param1 & param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCBitwiseAnd");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCShiftLeft:    //pop: [int param2: int param1]: push (param1 << param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCShiftLeft");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCShiftRight:   //pop: [int param2: int param1]: push (param1 >> param2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCShiftRight");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            //--------------------------------------------------------

            case BCLogicalNot:   //alter stack: [int val]: !val
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCLogicalNot");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            //--------------------------------------------------------

            case BCReturnS:      //pop: [String str]
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCReturnS");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            //--------------------------------------------------------

            case BCPushS:        //in: [String str]: push (value)
                {
                    String str;

                    data << str;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPushS ");
                    curBytecodeOutput << TEXT("[str: \"") << str << TEXT("\"]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCPopS:         //pop: [String str]
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPopS");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCPushDupS:     //push (dup string)
                {
                    String str;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPushDupS");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            //--------------------------------------------------------

            case BCPushADataS:   //in: [bool noremove]: apop: [address]: push: [data]
                {
                    int noremove;
                    data << noremove;

                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPushADataS ");
                    curBytecodeOutput << TEXT("[noremove: ") << IntString(noremove) << TEXT("]");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCAPopVarS:     //apop: [address]: pop: [data]: out (data)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAPopVarS");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCPushTDataS:   //tpop: [data]: push [data]
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCPushTDataS");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCTPopS:        //tpop: [string]
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCTPopS");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }

            //--------------------------------------------------------

            case BCAddS:         //pop: [String string2: String string1]: push (string1+string2)
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCAddS");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCEqualS:       //pop: [String string2: String string1]: push (string1.Compare(string2))
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCEqualS");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
            case BCNotEqualS:    //pop: [String string2: String string1]: push (string1.CompareI(string2))
                {
                    curBytecodeOutput << TEXT(": ") << FormattedString(TEXT("%04lX"), curCodePos) << TEXT(" -- ");
                    curBytecodeOutput << TEXT("BCNotEqualS");
                    curBytecodeOutput << TEXT("\r\n");
                    break;
                }
        }
    }
}