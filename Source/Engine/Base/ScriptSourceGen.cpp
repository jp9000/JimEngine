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
#include "ScriptDefs.h"
#include "ScriptCompiler.h"

CTSTR lpComment1 = TEXT("/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
CTSTR lpComment2 = TEXT("   This file was generated from script.\r\n\
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/\r\n\r\n");


String GetOperatorName(CTSTR lpName);
String ConvertTypeToString(Variable *var);
String ConvertParamTypeToString(DefaultVariable *var);


static const TCHAR *OperatorNames[VALIDOPS] = {TEXT("Add"), TEXT("Sub"), TEXT("Div"), TEXT("Mul"), TEXT("LShift"), TEXT("RShift"),
                                               TEXT("Equal"), TEXT("NotEqual"), TEXT("LessEqual"), TEXT("GreaterEqual"), TEXT("Less"),  TEXT("Greater"),
                                               TEXT("Not"), TEXT("BitNot"), TEXT("Assign"), TEXT("Mod")};



String GetOperatorName(CTSTR lpName)
{
    for(int i=0; i<VALIDOPS; i++)
    {
        if(scmp(ValidOperators[i], lpName) == 0)
            return String(OperatorNames[i]);
    }

    return String();
}

String ConvertTypeToString(TypeInfo &ti, TypeInfo *subType=NULL)
{
    if(!ti.name || !ti.name[0])
        return String() << TEXT("void");
    else if(ti.type == DataType_Object)
    {
        Class *cls = FindClass(ti.name);
        assert(cls);
        while(cls->IsPureScriptClass()) cls = cls->GetParent();
        return String() << cls->GetName() << TEXT("*");
    }
    else if(ti.type == DataType_String)
        return String() << TEXT("String");
    else if(ti.type == DataType_List)
        return String() << TEXT("List<") << ConvertTypeToString(*subType) << TEXT(">");
    else if(ti.type == DataType_Type)
        return String() << TEXT("TypeDataInfo");
    else if(scmp(ti.name, TEXT("bool")) == 0)
        return String() << TEXT("BOOL");
    else if(scmp(ti.name, TEXT("icolor")) == 0)
        return String() << TEXT("DWORD");
    else if(scmp(ti.name, TEXT("handle")) == 0)
        return String() << TEXT("HANDLE");
    else
        return String() << ti.name;
}

String ConvertParamTypeToString(DefaultVariable *var)
{
    if(!var->typeInfo.name || !var->typeInfo.name[0])
        return String() << TEXT("Error");
    else if(var->typeInfo.type == DataType_Object)
        return String() << var->typeInfo.name << ((var->flags & VAR_OUT) ? TEXT("*&") : TEXT("*"));
    else if(var->typeInfo.type == DataType_List)
    {
        if(~var->flags & VAR_OUT)
            return String() << TEXT("const List<") << ConvertTypeToString(var->subTypeInfo) << TEXT(">&");
        else
            return String() << TEXT("List<") << ConvertTypeToString(var->subTypeInfo) << TEXT(">&");
    }
    else if(var->typeInfo.type == DataType_String)
    {
        if(!(var->flags & VAR_OUT))
            return String(TEXT("CTSTR"));
        else
            return String(TEXT("String&"));
    }
    else if(var->typeInfo.type == DataType_Type)
        return String() << TEXT("TypeDataInfo&");
    else if(var->typeInfo.type == DataType_Struct)
    {
        if(!(var->flags & VAR_OUT))
            return String() << TEXT("const ") << var->typeInfo.name << TEXT("&");
        else
            return String() << var->typeInfo.name << TEXT("&");
    }
    else if(scmp(var->typeInfo.name, TEXT("icolor")) == 0)
        return String() << TEXT("DWORD") << ((var->flags & VAR_OUT) ? TEXT("&") : TEXT(""));
    else if(scmp(var->typeInfo.name, TEXT("handle")) == 0)
        return String() << TEXT("HANDLE") << ((var->flags & VAR_OUT) ? TEXT("&") : TEXT(""));
    else if(scmp(var->typeInfo.name, TEXT("bool")) == 0)
        return String() << TEXT("BOOL") << ((var->flags & VAR_OUT) ? TEXT("&") : TEXT(""));
    else
        return String() << var->typeInfo.name << ((var->flags & VAR_OUT) ? TEXT("&") : TEXT(""));
}



void Compiler::CreateSourceComment()
{
    curSourceFile.WriteAsUTF8(lpComment1);
    curSourceFile.WriteAsUTF8(String() << TEXT("   ") << curFile << TEXT("\r\n\r\n"));
    curSourceFile.WriteAsUTF8(lpComment2);
}

void Compiler::CreateSourceClass(ClassDefinition *classDef)
{
    if(classDef->classData->bPureScriptClass)
        return;

    String str;

    String parentName = classDef->Parent ? classDef->Parent->classData->name : TEXT("OrphanageWorker");

    str << TEXT("\r\nclass ") << classDef->classData->name;
    str << TEXT(" : public ") << parentName;
    str << TEXT("\r\n{\r\n    ");

    str << TEXT("DeclareClass(");

    str << classDef->classData->name << TEXT(", ") << parentName << TEXT(");\r\n");

    BOOL bFoundNativeClassLinks = FALSE;

    if(classDef->Variables.Num() || classDef->Functions.Num())
        str << TEXT("\r\npublic:\r\n");

    BOOL bFoundSomething = FALSE;
    BOOL bFoundVars = FALSE;

    if(classDef->Variables.Num())
    {
        if(strNativeClassLinks.IsEmpty())
            strNativeClassLinks << TEXT("\r\n    Class* curClass;\r\n");
        strNativeClassLinks << TEXT("\r\n    curClass = GetClass(") << classDef->classData->name << TEXT(");\r\n    assert(curClass);\r\n\r\n    if(curClass)\r\n    {\r\n");

        bFoundNativeClassLinks = TRUE;
        bFoundVars = TRUE;

        if(!bFoundSomething)
        {
            str << TEXT("    //<Script module=\"") << curModule << TEXT("\" classdecs=\"") << classDef->classData->name << ("\">");
            bFoundSomething = TRUE;
        }

        for(int i=0; i<classDef->Variables.Num(); i++)
        {
            DefaultVariable *var = &classDef->Variables[i];

            str << TEXT("\r\n    ");

            str << ConvertTypeToString(var->typeInfo) << TEXT(" ") << var->name;

            if(var->numElements)
                str << FormattedString(TEXT("[%d]"), var->numElements);

            str << TEXT(";");

            strNativeClassLinks << TEXT("        curClass->DefineNativeVariable(offsetof(") << classDef->classData->name
                                << TEXT(", ") << var->name << TEXT("), ") << IntString(i) << TEXT(");\r\n");
        }
    }

    String classDecs;
    String internalDecs;

    BOOL bFoundInternalFunc = FALSE;
    BOOL bFoundImplementable = FALSE;

    for(int i=0; i<classDef->Functions.Num(); i++)
    {
        FunctionDefinition *func = &classDef->Functions[i];

        if(func->flags & FUNC_IMPLEMENTABLE)
        {
            if(!bFoundSomething)
            {
                classDecs << TEXT("    //<Script module=\"") << curModule << TEXT("\" classdecs=\"") << classDef->classData->name << ("\">");
                bFoundSomething = TRUE;
            }

            if(!bFoundImplementable)
            {
                if(bFoundVars)
                    classDecs << TEXT("\r\n");
                bFoundImplementable = TRUE;
            }

            classDecs << TEXT("\r\n    ") << ConvertTypeToString(func->returnType) << TEXT(" script") << func->name << TEXT("(");

            for(int j=0; j<func->Params.Num(); j++)
            {
                if(j) classDecs << TEXT(", ");
                DefaultVariable *var = &func->Params[j];

                classDecs << ConvertParamTypeToString(var) << TEXT(" ") << var->name;
            }

            if(func->Params.Num())
                classDecs << TEXT(")\r\n    {\r\n        CallStruct cs;\r\n        cs.SetNumParams(") << UIntString(func->Params.Num()) << TEXT(");\r\n");
            else
                classDecs << TEXT(")\r\n    {\r\n        CallStruct cs;");

            for(int j=0; j<func->Params.Num(); j++)
            {
                DefaultVariable *var = &func->Params[j];

                classDecs << TEXT("        cs.Set");
                
                switch(var->typeInfo.type)
                {
                    case DataType_Integer: classDecs << TEXT("Int");          break;
                    case DataType_Float:   classDecs << TEXT("Float");        break;
                    case DataType_Handle:  classDecs << TEXT("Handle");       break;
                    case DataType_Object:  classDecs << TEXT("Object");       break;
                    case DataType_String:  classDecs << TEXT("String");       break;
                    case DataType_Struct:  classDecs << TEXT("Struct");       break;
                    case DataType_Type:    classDecs << TEXT("TypeDataInfo"); break;
                }

                classDecs << TEXT("(") << UIntString(j) << TEXT(", ");

                if(var->typeInfo.type == DataType_Object)
                    classDecs << TEXT("(Object*)");
                else if((var->typeInfo.type == DataType_Integer) && (scmp(var->typeInfo.name, TEXT("int")) != 0))
                    classDecs << TEXT("(int)");

                if(var->typeInfo.type == DataType_Struct)
                    classDecs << TEXT("&") << var->name << TEXT(", ") << UIntString(var->typeInfo.size);
                else
                    classDecs << var->name;

                classDecs << TEXT(");\r\n");
            }

            classDecs << TEXT("\r\n        GetLocalClass()->CallScriptMember(this, ") << UIntString(func->funcOffset-classDef->functionStartIndex) << TEXT(", cs);\r\n");

            if(func->returnType.type != DataType_Void)
            {
                classDecs << TEXT("\r\n        return ");

                if(func->returnType.type == DataType_Struct)
                    classDecs << TEXT("(") << func->returnType.name << TEXT("&)cs.GetStruct(RETURNVAL);\r\n");
                else
                {
                    classDecs << TEXT("cs.Get");
                    switch(func->returnType.type)
                    {
                        case DataType_Integer: classDecs << TEXT("Int");      break;
                        case DataType_Float:   classDecs << TEXT("Float");    break;
                        case DataType_Struct:  classDecs << TEXT("Struct");   break;
                        case DataType_Handle:  classDecs << TEXT("Handle");   break;
                        case DataType_Object:  classDecs << TEXT("Object");   break;
                        case DataType_String:  classDecs << TEXT("String");   break;
                        case DataType_Type:    classDecs << TEXT("TypeInfo"); break;
                    }

                    classDecs << TEXT("(RETURNVAL);\r\n"); 
                }
            }

            classDecs << TEXT("    }\r\n");
        }
        if(func->flags & FUNC_INTERNAL)
        {
            if(!bFoundSomething)
            {
                classDecs << TEXT("    //<Script module=\"") << curModule << TEXT("\" classdecs=\"") << classDef->classData->name << ("\">");
                bFoundSomething = TRUE;
            }

            String funcName;
            funcName << TEXT("native_");
            if(func->name.IsEmpty()) //constructor
                funcName << classDef->classData->name;
            else
                funcName << func->name;
            CreateSourceNativeFuncDef(classDef, NULL, func, funcName);

            if(!bFoundNativeClassLinks)
            {
                if(strNativeClassLinks.IsEmpty())
                    strNativeClassLinks << TEXT("\r\n    Class* curClass;\r\n");
                strNativeClassLinks << TEXT("\r\n    curClass = GetClass(") << classDef->classData->name << TEXT(");\r\n    assert(curClass);\r\n\r\n    if(curClass)\r\n    {\r\n");

                bFoundNativeClassLinks = TRUE;
            }
            else if(!bFoundInternalFunc)
                strNativeClassLinks << TEXT("\r\n");

            if(func->flags & FUNC_STATIC)
            {
                strNativeClassLinks << FormattedString(TEXT("        curClass->DefineNativeStaticMember((NATIVECALLBACK)&%s::%s, 0x%lX);\r\n"), classDef->classData->name, funcName.Array(), i);
                internalDecs << TEXT("    Declare_Internal_StaticMember(") << funcName << TEXT(");\r\n");
            }
            else
            {
                strNativeClassLinks << FormattedString(TEXT("        curClass->DefineNativeMember((OBJECTCALLBACK)&%s::%s, 0x%lX);\r\n"), classDef->classData->name, funcName.Array(), i);
                internalDecs << TEXT("    Declare_Internal_Member(") << funcName << TEXT(");\r\n");
            }

            bFoundInternalFunc = TRUE;
        }
    }

    if(bFoundNativeClassLinks)
    {
        strNativeClassLinks << TEXT("    }\r\n");

        if(bFoundVars && !bFoundImplementable && internalDecs.IsValid())
            classDecs << TEXT("\r\n");

        classDecs << TEXT("\r\n") << internalDecs;
    }

    if(bFoundSomething)
    {
        classDecs << TEXT("    //</Script>");
        str << classDecs;
    }

    str << TEXT("\r\n};\r\n");

    //----------------------------------

    strCurClasses << str;

    //----------------------------------

    strForwards << TEXT("class ") << classDef->classData->name << TEXT(";\r\n");

    //----------------------------------

    if(classDef->classData->IsAbstract())
        strCurClassDefs << TEXT("DefineAbstractClass(");
    else
        strCurClassDefs << TEXT("DefineClass("); 
    strCurClassDefs << classDef->classData->name << TEXT(");\r\n");
}

void Compiler::CreateSourceStruct(StructDefinition *structDef)
{
    String str;

    str << TEXT("\r\nstruct ") << structDef->name << TEXT("\r\n{\r\n");

    for(int i=0; i<structDef->Variables.Num(); i++)
    {
        Variable *var = &structDef->Variables[i];
        str << TEXT("    ") << ConvertTypeToString(var->typeInfo) << TEXT(" ") << var->name << TEXT(";\r\n");
    }

    //----------------------------------

    String internalDecs;
    BOOL bFoundInternalFunc = FALSE;

    for(int i=0; i<structDef->Functions.Num(); i++)
    {
        FunctionDefinition *func = &structDef->Functions[i];

        if(func->flags & FUNC_INTERNAL)
        {
            String funcName;

            if(func->flags & FUNC_OPERATOR)
            {
                String operatorName;
                if((func->name == TEXT("-")) && (func->Params.Num() == 0))
                    operatorName = TEXT("Negate");
                else
                    operatorName = GetOperatorName(func->name);

                if(func->Params.Num() == 1)
                    funcName << TEXT("Native_operator_") << operatorName << TEXT("_") << func->Params[0].typeInfo.name;
                if(func->Params.Num() == 0)
                    funcName << TEXT("Native_operator_") << operatorName;
            }
            else
            {
                funcName << TEXT("native_");
                if(func->name.IsEmpty()) //constructor
                    funcName << structDef->name;
                else
                    funcName << func->name;
            }

            CreateSourceNativeFuncDef(NULL, structDef, func, funcName);

            if(!bFoundInternalFunc)
                strNativeStructLinks << TEXT("\r\n");

            if(func->flags & FUNC_STATIC)
            {
                internalDecs << TEXT("    Declare_Internal_StaticMember(") << funcName << TEXT(");\r\n");
                strNativeStructLinks << FormattedString(TEXT("    DefineNativeStructStaticMember(TEXT(\"%s\"), (NATIVECALLBACK)&%s::%s, 0x%lX);\r\n"), structDef->name.Array(), structDef->name.Array(), funcName.Array(), i);
            }
            else
            {
                internalDecs << TEXT("    Declare_Internal_Member(") << funcName << TEXT(");\r\n");
                strNativeStructLinks << FormattedString(TEXT("    DefineNativeStructMember(TEXT(\"%s\"), (OBJECTCALLBACK)&%s::%s, 0x%lX);\r\n"), structDef->name.Array(), structDef->name.Array(), funcName.Array(), i);
            }

            bFoundInternalFunc = TRUE;
        }
    }

    if(bFoundInternalFunc)
    {
        str << TEXT("\r\n    //<Script module=\"") << curModule << TEXT("\" structdecs=\"") << structDef->name << TEXT("\">\r\n");
        str << internalDecs;
        str << TEXT("    //</Script>\r\n");
    }

    //----------------------------------

    str << TEXT("};\r\n");

    if(strCurStructs.IsEmpty())
        strCurStructs << TEXT("\r\n");

    strCurStructs << str;

    //----------------------------------

    strForwards << TEXT("struct ") << structDef->name << TEXT(";\r\n");
}

void Compiler::CreateSourceEnum(EnumDefinition *enumDef)
{
    String str;

    if(!enumDef->name.IsEmpty())
        str << TEXT("\r\nenum ") << enumDef->name << TEXT("\r\n{\r\n");
    else
        str << TEXT("\r\nenum\r\n{\r\n");

    int id = 0;
    for(int i=0; i<enumDef->Items.Num(); i++)
    {
        EnumItem *item = &enumDef->Items[i];
        if(item->val != id)
        {
            if((unsigned int)item->val >= 0x1000)
                str << TEXT("    ") << item->name << TEXT("=0x") << UIntString(item->val, 16) << TEXT(",\r\n");
            else
                str << TEXT("    ") << item->name << TEXT("=") << UIntString(item->val) << TEXT(",\r\n");
            id = item->val;
        }
        else
            str << TEXT("    ") << item->name << TEXT(",\r\n");

        ++id;
    }

    str << TEXT("};\r\n");

    //----------------------------------

    strCurEnums << str;

    //----------------------------------

    if(!enumDef->name.IsEmpty())
        strForwards << TEXT("enum ") << enumDef->name << TEXT(";\r\n");
}

void Compiler::CreateSourceGlobalNative(FunctionDefinition *func)
{
    String funcName;

    if(func->flags & FUNC_OPERATOR)
    {
        String operatorName;
        if((func->name == TEXT("-")) && (func->Params.Num() == 1))
            operatorName = TEXT("Negate");
        else
            operatorName = GetOperatorName(func->name);

        if(func->Params.Num() == 2)
            funcName << TEXT("Native_operator_") << func->Params[0].typeInfo.name << TEXT("_") << operatorName << TEXT("_") << func->Params[1].typeInfo.name;
        if(func->Params.Num() == 1)
            funcName << TEXT("Native_operator_") << func->returnType.name << TEXT("_") << operatorName;
    }
    else
        funcName << TEXT("NativeGlobal_") << func->name;

    CreateSourceNativeFuncDef(NULL, NULL, func, funcName);

    //----------------------------------

    strNativeLinks << TEXT("    Scripting->DefineNativeGlobal((NATIVECALLBACK)&") << funcName << TEXT(", 0x") << UIntString(func->funcOffset, 16) << TEXT(");\r\n");

    //----------------------------------

    strCurNativeDecs << TEXT("Declare_Native_Global(") << funcName << TEXT(");\r\n");
}

void Compiler::CreateSourceNativeFuncDef(ClassDefinition *classDef, StructDefinition *structDef, FunctionDefinition *func, String& funcName)
{
    String str;

    int count=0;

    if(classDef)
    {
        FunctionDefinition *curFunc = NULL;
        for(int i=0; i<classDef->Functions.Num(); i++)
        {
            curFunc = &classDef->Functions[i];
            if(curFunc == func)
                break;
            else if(scmp(curFunc->name, func->name) == 0)
                ++count;
        }
    }
    else if(!(func->flags & FUNC_OPERATOR))
    {
        if(structDef)
        {
            FunctionDefinition *curFunc = NULL;
            for(int i=0; i<structDef->Functions.Num(); i++)
            {
                curFunc = &structDef->Functions[i];
                if(curFunc == func)
                    break;
                else if(scmp(curFunc->name, func->name) == 0)
                    ++count;
            }
        }
        else
        {
            ModuleScriptData *module = Scripting->GetModule(curModule);
            FunctionDefinition *curFunc = NULL;
            for(int i=0; i<module->GlobalFunctionList.Num(); i++)
            {
                curFunc = &module->GlobalFunctionList[i];
                if(curFunc == func)
                    break;
                else if(scmp(curFunc->name, func->name) == 0)
                    ++count;
            }
        }
    }

    if(count)
        funcName << TEXT("_") << UIntString(count+1);

    str << TEXT("\r\nvoid ");

    if(func->flags & FUNC_STATIC)
        str << TEXT("ENGINEAPI ");

    if(classDef)
        str << classDef->classData->name << TEXT("::");
    else if(structDef)
        str << structDef->name << TEXT("::");
    else
        str << TEXT("ENGINEAPI ");

    str << funcName << TEXT("(CallStruct &cs)\r\n{\r\n");

    BOOL bFoundSomething = FALSE;

    if(func->returnType.type != DataType_Void)
    {
        String paramDefiner = ConvertTypeToString(func->returnType);

        str << TEXT("    ") << paramDefiner << TEXT("& returnVal = ");

        if( (func->returnType.type == DataType_Object) ||
            (func->returnType.type == DataType_Struct) ||
            ((func->returnType.type == DataType_Integer) && (scmp(func->returnType.name, TEXT("int")) != 0)))
        {
            str << TEXT("(") << paramDefiner << TEXT("&)");
        }

        str << TEXT("cs.Get");

        if(func->returnType.type == DataType_Object)
            str << TEXT("Object");
        else if(func->returnType.type == DataType_Handle)
            str << TEXT("Handle");
        else if(func->returnType.type == DataType_String)
            str << TEXT("String");
        else if(func->returnType.type == DataType_Struct)
            str << TEXT("Struct");
        else if(func->returnType.name == TEXT("bool"))
            str << TEXT("Bool");
        else if(func->returnType.type == DataType_Integer)
            str << TEXT("Int");
        else if(func->returnType.type == DataType_Float)
            str << TEXT("Float");
        else if(func->returnType.type == DataType_Type)
            str << TEXT("TypeInfo");

        str << TEXT("Out");

        str << TEXT("(RETURNVAL);\r\n");

        bFoundSomething = TRUE;
    }

    for(int i=0; i<func->Params.Num(); i++)
    {
        DefaultVariable *var = &func->Params[i];

        String paramDefiner = ConvertTypeToString(var->typeInfo, &var->subTypeInfo);

        if(var->flags & VAR_OUT)
        {
            str << TEXT("    ") << paramDefiner << TEXT(" &") << var->name << TEXT(" = ");
            if( (var->typeInfo.type == DataType_Object) ||
                (var->typeInfo.type == DataType_Struct) ||
                (var->typeInfo.type == DataType_List)   ||
                ((var->typeInfo.type == DataType_Integer) && (scmp(var->typeInfo.name, TEXT("int")) != 0)))
            {
                str << TEXT("(") << paramDefiner << TEXT("&)");
            }
        }
        else
        {
            if( (var->typeInfo.type == DataType_Struct) ||
                (var->typeInfo.type == DataType_List)   )
            {
                paramDefiner.InsertString(0, TEXT("const "));
                str << TEXT("    ") << paramDefiner << TEXT(" &") << var->name << TEXT(" = ");
                str << TEXT("(") << paramDefiner << TEXT("&)");
            }
            else
            {
                str << TEXT("    ") << paramDefiner << TEXT(" ") << var->name << TEXT(" = ");
                if( (var->typeInfo.type == DataType_Object) ||
                    ((var->typeInfo.type == DataType_Integer) && (scmp(var->typeInfo.name, TEXT("int")) != 0)))
                {
                    str << TEXT("(") << paramDefiner << TEXT(")");
                }
            }
        }

        str << TEXT("cs.Get");

        if(var->typeInfo.type == DataType_Object)
            str << TEXT("Object");
        else if(var->typeInfo.type == DataType_Handle)
            str << TEXT("Handle");
        else if(var->typeInfo.type == DataType_String)
            str << TEXT("String");
        else if(var->typeInfo.type == DataType_Struct)
            str << TEXT("Struct");
        else if(var->typeInfo.name == TEXT("bool"))
            str << TEXT("Bool");
        else if(var->typeInfo.type == DataType_Integer)
            str << TEXT("Int");
        else if(var->typeInfo.type == DataType_List)
            str << TEXT("List");
        else if(var->typeInfo.type == DataType_Float)
            str << TEXT("Float");

        if(var->flags & VAR_OUT)
            str << TEXT("Out");

        str << FormattedString(TEXT("(%d);\r\n"), i);

        bFoundSomething = TRUE;
    }

    if(bFoundSomething)
        str << TEXT("\r\n");

    //------------------------------------------------

    //str << TEXT("    //code goes here\r\n}\r\n");

    if(func->returnType.type != DataType_Void)
        str << TEXT("    returnVal = ");
    else
        str << TEXT("    ");

    if(func->flags & FUNC_OPERATOR)
    {
        if(structDef)
        {
            if(func->Params.Num() == 1)
                str << TEXT("(") << TEXT("*this ") << func->name << TEXT(" ") << func->Params[0].name;
            else
                str << func->name << TEXT("(*this");
        }
        else
        {
            if(func->Params.Num() == 2)
                str << TEXT("(") << func->Params[0].name << TEXT(" ") << func->name << TEXT(" ") << func->Params[1].name;
            else
                str << TEXT("(") << func->name << func->Params[0].name;
        }
    }
    else
    {
        str << func->name << TEXT("(");

        for(int i=0; i<func->Params.Num(); i++)
        {
            Variable *var = &func->Params[i];

            if(i)
                str << TEXT(", ");

            str << var->name;
        }
    }

    str << TEXT(");\r\n}\r\n");

    //------------------------------------------------

    if(classDef)
        strCurNativeClassDefs << str;
    else if(structDef)
        strCurNativeStructDefs << str;
    else
        strCurNativeDefs << str;
}


void Compiler::SaveCurrentSource()
{
    if( strCurEnums.IsValid()       ||
        strCurStructs.IsValid()     ||
        strCurClasses.IsValid()     ||
        strCurNativeDecs.IsValid()  )
    {
        String scriptFile = curFile;
        String baseFileName = GetPathFileName(scriptFile, FALSE);
        curFile.Clear();
        curFile << baseFileName << TEXT(".h");

        strIncludes << TEXT("#include \"") << curFile << TEXT("\"\r\n");

        //------------------------------------

        String filePath;
        filePath << TEXT("ScriptOutput/") << curModule << TEXT("/Headers/") << curFile;
        if(!curSourceFile.Open(filePath, XFILE_WRITE, XFILE_CREATEALWAYS))
        {
            AddError(TEXT("Could not open path '%s' for source generation"), filePath.Array());
            return;
        }

        curSourceFile.Write("\xEF\xBB\xBF", 3);
        CreateSourceComment();
        curSourceFile.WriteAsUTF8(strCurEnums);

        curSourceFile.WriteAsUTF8(strCurStructs);
        curSourceFile.WriteAsUTF8(strCurClasses);
        curSourceFile.Write("\r\n", 2);
        if(strCurNativeDecs.IsValid())
        {
            curSourceFile.WriteAsUTF8(TEXT("//<Script module=\""));
            curSourceFile.WriteAsUTF8(curModule);
            curSourceFile.WriteAsUTF8(TEXT("\" globaldecs=\""));
            curSourceFile.WriteAsUTF8(scriptFile);
            curSourceFile.WriteAsUTF8(TEXT("\">\r\n"));
            curSourceFile.WriteAsUTF8(strCurNativeDecs);
            curSourceFile.WriteAsUTF8(TEXT("//</Script>\r\n"));
            strCurNativeDecs.Clear();
        }
        strCurEnums.Clear();
        strCurStructs.Clear();
        strCurClasses.Clear();

        curSourceFile.Close();

        //------------------------------------

        if( strCurClassDefs.IsValid()        ||
            strCurNativeClassDefs.IsValid()  ||
            strCurNativeStructDefs.IsValid() ||
            strCurNativeDefs.IsValid()       )
        {
            filePath.Clear();

            curFile.Clear();
            curFile << GetPathFileName(scriptFile, FALSE) << TEXT(".cpp");

            String sourcePath;

            filePath << TEXT("ScriptOutput/") << curModule << TEXT("/Source/") << curFile;
            if(!curSourceFile.Open(filePath, XFILE_WRITE, XFILE_CREATEALWAYS))
            {
                AddError(TEXT("Could not open path '%s' for source generation"), filePath.Array());
                return;
            }

            curSourceFile.Write("\xEF\xBB\xBF", 3);
            CreateSourceComment();
            curSourceFile.WriteAsUTF8(TEXT("\r\n#include \"ScriptHeaders.h\"\r\n\r\n\r\n"));
            curSourceFile.WriteAsUTF8(strCurClassDefs);
            curSourceFile.Write("\r\n", 2);

            if( strCurNativeClassDefs.IsValid()  ||
                strCurNativeStructDefs.IsValid() ||
                strCurNativeDefs.IsValid()       )
            {
                curSourceFile.WriteAsUTF8(TEXT("//<Script module=\""));
                curSourceFile.WriteAsUTF8(curModule);
                curSourceFile.WriteAsUTF8(TEXT("\" filedefs=\""));
                curSourceFile.WriteAsUTF8(scriptFile);
                curSourceFile.WriteAsUTF8(TEXT("\">"));
                curSourceFile.WriteAsUTF8(strCurNativeClassDefs);
                curSourceFile.WriteAsUTF8(strCurNativeStructDefs);
                curSourceFile.WriteAsUTF8(strCurNativeDefs);
                curSourceFile.WriteAsUTF8(TEXT("//</Script>\r\n"));
                strCurNativeDefs.Clear();
                strCurNativeStructDefs.Clear();
                strCurNativeClassDefs.Clear();
            }

            strCurClassDefs.Clear();

            curSourceFile.Close();
        }
    }
}

BOOL Compiler::CreateMainSourceFiles()
{
    String str, strFileName;

    curFile = TEXT("ScriptHeaders.h");
    str << TEXT("#include <Base.h>\r\n\r\n") << strForwards << TEXT("\r\n") << strIncludes;
    strForwards.Clear();
    strIncludes.Clear();

    //----------------------------------

    strFileName << TEXT("ScriptOutput/") << curModule << TEXT("/Headers/") << curFile;
    if(curSourceFile.Open(strFileName, XFILE_WRITE, XFILE_CREATEALWAYS))
    {
        curSourceFile.Write("\xEF\xBB\xBF", 3);
        CreateSourceComment();
        curSourceFile.WriteAsUTF8(str);
        curSourceFile.Close();
        str.Clear();
    }
    else
    {
        AddError(TEXT("Could not open file '%s'"), strFileName.Array());
        return FALSE;
    }

    //----------------------------------

    curFile = TEXT("ScriptMain.cpp");
    str << TEXT("\r\n#include \"ScriptHeaders.h\"\r\n\r\n");

    if(!strNativeClassLinks.IsEmpty() || !strNativeStructLinks.IsEmpty() || !strNativeLinks.IsEmpty())
    {
        str << TEXT("Load_Module_Natives();\r\n\r\n");
        str << TEXT("void ENGINEAPI LoadModuleNatives()\r\n{\r\n    //<Script module=\"") << curModule << TEXT("\" nativeloader>");
        
        if(strNativeClassLinks.IsEmpty())
            str << TEXT("\r\n");
        else
            str << strNativeClassLinks;
        
        if(!strNativeStructLinks.IsEmpty())
            str << strNativeStructLinks;

        if(!strNativeLinks.IsEmpty())
            str << TEXT("\r\n") << strNativeLinks;
        
        str << TEXT("    //</Script>\r\n}\r\n");

        strNativeClassLinks.Clear();
        strNativeStructLinks.Clear();
        strNativeLinks.Clear();
    }

    //----------------------------------

    strFileName.Clear();
    strFileName << TEXT("ScriptOutput/") << curModule << TEXT("/Source/") << curFile;
    if(curSourceFile.Open(strFileName, XFILE_WRITE, XFILE_CREATEALWAYS))
    {
        curSourceFile.Write("\xEF\xBB\xBF", 3);
        CreateSourceComment();
        curSourceFile.WriteAsUTF8(str);
        curSourceFile.Close();
        str.Clear();
    }
    else
    {
        AddError(TEXT("Could not open file '%s'"), strFileName.Array());
        return FALSE;
    }

    return TRUE;
}

