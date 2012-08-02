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
#include "ScriptByteCode.h"


int ENGINEAPI GetStringLine(const TCHAR *lpStart, const TCHAR *lpOffset);


#define AddErrorExpecting(expecting, got)   AddError(TEXT("Expecting '%s', but got '%s'"), (TSTR)expecting, (TSTR)got)
#define AddErrorEndOfCode()                 AddError(TEXT("Unexpected end of code"))
#define AddErrorUnrecoverable()             AddError(TEXT("Compiler terminated due to unrecoverable error"))
#define AddErrorRedefinition(redef)         AddError(TEXT("Redefinition: '%s'"), (TSTR)redef)
#define AddErrorNoClue(item)                AddError(TEXT("Syntax Error: '%s'"), (TSTR)curToken)

#define PeekAtAToken(str) if(!ct.GetNextToken(curToken, TRUE)) {AddError(TEXT("Expecting reprocessor directive")); continue;}
#define HandMeAToken(str) if(!ct.GetNextToken(curToken)) {AddError(TEXT("Expecting reprocessor directive")); continue;}


BOOL Compiler::ParsePreprocessor(String &fileDataOut, TSTR lpIfStart, BOOL bTrue)
{
    String curToken;

    BOOL bCalledElse = FALSE;

    TSTR lastReturn = lpTemp;
    while(GetNextToken(curToken))
    {
        TSTR nextReturn = schr(lpTemp, '\n');
        if(!nextReturn) nextReturn = lpTemp + slen(lpTemp);
        else nextReturn++;

        if(lastReturn != nextReturn && curToken == TEXT("#"))
        {
            TCHAR lastChar = *nextReturn;
            *nextReturn = 0;
            String preprocessorCode = lpTemp;
            preprocessorCode.FindReplace(TEXT("\r"), TEXT(""));
            preprocessorCode.FindReplace(TEXT("\n"), TEXT(""));
            *nextReturn = lastChar;

            int startPos = GetCodePos(lpTemp-1);
            int endPos = GetCodePos(nextReturn);
            dupString.RemoveRange(startPos, endPos);
            dupString.InsertString(startPos, TEXT("\r\n"));
            lpCode = dupString;
            lpTemp = lpCode+startPos;

            CodeTokenizer ct;
            ct.SetCodeStart(preprocessorCode);

            HandMeAToken(curToken);

            if(bTrue && curToken == TEXT("define"))
            {
                HandMeAToken(curToken);

                DefineDefinition *def = Scripting->GetDefineDef(curToken);
                if(def)
                {
                    AddError(TEXT("Define '%s' already exists"), curToken.Array());
                    continue;
                }

                def = Scripting->GlobalDefineList.CreateNew();
                def->name = curToken;
                def->val = ct.lpTemp;
                def->val.KillSpaces();
            }
            else if(bTrue && curToken == TEXT("undef"))
            {
                HandMeAToken(curToken);

                for(int i=0; i<Scripting->GlobalDefineList.Num(); i++)
                {
                    if(Scripting->GlobalDefineList[i].name.Compare(curToken))
                    {
                        Scripting->GlobalDefineList.Remove(i);
                        break;
                    }
                }
            }
            else if(bTrue && curToken == TEXT("ifdef"))
            {
                HandMeAToken(curToken);
                DefineDefinition *def = Scripting->GetDefineDef(curToken);

                if(!ParsePreprocessor(fileDataOut, lpTemp, def != NULL))
                    return FALSE;
                continue;
            }
            else if(bTrue && curToken == TEXT("ifndef"))
            {
                HandMeAToken(curToken);
                DefineDefinition *def = Scripting->GetDefineDef(curToken);

                if(!ParsePreprocessor(fileDataOut, lpTemp, def == NULL))
                    return FALSE;
                continue;
            }
            else if(curToken == TEXT("else") || curToken == TEXT("endif"))
            {
                BOOL bEndif = curToken == TEXT("endif");

                if(!bEndif)
                {
                    if(bCalledElse)
                    {
                        AddError(TEXT("'else' called multiple times inside of if clause"));
                        continue;
                    }
                    bCalledElse = TRUE;
                }

                if(!lpIfStart)
                {
                    AddError(TEXT("'else' called outside of if clause"));
                    continue;
                }

                if(bTrue = !bTrue)
                {
                    while(lpTemp != lpCode && *lpTemp != '\n')
                        --lpTemp;

                    if(*lpTemp == '\n') ++lpTemp;

                    int startCode = GetCodePos(lpIfStart);
                    int endCode = GetCodePos(lpTemp);
                    int numLines = 0;

                    for(int i=startCode; i<endCode; i++)
                    {
                        if(lpCode[i] == '\n')
                            ++numLines;
                    }

                    dupString.RemoveRange(startCode, endCode);
                    while(numLines--)
                    {
                        dupString.InsertString(startCode, TEXT("\r\n"));
                        startCode += 2;
                    }

                    lpCode = dupString;
                    lpTemp = lpCode+startCode;
                    if(!bEndif)
                        continue;
                }
                else
                    lpIfStart = lpTemp;

                if(bEndif)
                    return TRUE;
            }
            else if(bTrue)
                AddError(TEXT("Unknown preprocessor directive"));
        }
        else if(!lpIfStart || bTrue)
        {
            DefineDefinition *def = Scripting->GetDefineDef(curToken);
            if(def)
            {
                int startCode = GetCodePos(lpTemp - curToken.Length());
                int endCode = GetCodePos(lpTemp);
                dupString.RemoveRange(startCode, endCode);
                dupString.InsertString(startCode, def->val);
                lpCode = dupString;
                lpTemp = lpCode+startCode;
            }
        }

        lastReturn = nextReturn;
    }

    if(!lpIfStart)
    {
        fileDataOut = dupString;
        return TRUE;
    }

    return FALSE;
}

#undef PeekAtAToken
#undef HandMeAToken
#define PeekAtAToken(str) if(!GetNextToken(str, TRUE)) {AddErrorEndOfCode(); return FALSE;}
#define HandMeAToken(str) if(!GetNextToken(str)) {AddErrorEndOfCode(); return FALSE;}

BOOL Compiler::CompileObjectScopeFunction(FunctionDefinition &functionDef, CTSTR lpScript, Object *obj, String &errorList)
{
    SetCodeStart(lpScript);
    curObject = obj;
    curClass = obj->GetObjectClass()->GetScriptClass();
    compilerError = &errorList;

    curFile.Clear();
    curFile << functionDef.name;

    String curToken;

    do
    {
        HandMeAToken(curToken);

        if(curToken == TEXT("implemented"))
        {
            AddError(TEXT("Object-scope functions cannot be implemented."));
            continue;
        }
        else if(curToken == TEXT("internal"))
        {
            AddError(TEXT("Object-scope functions cannot be internal."));
            continue;
        }
    }while(false);

    if(!curToken.Compare(functionDef.returnType.name))
    {
        AddErrorExpecting(functionDef.returnType.name, curToken);
        return FALSE;
    }

    HandMeAToken(curToken);

    if(!curToken.Compare(functionDef.name))
    {
        AddErrorExpecting(functionDef.name, curToken);
        return FALSE;
    }

    HandMeAToken(curToken);

    if(curToken[0] != '(')
    {
        AddErrorExpecting(TEXT("("), curToken);
        return FALSE;
    }

    HandMeAToken(curToken);

    for(int i=0; i<functionDef.Params.Num(); i++)
    {
        DefaultVariable &var = functionDef.Params[i];

        if(curToken[0] == ')')
        {
            AddError(TEXT("Invalid number of parameters for the function"));
            return FALSE;
        }

        if(var.flags & VAR_OUT)
        {
            if(!curToken.Compare(TEXT("out")))
            {
                AddErrorExpecting(TEXT("out"), curToken);
                return FALSE;
            }

            HandMeAToken(curToken);
        }

        if(!curToken.Compare(var.typeInfo.name))
        {
            AddError(TEXT("Function parameter %d is supposed to be type '%s'"), i, var.typeInfo.name);
            return FALSE;
        }

        HandMeAToken(curToken);
        HandMeAToken(curToken);

        if((curToken[0] != ',') && (curToken[0] != ')'))
        {
            AddErrorExpecting(TEXT(",' or ')"), curToken);
            return FALSE;
        }
        else if(curToken[0] == ',')
            HandMeAToken(curToken);
    }

    if(curToken[0] != ')')
    {
        AddErrorExpecting(TEXT(")"), curToken);
        return FALSE;
    }

    PeekAtAToken(curToken);

    if(curToken[0] != '{')
    {
        AddErrorExpecting(TEXT("{"), curToken);
        return FALSE;
    }

    return CompileCode(functionDef);
}


void Compiler::InitializeClasses()
{
    Class *curTopClass = Class::LastClass();

    if(errorCount)
        return;

    do
    {
        if(!curTopClass->scriptClass)
            continue;

        if(curTopClass->initializationLevel)
            continue;

        Class *curClass = curTopClass;

        List<Class*> Heirarchy;
        do
        {
            Heirarchy.Insert(0, curClass);
        }while(curClass = curClass->Parent);

        DWORD offset=0;
        DWORD lastSize=0;
        DWORD lastFuncCount=0;

        for(int i=0; i<Heirarchy.Num(); i++)
        {
            curClass = Heirarchy[i];
            ClassDefinition *scriptClass = curClass->scriptClass;

            assert(scriptClass);

            if(!curClass->initializationLevel)
            {
                scriptClass->variableStartIndex = offset;

                if(curClass->bPureScriptClass)
                    curClass->classSize = lastSize;

                for(int j=0; j<scriptClass->Variables.Num(); j++)
                {
                    Variable &var = scriptClass->Variables[j];

                    if(var.typeInfo.type == DataType_Struct)
                    {
                        StructDefinition *structDef = Scripting->GetStruct(var.typeInfo.name, 0);
                        var.typeInfo.size = structDef->size;
                    }
                    else if(var.typeInfo.type == DataType_List)
                    {
                        if(var.subTypeInfo.type == DataType_Struct)
                        {
                            StructDefinition *structDef = Scripting->GetStruct(var.subTypeInfo.name, 0);
                            var.subTypeInfo.size = structDef->size;
                        }
                    }

                    if(curClass->bPureScriptClass)
                    {
                        AlignOffset(curClass->classSize, var.typeInfo.align);
                        var.offset = curClass->classSize;

                        int multiplayer = MAX(var.numElements, 1);
                        curClass->classSize += var.typeInfo.size*multiplayer;
                    }
                }

                for(int j=0; j<scriptClass->Functions.Num(); j++)
                {
                    FunctionDefinition &funcDef = scriptClass->Functions[j];

                    for(int k=0; k<funcDef.Params.Num(); k++)
                    {
                        if(funcDef.Params[k].typeInfo.type == DataType_Struct)
                        {
                            StructDefinition *structDef = Scripting->GetStruct(funcDef.Params[k].typeInfo.name, 0);
                            funcDef.Params[k].typeInfo.size = structDef->size;
                        }
                    }

                    if(funcDef.returnType.type == DataType_Struct)
                    {
                        StructDefinition *structDef = Scripting->GetStruct(funcDef.returnType.name, 0);
                        funcDef.returnType.size = structDef->size;
                    }
                }

                curClass->initializationLevel = 1;

                //align all pure script classes to 16 byte boundries
                if(curClass->bPureScriptClass)
                    AlignOffset(curClass->classSize, 16);
            }

            offset += scriptClass->Variables.Num();
            lastSize = curClass->classSize;
        }
    }while(curTopClass = curTopClass->PrevClass());
}

void Compiler::InitializeClassFunctions()
{
    Class *curTopClass = Class::LastClass();

    if(errorCount)
        return;
    
    do
    {
        if(scmp(curTopClass->GetName(), TEXT("SpaceBackdrop")) == 0)
            nop();

        if(!curTopClass->scriptClass)
            continue;

        if(curTopClass->initializationLevel == 2)
            continue;

        Class *curClass = curTopClass;

        List<Class*> Heirarchy;
        do
        {
            Heirarchy.Insert(0, curClass);
        }while(curClass = curClass->Parent);

        for(int i=0; i<Heirarchy.Num(); i++)
        {
            curClass = Heirarchy[i];
            ClassDefinition *scriptClass = curClass->scriptClass;

            assert(scriptClass);

            if(curClass->initializationLevel == 1)
            {
                for(int j=0; j<scriptClass->Functions.Num(); j++)
                    scriptClass->SetFunctionClassData(scriptClass->Functions[j]);

                curClass->initializationLevel++;
            }
        }
    }while(curTopClass = curTopClass->PrevClass());
}

void Compiler::InitializeStructures()
{
    List<StructDefinition*> UnknownStructs;

    ModuleScriptData *module = Scripting->GetModule(curModule, TRUE);
    for(int i=0; i<module->GlobalStructList.Num(); i++)
    {
        StructDefinition *curStruct = &module->GlobalStructList[i];

        if(!curStruct->size)
            UnknownStructs << curStruct;
    }

    int lastCount;

    while(lastCount = UnknownStructs.Num())
    {
        for(int i=0; i<UnknownStructs.Num(); i++)
        {
            StructDefinition *curStruct = UnknownStructs[i];
            BOOL bFoundUnknownStruct = FALSE;
            unsigned int totalSize = 0, align = curStruct->align;

            if(curStruct->Parent)
            {
                if(!curStruct->Parent->size)
                    continue;

                totalSize = curStruct->Parent->size;
            }

            for(int j=0; j<curStruct->Variables.Num(); j++)
            {
                Variable *var = &curStruct->Variables[j];

                if(var->typeInfo.type == DataType_Struct)
                {
                    if(UnknownStructs.HasValue(var->structDef))
                    {
                        bFoundUnknownStruct = TRUE;
                        break;
                    }
                    else
                    {
                        var->typeInfo.size  = var->structDef->size;
                        var->typeInfo.align = var->structDef->align;

                        int multiplier = MAX(var->numElements, 1);

                        align = MAX(align, var->structDef->align);

                        AlignOffset(totalSize, var->structDef->align);
                        var->offset = totalSize;

                        totalSize += var->structDef->size*multiplier;
                    }
                }
                else
                {
                    int multiplier = MAX(var->numElements, 1);

                    align = MAX(align, var->typeInfo.align);

                    AlignOffset(totalSize, var->typeInfo.align);
                    var->offset = totalSize;

                    totalSize += var->typeInfo.size*multiplier;
                }

                if(bFoundUnknownStruct)
                    break;
            }

            if(!bFoundUnknownStruct)
            {
                for(int j=0; j<curStruct->Functions.Num(); j++)
                {
                    curStruct->Functions[j].funcOffset = j;
                }

                totalSize = MAX(curStruct->pad, totalSize);
                AlignOffset(totalSize, align);

                curStruct->size = totalSize;
                curStruct->align = align;
                UnknownStructs.Remove(i--);
            }
        }

        if(lastCount == UnknownStructs.Num())
        {
            String strErrorData;
            strErrorData << TEXT("Found structures that erroneously reference each other or themselves in a recursive manner: ");

            for(int i=0; i<UnknownStructs.Num(); i++)
            {
                if(!i) strErrorData << TEXT(", ");
                strErrorData << UnknownStructs[i]->name;
            }

            AddError(TEXT("%s"), strErrorData);
            break;
        }
    }

    for(int i=0; i<module->GlobalStructList.Num(); i++)
    {
        StructDefinition *structDef = &module->GlobalStructList[i];
        for(int j=0; j<structDef->Variables.Num(); j++)
        {
            Variable *var = &structDef->Variables[j];

            if(var->typeInfo.type == DataType_List && var->subTypeInfo.type == DataType_Struct)
            {
                StructDefinition *subStruct;
                for(int i=0; i<module->GlobalStructList.Num(); i++)
                {
                    if(var->subTypeInfo.name == (CTSTR)module->GlobalStructList[i].name)
                    {
                        subStruct = &module->GlobalStructList[i];
                        break;
                    }
                }

                var->subTypeInfo.size = subStruct->size;
            }
        }

        for(int j=0; j<structDef->Functions.Num(); j++)
        {
            FunctionDefinition &funcDef = structDef->Functions[j];

            for(int k=0; k<funcDef.Params.Num(); k++)
            {
                if(funcDef.Params[k].typeInfo.type == DataType_Struct)
                {
                    StructDefinition *structDef = Scripting->GetStruct(funcDef.Params[k].typeInfo.name, 0);
                    funcDef.Params[k].typeInfo.size = structDef->size;
                }
            }

            if(funcDef.returnType.type == DataType_Struct)
            {
                StructDefinition *structDef = Scripting->GetStruct(funcDef.returnType.name, 0);
                funcDef.returnType.size = structDef->size;
            }
        }
    }

    for(int i=0; i<module->GlobalFunctionList.Num(); i++)
    {
        FunctionDefinition &funcDef = module->GlobalFunctionList[i];

        for(int j=0; j<funcDef.Params.Num(); j++)
        {
            if(funcDef.Params[j].typeInfo.type == DataType_Struct)
            {
                StructDefinition *structDef = Scripting->GetStruct(funcDef.Params[j].typeInfo.name);
                funcDef.Params[j].typeInfo.size = structDef->size;
            }
        }

        if(funcDef.returnType.type == DataType_Struct)
        {
            StructDefinition *structDef = Scripting->GetStruct(funcDef.returnType.name);
            funcDef.returnType.size = structDef->size;
        }
    }

    for(int i=0; i<module->GlobalVariableList.Num(); i++)
    {
        Variable &var = module->GlobalVariableList[i];

        if(var.typeInfo.type == DataType_Struct)
        {
            StructDefinition *structDef = Scripting->GetStruct(var.typeInfo.name);
            var.typeInfo.size = structDef->size;
        }
    }
}


BOOL Compiler::CompileStage(CTSTR lpCurModule, int stage, String &errorList, CTSTR lpFile)
{
    switch(stage)
    {
        case 0:
            compileStage = TEXT("Name preprocessor "); curStage = 0; break;
        case 1:
            compileStage = TEXT("Data preprocessor "); curStage = 1; break;
        case 2:
            compileStage = TEXT("Compiler "); curStage = 2; break;
        case 3:
            compileStage = TEXT("Header Generation "); curStage = 3; break;
    }

    if(!curClass && !curStruct)
    {
        curModule = lpCurModule;
        
        curFile = GetPathFileName(lpFile, TRUE);

        compilerError = &errorList;

        if(bGeneratingBytecodeText)
            lpLastTextOffset = lpTemp;
    }

    ModuleScriptData *module = Scripting->GetModule(curModule, TRUE);

    List<FunctionDefinition> &funcs = (curClass) ? curClass->Functions : ((curStruct) ? curStruct->Functions : module->GlobalFunctionList);

    String curToken;

    BOOL bSetInternal = FALSE;
    BOOL bSetAbstract = FALSE;
    BOOL bSetImplementable = FALSE;
    BOOL bSetProperty = FALSE;
    BOOL bSetStatic = FALSE;

    BOOL bInternal;
    BOOL bAbstract;
    BOOL bImplementable;
    BOOL bStatic;

    TSTR lpStartPos;

    while(GetNextToken(curToken))
    {
        if(bGeneratingBytecodeText)
            lpStartPos = lpTemp-curToken.Length();

        if(bSetInternal)
        {
            bInternal = TRUE;
            bSetInternal = FALSE;
        }
        else if(bSetAbstract)
        {
            bAbstract = TRUE;
            bSetAbstract = FALSE;
        }
        else if(bSetImplementable)
        {
            bImplementable = TRUE;
            bSetImplementable = FALSE;
        }
        else if(bSetStatic)
        {
            bStatic = TRUE;
            bSetStatic = FALSE;
        }
        else
            bInternal = bAbstract = bImplementable = bStatic = FALSE;

        if((curClass || curStruct) && (curToken[0] == '}'))
            return TRUE;
        else if(curToken == TEXT("internal") && !bInternal)
            bSetInternal = TRUE;
        else if(curToken == TEXT("static") && !bStatic)
            bSetStatic = TRUE;
        else if(curToken == TEXT("abstract") & !bAbstract)
            bSetAbstract = TRUE;
        else if(curToken == TEXT("implementable") & !bAbstract)
            bSetImplementable = TRUE;
        else if(curToken == TEXT("struct"))
        {
            if(curClass || curStruct)
            {
                if(stage == 1)
                    AddError(TEXT("Cannot define structs/classes/enums inside a class"));
                if(!GotoToken(TEXT("{")))  {AddErrorEndOfCode(); return FALSE;}
                continue;
            }

            StructDefinition *structDef = NULL;

            HandMeAToken(curToken);

            int align = 4, pad = 0;

            if(curToken == TEXT("align128"))
            {
                #ifdef USE_SSE
                align = 16;
                #endif

                pad = 16;
                HandMeAToken(curToken);
            }

            if(stage == 0)
            {
                if(bInternal)
                    AddError(TEXT("structs cannot be internal."));

                if(bAbstract)
                    AddError(TEXT("structs cannot be abstract."));

                if(bImplementable)
                    AddError(TEXT("Only class member functions can be implementable"));

                if(bStatic)
                    AddError(TEXT("Only class/struct member functions can be static"));

                structDef = module->GlobalStructList.CreateNew();
                structDef->pad = pad;
                structDef->align = align;

                if(!iswalpha(curToken[0]))
                {
                    AddError(TEXT("Names must begin with valid characters: '%s'"), (TSTR)curToken);
                    //AddStartCharacter(curToken);
                }

                if(NameDefined(curClass, curStruct, curToken))
                {
                    AddErrorRedefinition(curToken);
                    //AddRandomNumber(curToken);
                }

                structDef->name = curToken;
            }
            else if(stage >= 1)
                structDef = Scripting->GetStruct(curToken, curModule);

            if(!structDef)
            {
                if(!GotoToken(TEXT("{")))  {AddErrorEndOfCode(); return FALSE;}
                continue;
            }

            if(stage == 3)
                CreateSourceStruct(structDef);

            //---------------------------------

            HandMeAToken(curToken);

            if(curToken[0] == ':')
            {
                HandMeAToken(curToken);

                if(stage == 1)
                {
                    structDef->Parent = Scripting->GetStruct(curToken);
                    if(!structDef->Parent)
                    {
                        AddError(TEXT("Could not find parent struct '%s'"), curToken.Array());
                        if(!GotoToken(TEXT("{")))  {AddErrorEndOfCode(); return FALSE;}
                        continue;
                    }
                }

                HandMeAToken(curToken);
            }

            if(curToken[0] != '{')
            {
                if(stage == 0)
                    AddErrorExpecting(TEXT("{"), curToken);
                continue;
            }

            if(stage == 0)
            {
                if(!PassBracers(lpTemp-1))  {AddErrorEndOfCode(); return FALSE;}

                HandMeAToken(curToken);

                continue;
            }
            else
            {
                curStruct = structDef;
                CompileStage(lpCurModule, stage, errorList, lpFile);  //brilliance in a basket
                curStruct = NULL;
            }
        }
        else if(curToken == TEXT("class"))
        {
            if(curClass || curStruct)
            {
                if(stage == 1)
                    AddError(TEXT("Cannot define structs/classes/enums inside a class"));
                if(!GotoToken(TEXT("{")))  {AddErrorEndOfCode(); return FALSE;}
                continue;
            }

            String strName;
            HandMeAToken(strName);

            Class *classInfo=NULL;
            ClassDefinition *classDef=NULL;

            if(stage == 0)
            {
                if(!iswalpha(strName[0]))
                {
                    AddError(TEXT("Names must begin with valid characters: '%s'"), (TSTR)strName);
                    //AddStartCharacter(strName);
                }

                classInfo = FindClass(strName);

                if(bImplementable)
                    AddError(TEXT("Only class member functions can be implementable"));

                if(bStatic)
                    AddError(TEXT("Only class/struct member functions can be static"));

                if(!classInfo)
                {
                    if(NameDefined(NULL, NULL, strName))
                    {
                        AddErrorRedefinition(strName);
                        //AddRandomNumber(strName);
                    }

                    classInfo = new Class(sdup(strName), NULL, bAbstract, NULL, 0);
                    classInfo->bFreeManually = TRUE;
                }

                scpy_n(classInfo->module, curModule, 254);

                classDef = classInfo->scriptClass = new ClassDefinition;
                classDef->classData = classInfo;
                classInfo->bPureScriptClass = !bInternal;
                classInfo->isAbstract = bAbstract;
            }
            else
            {
                classInfo = FindClass(strName, curModule);
                if(!classInfo)
                {
                    if(!GotoToken(TEXT("{")))  {AddErrorEndOfCode(); return FALSE;}
                    continue;
                }

                classDef = classInfo->scriptClass;
                if(!classInfo->bPureScriptClass && !bInternal)
                    AddWarning(TEXT("internal class found, but not marked as internal in script"));

                if(classInfo->bPureScriptClass)
                {
                    Class *curClass = classInfo;
                    while(curClass = curClass->Parent)
                    {
                        if(curClass->pCreate != NULL)
                        {
                            classInfo->pCreate = curClass->pCreate;
                            break;
                        }
                    }
                }
            }

            if(stage == 3 && classDef)
                CreateSourceClass(classDef);

            HandMeAToken(curToken);

            if(curToken[0] == ':')
            {
                HandMeAToken(curToken);

                if(stage == 1)
                {
                    if(classInfo->bFreeManually)
                        classInfo->Parent = FindClass(curToken);
                    if(!classInfo->Parent)
                        AddError(TEXT("Could not find parent class '%s'"), curToken.Array());
                    else
                        classInfo->scriptClass->Parent = classInfo->Parent->scriptClass;

                    if(!classInfo->Parent)
                    {
                        if(!GotoToken(TEXT("{")))  {AddErrorEndOfCode(); return FALSE;}
                        continue;
                    }
                }

                HandMeAToken(curToken);
            }
            else if(stage == 2)
            {
                if(strName != TEXT("Object"))
                    AddError(TEXT("Only the Object class can be the base class"));
            }

            //---------------------------------

            if(curToken[0] != '{')
            {
                AddErrorExpecting(TEXT("{"), curToken);
                continue;
            }

            //---------------------------------

            if(stage == 0)
            {
                if(!PassBracers(lpTemp-1)) {AddErrorEndOfCode(); return FALSE;}
            }
            else
            {
                curClass = classDef;
                CompileStage(lpCurModule, stage, errorList, lpFile);  //brilliance in a basket
                curClass = NULL;
            }

            //---------------------------------

            HandMeAToken(curToken);

            if(stage == 1 && curToken[0] != ';')
                AddErrorExpecting(TEXT(";"), curToken);
        }
        else if(curToken == TEXT("enum"))
        {
            if(curClass || curStruct)
            {
                if(stage == 1)
                    AddError(TEXT("Cannot define structs/classes/enums inside a class"));
                if(!GotoToken(TEXT("{")))  {AddErrorEndOfCode(); return FALSE;}
                continue;
            }

            if(stage == 0)
            {
                if(bInternal)
                    AddError(TEXT("enums cannot be internal."));

                if(bAbstract)
                    AddError(TEXT("enums cannot be abstract."));

                if(bImplementable)
                    AddError(TEXT("Only class member functions can be implementable"));

                if(bStatic)
                    AddError(TEXT("Only class/struct member functions can be static"));
            }

            HandMeAToken(curToken);

            EnumDefinition *enumDef = NULL;
            
            if(stage == 0)
                enumDef = module->GlobalEnumList.CreateNew();
            else if(stage == 3)
            {
                if(enumCount < module->GlobalEnumList.Num())
                {
                    enumDef = &module->GlobalEnumList[enumCount];
                    CreateSourceEnum(enumDef);
                    ++enumCount;
                }
            }

            if(curToken[0] != '{')
            {
                if(stage == 0)
                {
                    if(!iswalpha(curToken[0]))
                    {
                        AddError(TEXT("Names must begin with valid characters: '%s'"), (TSTR)curToken);
                        //AddStartCharacter(curToken);
                    }

                    if(NameDefined(curClass, curStruct, curToken))
                    {
                        AddErrorRedefinition(curToken);
                        //AddRandomNumber(curToken);
                    }

                    enumDef->name = curToken;
                }

                HandMeAToken(curToken);
            }

            if(curToken[0] != '{')
            {
                if(stage == 0)
                    AddErrorExpecting(TEXT("{"), curToken);
                continue;
            }

            //---------------------------------

            if(stage != 0)
            {
                if(!PassBracers(lpTemp-1))  {AddErrorEndOfCode(); return FALSE;}
            }
            else
            {
                HandMeAToken(curToken);

                int curPos = 0;

                while(curToken[0] != '}')
                {
                    if(curToken[0] == ',')
                    {
                        AddErrorNoClue(curToken);
                        continue;
                    }

                    EnumItem *item = enumDef->Items.CreateNew();

                    if(!iswalpha(curToken[0]))
                    {
                        AddError(TEXT("Names must begin with valid characters: '%s'"), (TSTR)curToken);
                        //AddStartCharacter(curToken);
                    }

                    if(NameDefined(curClass, curStruct, curToken))
                    {
                        AddErrorRedefinition(curToken);
                        //AddRandomNumber(curToken);
                    }

                    item->name = curToken;

                    HandMeAToken(curToken);

                    if(curToken == TEXT("="))
                    {
                        BOOL bFloatOccurance = FALSE;
                        if(!GetNextTokenEval(curToken, &bFloatOccurance)) {AddErrorEndOfCode(); return FALSE;}

                        TokenType type = GetTokenType(curClass, curStruct, curToken);

                        if(type == TokenType_Number && !bFloatOccurance)
                            curPos = tstoi(curToken);
                        else if(type == TokenType_Enum)
                            curPos = GetEnumVal(curToken);
                        else
                            AddErrorExpecting(TEXT("integer or enum value"), curToken);

                        HandMeAToken(curToken);
                    }

                    item->val = curPos;

                    if(curToken[0] == '}')
                        break;

                    if(curToken[0] != ',')
                        AddErrorExpecting(TEXT(",' or '}"), curToken);

                    HandMeAToken(curToken);

                    ++curPos;
                }
            }

            //---------------------------------

            HandMeAToken(curToken);

            if(stage == 1 && curToken[0] != ';')
                AddErrorExpecting(TEXT(";"), curToken);
        }
        else if(curToken[0] == ';')
            continue;
        else if(curToken[0] == '{')
        {
            if(!PassBracers(lpTemp-1))  {AddErrorEndOfCode(); return FALSE;}
        }
        else //function or variable
        {
            BOOL bProperty = FALSE;

            if(curToken == TEXT("property"))
            {
                if(curClass)
                    bProperty = TRUE;
                else if(stage == 1)
                    AddError(TEXT("Property variables can only be used in classes"));

                HandMeAToken(curToken);
            }

            String typeName = curToken;
            BOOL bList = FALSE;
            String subType;

            if(typeName == TEXT("list"))
            {
                bList = TRUE;

                HandMeAToken(curToken);
                if(curToken != TEXT("<"))
                {
                    AddErrorExpecting(TEXT("<"), curToken);
                    if(!GotoClosestToken(TEXT(";"), TEXT("{"))) {AddErrorEndOfCode(); return FALSE;}
                    continue;
                }

                HandMeAToken(curToken);

                if(subType != TEXT(">"))
                {
                    subType = curToken;

                    HandMeAToken(curToken);
                    if(curToken != TEXT(">"))
                    {
                        AddErrorExpecting(TEXT(">"), curToken);
                        if(!GotoClosestToken(TEXT(";"), TEXT("{"))) {AddErrorEndOfCode(); return FALSE;}
                        continue;
                    }
                }
                else
                    AddError(TEXT("A type must be specified for a list variable"));

                curToken = typeName;
            }

            TokenType type = GetTokenType(curClass, curStruct, curToken);

            if((type != TokenType_Type) && (type != TokenType_Struct) && (type != TokenType_Class) && (type != TokenType_EnumDef))
            {
                if(stage == 1)
                    AddErrorNoClue(curToken);

                if(!GotoClosestToken(TEXT(";"), TEXT("{"))) {AddErrorEndOfCode(); return FALSE;}
                continue;
            }

            if(stage == 1 && bAbstract)
                AddError(TEXT("Variables and functions cannot be abstract."));

            //--------------------------------

            FunctionDefinition *funcDef = NULL;
            String funcName, nextToken;
            BOOL bOperator = FALSE;

            PeekAtAToken(nextToken);

            BOOL bConstructor = FALSE;
            if((curStruct || curClass) && nextToken[0] == '(')
            {
                if((curStruct && typeName == curStruct->name) || (curClass && typeName == curClass->classData->name))
                {
                    if(curClass && scmp(curClass->classData->GetName(), TEXT("SpaceBackdrop")) == 0)
                        nop();
                    bConstructor = TRUE;
                }
            }

            if(!bConstructor)
            {
                HandMeAToken(curToken);

                bOperator = (curToken == TEXT("operator"));

                if(bOperator && curClass)
                {
                    if(stage == 1)
                        AddError(TEXT("operators cannot be class member functions"));
                    if(!GotoToken(TEXT(")"), TRUE))   {AddErrorEndOfCode(); return FALSE;}
                    continue;
                }

                PeekAtAToken(nextToken);
            }

            if(!bOperator && (nextToken[0] != '(')) //assume variable
            {
                if(stage == 1)
                {
                    if(curStruct && typeName == curStruct->name)
                        AddError(TEXT("Cannot use a structure variable within the stucture of the same type"));
                    if(bInternal)
                        AddError(TEXT("Variables cannot be internal."));
                    if(bImplementable)
                        AddError(TEXT("Only class member functions can be implementable"));
                    if(NameDefined(curClass, curStruct, curToken))
                    {
                        AddErrorRedefinition(curToken);
                        //AddRandomNumber(curToken);
                    }
                }

                while(true) //retarded but works
                {
                    int curPos = module->GlobalVariableList.Num();

                    Variable *var;
                    
                    if(stage == 1)
                    {
                        if(curClass)
                            var = curClass->Variables.CreateNew();
                        else if(curStruct)
                            var = curStruct->Variables.CreateNew();
                        else
                            var = module->GlobalVariableList.CreateNew();

                        if(bProperty)
                            static_cast<DefaultVariable*>(var)->flags |= VAR_PROPERTY;

                        var->name = curToken;
                        var->numElements = 0;

                        if(curClass)
                            var->scope = VarScope_Class;
                        else if(curStruct)
                            var->scope = VarScope_Struct;
                        else
                            var->scope = VarScope_Global;

                        if(bList)
                        {
                            if(!GetTypeInfo(subType, var->subTypeInfo))
                                AddError(TEXT("Unknown data type: '%s'"), subType.Array());
                            else if(var->subTypeInfo.type == DataType_Void)
                                AddError(TEXT("Cannot use void as a data type for variables"));
                        }

                        var->offset = (curClass || curStruct) ? 0 : MAKEDWORD(curPos, module->moduleHash);
                        if(!GetTypeInfo(typeName, var->typeInfo))
                            AddError(TEXT("Unknown data type: '%s'"), typeName.Array());
                        else if(var->typeInfo.type == DataType_Void)
                            AddError(TEXT("Cannot use void as a data type for variables"));

                        if(var->typeInfo.type == DataType_Struct)
                            var->structDef = Scripting->GetStruct(var->typeInfo.name);
                    }

                    HandMeAToken(curToken);

                    if(curToken[0] == '[')
                    {
                        HandMeAToken(curToken);

                        if(stage == 1)
                        {
                            if((curToken[0] != ']') && !ValidIntString(curToken))
                                AddError(TEXT("Invalid array count: %s"), curToken.Array());
                        }

                        if(curToken[0] == ']')
                        {
                            if(stage == 1)
                                AddError(TEXT("Must specify an integer count of 1 or higher"));
                        }
                        else
                        {
                            if(stage == 1)
                            {
                                var->numElements = tstoi(curToken);

                                if(var->numElements == 0)
                                    AddError(TEXT("Must specify an integer count of 1 or higher"));
                            }

                            HandMeAToken(curToken);

                            if(curToken[0] != ']')
                            {
                                if(stage == 1)
                                    AddErrorExpecting(TEXT("]"), curToken);

                                if(!GotoToken(TEXT("]"), TRUE))   {AddErrorEndOfCode(); return FALSE;}
                            }
                        }

                        HandMeAToken(curToken);
                    }

                    PropertyVariable *propVar;

                    if(curClass && stage == 1)
                    {
                        propVar = (PropertyVariable*)var;

                        if(propVar->typeInfo.type == DataType_Float)
                        {
                            propVar->propertyType = PROPERTY_SCROLLER;
                            propVar->fMin = -1e9f;
                            propVar->fMax = 1e9f;
                            propVar->fInc = 0.01f;
                        }
                        else if(propVar->typeInfo.type == DataType_Integer)
                        {
                            propVar->propertyType = PROPERTY_SCROLLER;
                            propVar->iMin = -0x7FFFFFFF;
                            propVar->iMax = 0x7FFFFFFF;
                            propVar->iInc = 1;
                        }
                    }

                    if((curStruct || curClass) && curToken == TEXT("="))
                    {
                        if(stage == 1)
                            AddError(TEXT("Structure and class variables cannot have default values (use a constructor)"));
                        if(!GotoToken(TEXT(";"), TRUE)) {AddErrorEndOfCode(); return FALSE;}
                        curToken = TEXT(";");
                    }

                    if(!curStruct && !curClass && curToken == TEXT("="))
                    {
                        if(stage == 1 && var->numElements)
                            AddError(TEXT("Default variables cannot be assigned to arrays"));

                        HandMeAToken(curToken);

                        if(curToken == TEXT("-"))
                        {
                            HandMeAToken(nextToken);
                            curToken << nextToken;
                        }

                        if(stage == 1)
                        {
                            DefaultVariable *defVar = (DefaultVariable*)var;

                            BufferOutputSerializer defOut(defVar->DefaultParamData);

                            defVar->flags |= VAR_DEFAULT;

                            type = GetTokenType(NULL, NULL, curToken);

                            if(type == TokenType_Number)
                            {
                                if(GetBaseFloat(curToken))
                                {
                                    float fChi = (float)tstof(curToken);

                                    if(var->typeInfo.type == DataType_Float)
                                        defOut << fChi;
                                    else if(var->typeInfo.type == DataType_Integer)
                                    {
                                        int iChi = (int)fChi;
                                        defOut << iChi;
                                        //AddWarning(TEXT("Automatically converting float value to integer: %d"), iChi);
                                    }
                                    else
                                        AddErrorNoClue(curToken);
                                }
                                else
                                {
                                    int iChi = tstoi(curToken);

                                    if(var->typeInfo.type == DataType_Float)
                                    {
                                        float fChi = (float)iChi;
                                        defOut << fChi;
                                        //AddWarning(TEXT("Automatically converting int value to float"), fChi);
                                    }
                                    else if(var->typeInfo.type == DataType_Integer)
                                        defOut << iChi;
                                    else
                                        AddErrorNoClue(curToken);
                                }
                            }
                            else if(var->typeInfo.type == DataType_String)
                            {
                                if(type == TokenType_String)
                                    defOut << GetActualString(curToken);
                                else if(curToken == TEXT("null"))
                                    defOut << String();
                                else
                                    AddErrorNoClue(curToken);
                            }
                            else if(var->typeInfo.type == DataType_Object || var->typeInfo.type == DataType_Handle)
                            {
                                do
                                {
                                    if(curToken == TEXT("null"))
                                    {
                                        Object *bla = NULL;
                                        defOut.Serialize(&bla, sizeof(Object*));
                                    }
                                    else
                                        AddError(TEXT("Unknown default value"));

                                }while(false);

                            }
                            else if(type == TokenType_Enum)
                            {
                                int val = GetEnumVal(curToken);

                                if(var->typeInfo.type == DataType_Integer)
                                    defOut << val;
                                else
                                    AddErrorNoClue(curToken);
                            }
                            else
                                AddErrorNoClue(curToken);
                        }

                        HandMeAToken(curToken);
                    }
                    else if(stage == 1 && var->scope == VarScope_Global)
                        static_cast<DefaultVariable*>(var)->DefaultParamData.SetSize(var->typeInfo.size);

                    while(curToken[0] == '<')
                    {
                        if(!bProperty)
                            AddError(TEXT("Cannot use property descriptors on a non-property variable"));

                        if(!curClass)
                        {
                            AddError(TEXT("Only class variables may use property descriptors"));
                            if(!GotoToken(TEXT(">"), TRUE)) {AddErrorEndOfCode(); return FALSE;}
                            HandMeAToken(curToken);
                            continue;
                        }

                        if(curClass && bProperty && stage == 1 && engine->InEditor())
                        {
                            String strPropertyType;
                            HandMeAToken(strPropertyType);
                            HandMeAToken(curToken);

                            if(curToken != TEXT("="))
                            {
                                AddErrorExpecting(TEXT("="), curToken);
                                break;
                            }

                            do
                            {
                                if(strPropertyType.CompareI(TEXT("type")))
                                {
                                    HandMeAToken(curToken);

                                    BOOL bNegate = FALSE;

                                    if(curToken.CompareI(TEXT("scroller")))
                                    {
                                        if(var->typeInfo.type == DataType_Float)
                                        {
                                            HandMeAToken(curToken);
                                            if(curToken[0] != '(') {AddErrorExpecting(TEXT("("), curToken); break;}

                                            HandMeAToken(curToken);
                                            if(curToken == TEXT("-"))
                                            {
                                                bNegate = TRUE;
                                                HandMeAToken(curToken);
                                            }

                                            if(!ValidFloatString(curToken)) {AddErrorExpecting(TEXT("float"), curToken); break;}
                                            propVar->fMin = tstof(curToken);
                                            if(bNegate) propVar->fMin = -propVar->fMin;
                                            bNegate = FALSE;

                                            //-------------------------------

                                            HandMeAToken(curToken);
                                            if(curToken[0] != ',') {AddErrorExpecting(TEXT(","), curToken); break;}

                                            HandMeAToken(curToken);
                                            if(curToken == TEXT("-"))
                                            {
                                                bNegate = TRUE;
                                                HandMeAToken(curToken);
                                            }

                                            if(!ValidFloatString(curToken)) {AddErrorExpecting(TEXT("float"), curToken); break;}
                                            propVar->fMax = tstof(curToken);
                                            if(bNegate) propVar->fMax = -propVar->fMax;
                                            bNegate = FALSE;

                                            //-------------------------------

                                            HandMeAToken(curToken);
                                            if(curToken[0] != ',') {AddErrorExpecting(TEXT(","), curToken); break;}

                                            HandMeAToken(curToken);
                                            if(curToken == TEXT("-"))
                                            {
                                                bNegate = TRUE;
                                                HandMeAToken(curToken);
                                            }

                                            if(!ValidFloatString(curToken)) {AddErrorExpecting(TEXT("float"), curToken); break;}
                                            propVar->fInc = tstof(curToken);
                                            if(bNegate) propVar->fInc = -propVar->fInc;

                                            //-------------------------------

                                            HandMeAToken(curToken);
                                            if(curToken[0] != ')') {AddErrorExpecting(TEXT(")"), curToken); break;}

                                            propVar->propertyType = PROPERTY_SCROLLER;
                                        }
                                        else if((var->typeInfo.type == DataType_Integer)     &&
                                                (scmp(var->typeInfo.name, TEXT("int")) == 0) )
                                        {
                                            HandMeAToken(curToken);
                                            if(curToken[0] != '(') {AddErrorExpecting(TEXT("("), curToken); break;}

                                            HandMeAToken(curToken);
                                            if(curToken == TEXT("-"))
                                            {
                                                bNegate = TRUE;
                                                HandMeAToken(curToken);
                                            }

                                            if(!ValidIntString(curToken)) {AddErrorExpecting(TEXT("float"), curToken); break;}
                                            propVar->iMin = tstoi(curToken);
                                            if(bNegate) propVar->iMin = -propVar->iMin;
                                            bNegate = FALSE;

                                            //-------------------------------

                                            HandMeAToken(curToken);
                                            if(curToken[0] != ',') {AddErrorExpecting(TEXT(","), curToken); break;}

                                            HandMeAToken(curToken);
                                            if(curToken == TEXT("-"))
                                            {
                                                bNegate = TRUE;
                                                HandMeAToken(curToken);
                                            }

                                            if(!ValidIntString(curToken)) {AddErrorExpecting(TEXT("float"), curToken); break;}
                                            propVar->iMax = tstoi(curToken);
                                            if(bNegate) propVar->iMax = -propVar->iMax;
                                            bNegate = FALSE;

                                            //-------------------------------

                                            HandMeAToken(curToken);
                                            if(curToken[0] != ',') {AddErrorExpecting(TEXT(","), curToken); break;}

                                            HandMeAToken(curToken);
                                            if(curToken == TEXT("-"))
                                            {
                                                bNegate = TRUE;
                                                HandMeAToken(curToken);
                                            }

                                            if(!ValidIntString(curToken)) {AddErrorExpecting(TEXT("float"), curToken); break;}
                                            propVar->iInc = tstoi(curToken);
                                            if(bNegate) propVar->iInc = -propVar->iInc;

                                            //-------------------------------

                                            HandMeAToken(curToken);
                                            if(curToken[0] != ')') {AddErrorExpecting(TEXT(")"), curToken); break;}

                                            propVar->propertyType = PROPERTY_SCROLLER;
                                        }
                                        else
                                            {AddError(TEXT("Scroller property type can only be used with floats and integers")); break;}
                                    }
                                    else if(curToken.CompareI(TEXT("texture")))
                                        propVar->propertyType = PROPERTY_TEXTURE;
                                    else if(curToken.CompareI(TEXT("sound")))
                                        propVar->propertyType = PROPERTY_SOUND;
                                    else if(curToken.CompareI(TEXT("music")))
                                        propVar->propertyType = PROPERTY_MUSIC;
                                    else if(curToken.CompareI(TEXT("material")))
                                        propVar->propertyType = PROPERTY_MATERIAL;
                                    else if(curToken.CompareI(TEXT("texture2D")))
                                        propVar->propertyType = PROPERTY_TEXTURE2D;
                                    else if(curToken.CompareI(TEXT("sound2D")))
                                        propVar->propertyType = PROPERTY_SOUND2D;
                                    else
                                        {AddErrorNoClue(curToken); break;}
                                }
                                else if(strPropertyType.CompareI(TEXT("section"))  ||
                                        strPropertyType.CompareI(TEXT("catagory")) )
                                {
                                    HandMeAToken(curToken);
                                    if(curToken[0] != '"') {AddErrorExpecting(TEXT("string"), curToken); break;}

                                    propVar->section = GetActualString(curToken);
                                }
                                else if(strPropertyType.CompareI(TEXT("description")))
                                {
                                    HandMeAToken(curToken);
                                    if(curToken[0] != '"') {AddErrorExpecting(TEXT("string"), curToken); break;}

                                    propVar->description = GetActualString(curToken);
                                }
                            }while(false); //one the slimiest c/c++ hacks I've *ever* done to get cleaner code.
                        }

                        if(!GotoToken(TEXT(">"), TRUE))
                        {
                            AddErrorEndOfCode();
                            return FALSE;
                        }

                        HandMeAToken(curToken);
                    }

                    if(curToken[0] != ',')
                    {
                        if(stage == 1 && curToken[0] != ';')
                            AddErrorExpecting(TEXT(";"), curToken);
                        break;
                    }

                    HandMeAToken(curToken);
                }
            }
            else //assume function
            {
                if(bList)
                    AddError(TEXT("Functions cannot return lists.  Use an output parameter instead."));

                if(bProperty)
                    AddError(TEXT("Functions cannot be marked as property variables"));

                if(stage == 1)
                {
                    if(bStatic)
                    {
                        if(bOperator)
                            AddError(TEXT("Operators cannot be static"));

                        if(bConstructor)
                            AddError(TEXT("Constructors cannot be static"));

                        if(!curClass && !curStruct)
                        {
                            AddWarning(TEXT("Global functions do not need to be marked as static; ignored"));
                            bStatic = FALSE;
                        }
                    }

                    if(curClass && curClass->classData->bPureScriptClass && bInternal)
                        AddError(TEXT("Functions cannot be declared as internal inside a non-internal class"));


                    if(bImplementable)
                    {
                        if(!curClass)
                            AddError(TEXT("Only class member functions can be implementable"));

                        if(bConstructor)
                            AddError(TEXT("Constructors cannot be implementable"));

                        if(bOperator)
                            AddError(TEXT("Operators cannot be implementable"));

                        if(bInternal)
                            AddError(TEXT("A function cannot be both internal and implementable.  However, derived versions of those functions can be internal."));
                    }

                    if(curClass && bOperator)
                        AddError(TEXT("Cannot use operators inside classes"));
                }

                if(bOperator)
                {
                    HandMeAToken(curToken);
                }

                if(!bConstructor)
                    funcName = curToken;

                BOOL bAssignmentOperator = (funcName == TEXT("="));
                TypeInfo returnType;

                if(stage > 0)
                {
                    if(!bConstructor)
                    {
                        if(!GetTypeInfo(typeName, returnType) && stage == 1)
                            AddError(TEXT("Unknown data type: '%s'"), typeName.Array());
                    }
                    else
                        GetTypeInfo(TEXT("void"), returnType);
                }

                FunctionDefinition tempFunc;
                zero(&tempFunc, sizeof(FunctionDefinition));

                if(stage == 1)
                {
                    if(bOperator)
                    {
                        TokenType opType = GetTokenType(curClass, curStruct, curToken);
                        if(!ValidOperator(curToken))
                            AddError(TEXT("Invalid operator type '%s' for operator function"), (TSTR)curToken);
                    }

                    funcDef = &tempFunc;

                    if(!bConstructor)
                    {
                        if(!bOperator && !iswalpha(curToken[0]))
                        {
                            AddError(TEXT("Names must begin with valid characters: '%s'"), (TSTR)curToken);
                            //AddStartCharacter(curToken);
                        }

                        if(!bOperator && NameDefined(curClass, curStruct, curToken, TRUE))
                        {
                            AddErrorRedefinition(curToken);
                            //AddRandomNumber(curToken);
                        }

                        funcDef->name = curToken;
                    }

                    funcDef->returnType = returnType;

                    if(curClass)
                        funcDef->classContext  = curClass;
                    else if(curStruct)
                        funcDef->structContext = curStruct;
                }

                //--------------------------------

                HandMeAToken(curToken);

                if(curToken[0] != '(')
                {
                    if(stage == 1)
                        AddErrorExpecting(TEXT("("), curToken);
                    if(!GotoToken(TEXT("("), TRUE))
                         {AddErrorEndOfCode(); return FALSE;}
                }

                HandMeAToken(curToken);

                int id = 0;

                List<TypeInfo> paramTypes;
                List<TypeInfo> subParamTypes;

                BOOL bAllDefaults = TRUE;

                while(curToken[0] != ')')
                {
                    BOOL bOutVar = FALSE;

                    if(curToken == TEXT("out"))
                    {
                        bOutVar = TRUE;
                        HandMeAToken(curToken);
                    }

                    DefaultVariable *var;
                    if(stage == 1)
                    {
                        var = funcDef->Params.CreateNew();
                        if(!GetTypeInfo(curToken, var->typeInfo))
                            AddError(TEXT("Unknown data type: '%s'"), curToken.Array());
                        else if(var->typeInfo.type == DataType_Void)
                            AddError(TEXT("Cannot use void as a data type for variables"));

                        if(var->typeInfo.type == DataType_Struct)
                            var->structDef = Scripting->GetStruct(var->typeInfo.name);
                    }

                    String varTypeName = curToken, subType;
                    if(curToken == TEXT("list"))
                    {
                        HandMeAToken(curToken);
                        if(curToken == TEXT("<"))
                        {
                            HandMeAToken(subType);
                            if(curToken != TEXT(">"))
                            {
                                if(stage == 1)
                                {
                                    if(!GetTypeInfo(subType, var->subTypeInfo))
                                        AddError(TEXT("Unknown data type: '%s'"), subType.Array());
                                    else if(var->typeInfo.type == DataType_Void)
                                        AddError(TEXT("Cannot use void as a data type for variables"));
                                }

                                HandMeAToken(curToken);
                                if(curToken != TEXT(">"))
                                {
                                    if(stage == 1)
                                        AddErrorExpecting(TEXT(">"), curToken);
                                    if(!GotoToken(TEXT(">"), TRUE)) {AddErrorEndOfCode(); return FALSE;}
                                }
                            }
                            else if(stage == 1)
                                AddError(TEXT("A type must be specified for a list variable"));
                        }
                        else if(stage == 1)
                        {
                            AddErrorExpecting(TEXT("<"), curToken);
                        }
                    }

                    if(stage == 1)
                    {
                        var->offset = id;
                        var->scope = VarScope_Param;
                        var->numElements = 0;

                        if(bOutVar)
                            var->flags |= VAR_OUT;
                    }
                    else if(stage > 1)
                    {
                        TypeInfo &paramType = *paramTypes.CreateNew();
                        TypeInfo &subParamType = *subParamTypes.CreateNew();
                        GetTypeInfo(varTypeName, paramType);
                        if(subType.IsValid())
                            GetTypeInfo(subType, subParamType);
                    }

                    String strVarName;
                    HandMeAToken(strVarName);

                    if(stage == 1)
                    {
                        var->name = strVarName;

                        TokenType type = GetTokenType(curClass, curStruct, var->name);

                        if((var->name[0] == ',') || (var->name[0] == ')'))
                        {
                            AddErrorExpecting(TEXT("name"), var->name.Array());
                            lpTemp--;
                        }
                        else if(!iswalpha(var->name[0]))
                        {
                            AddError(TEXT("Names must begin with valid characters: '%s'"), (TSTR)var->name);
                            //AddStartCharacter(var->name);
                        }
                        else if(type != TokenType_Variable && type != TokenType_Unknown)
                        {
                            AddError(TEXT("Parameter name not valid or has already been defined elsewhere: '%s'"), (TSTR)var->name);
                            //AddRandomNumber(var->name);
                        }
                        else
                        {
                            for(int i=0; i<id; i++)
                            {
                                if(funcDef->Params[i].name == var->name)
                                {
                                    AddErrorRedefinition(var->name);
                                    //AddRandomNumber(var->name);
                                    break;
                                }
                            }
                        }
                    }

                    HandMeAToken(curToken);

                    if(curToken == TEXT("="))
                    {
                        GetNextTokenEval(curToken);

                        if(stage == 1)
                        {
                            if(bOperator)
                                AddWarning(TEXT("Default parameter values are ignored for operator functions: parameter %d"), id);

                            if(var->flags & VAR_OUT)
                                AddError(TEXT("Output parameters cannot have default values: parameter %d"), id);

                            var->flags |= VAR_DEFAULT;

                            BufferOutputSerializer defOut(var->DefaultParamData);

                            type = GetTokenType(curClass, curStruct, curToken);

                            if(type == TokenType_Number)
                            {
                                if(GetBaseFloat(curToken))
                                {
                                    float fChi = (float)tstof(curToken);

                                    if(var->typeInfo.type == DataType_Float)
                                        defOut << fChi;
                                    else if(var->typeInfo.type == DataType_Integer)
                                    {
                                        int iChi = (int)fChi;
                                        defOut << iChi;
                                        //AddWarning(TEXT("Automatically converting float value to integer: %d"), iChi);
                                    }
                                    else
                                        AddErrorNoClue(curToken);
                                }
                                else
                                {
                                    int iChi = tstoi(curToken);

                                    if(var->typeInfo.type == DataType_Float)
                                    {
                                        float fChi = (float)iChi;
                                        defOut << fChi;
                                        //AddWarning(TEXT("Automatically converting int value to float"), fChi);
                                    }
                                    else if(var->typeInfo.type == DataType_Integer)
                                        defOut << iChi;
                                    else
                                        AddErrorNoClue(curToken);
                                }
                            }
                            else if(var->typeInfo.type == DataType_String)
                            {
                                if(type == TokenType_String)
                                    defOut << GetActualString(curToken);
                                else if(curToken == TEXT("null"))
                                    defOut << String();
                                else
                                    AddErrorNoClue(curToken);
                            }
                            else if(var->typeInfo.type == DataType_Object || var->typeInfo.type == DataType_Handle)
                            {
                                if(curToken != TEXT("null"))
                                    AddError(TEXT("Object/Handle variables cannot have any default value outside of 'null'"));
                                else
                                {
                                    Object *obj = NULL;
                                    defOut.Serialize(&obj, sizeof(Object*));
                                }
                            }
                            else if(type == TokenType_Enum)
                            {
                                int val = GetEnumVal(curToken);

                                if(var->typeInfo.type == DataType_Integer)
                                    defOut << val;
                                else
                                    AddErrorNoClue(curToken);
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
                            else
                                AddErrorNoClue(curToken);
                        }

                        HandMeAToken(curToken);
                    }
                    else
                        bAllDefaults = FALSE;

                    if(curToken[0] != ')')
                    {
                        if(stage == 1 && curToken[0] != ',')
                            AddErrorExpecting(TEXT(",' or ')"), curToken);

                        HandMeAToken(curToken);
                    }

                    ++id;
                }

                BOOL bValidFunc = TRUE;
                if(bOperator)
                {
                    int startErrors = errorCount;

                    if(stage == 1)
                    {
                        if(curStruct)
                        {
                            BOOL bValidPrefix = ValidPrefixOperator(funcDef->name);

                            if(funcDef->Params.Num() > 1)
                                AddError(TEXT("Operator '%s' has an invalid number of parameters"), (TSTR)funcDef->name);
                            else if( ((funcDef->Params.Num() == 0) && bValidPrefix)  || 
                                     ((funcDef->Params.Num() == 1) && !bValidPrefix) ||
                                     funcDef->name == TEXT("-"))
                            {
                                TokenType opType = GetTokenType(NULL, NULL, funcDef->name);

                                if( ((opType == TokenType_ConditionalOperator) || (funcDef->name.Compare(TEXT("!")))) &&
                                    (funcDef->returnType.type != DataType_Integer))
                                {
                                    AddError(TEXT("Return type for conditional operators must be an integer"));
                                }

                                for(int i=0; i<funcDef->Params.Num(); i++)
                                {
                                    if(funcDef->Params[i].flags & VAR_OUT)
                                        AddError(TEXT("Cannot use an 'out' parameter type with operators"));
                                }
                            }
                            else
                                AddError(TEXT("Operator '%s' has an invalid number of parameters"), (TSTR)funcDef->name);
                        }
                        else
                        {
                            BOOL bValidPrefix = ValidPrefixOperator(funcDef->name);

                            if(!funcDef->Params.Num() || (funcDef->Params.Num() > 2))
                                AddError(TEXT("Operator '%s' has an invalid number of parameters"), (TSTR)funcDef->name);
                            else if( ((funcDef->Params.Num() == 1) && bValidPrefix)  || 
                                     ((funcDef->Params.Num() == 2) && !bValidPrefix) ||
                                     funcDef->name == TEXT("-"))
                            {
                                TokenType opType = GetTokenType(NULL, NULL, funcDef->name);

                                if( ((opType == TokenType_ConditionalOperator) || (funcDef->name.Compare(TEXT("!")))) &&
                                    (funcDef->returnType.type != DataType_Integer))
                                {
                                    AddError(TEXT("Return type for conditional operators must be an integer"));
                                }

                                for(int i=0; i<funcDef->Params.Num(); i++)
                                {
                                    if(funcDef->Params[i].flags & VAR_OUT)
                                        AddError(TEXT("Cannot use an 'out' parameter type with operators"));
                                }
                            }
                            else
                                AddError(TEXT("Operator '%s' has an invalid number of parameters"), (TSTR)funcDef->name);
                        }
                    }

                    if(errorCount > startErrors)
                        bValidFunc = FALSE;
                }

                if(stage == 1)
                {
                    BOOL bFoundFunc = FALSE;
                    FunctionDefinition *curFunc = NULL;
                    while(curFunc = GetNextFunction(curClass, curStruct, funcDef->name, curFunc, TRUE))
                    {
                        if((curFunc->Params.Num() == funcDef->Params.Num()) && (curFunc->returnType == funcDef->returnType))
                        {
                            int i;
                            for(i=0; i<curFunc->Params.Num(); i++)
                            {
                                if(curFunc->Params[i].typeInfo != funcDef->Params[i].typeInfo)
                                    break;
                            }

                            if(i == curFunc->Params.Num())
                            {
                                if(bConstructor)
                                    AddError(TEXT("Constructor '%s' as declared already exists"), typeName.Array());
                                else
                                    AddError(TEXT("Function '%s' as declared already exists"), funcDef->name.Array());
                                bFoundFunc = TRUE;
                                break;
                            }
                        }
                    }

                    if(!bFoundFunc && bValidFunc)
                    {
                        if(!curClass && !curStruct)
                            funcDef->funcOffset = MAKEDWORD(funcs.Num(), module->moduleHash);
                        int id = funcs.Add(*funcDef);
                        zero(&tempFunc, sizeof(tempFunc));
                        funcDef = &funcs[id];
                    }
                    else
                        funcDef->FreeData();
                }
                else if(stage > 1)
                {
                    FunctionDefinition *curFunc = NULL;
                    while(curFunc = GetNextFunction(curClass, curStruct, funcName, curFunc))
                    {
                        if(curFunc->Params.Num() == paramTypes.Num())
                        {
                            if(curFunc->returnType != returnType)
                                continue;

                            int i;
                            for(i=0; i<curFunc->Params.Num(); i++)
                            {
                                if(curFunc->Params[i].typeInfo != paramTypes[i])
                                    break;
                                if(curFunc->Params[i].typeInfo.type == DataType_List)
                                {
                                    if(curFunc->Params[i].subTypeInfo != subParamTypes[i])
                                        break;
                                }
                            }

                            if(i == curFunc->Params.Num())
                            {
                                funcDef = curFunc;
                                break;
                            }
                        }
                    }

                    if(stage == 2 && curClass && bConstructor && bAllDefaults)
                        curClass->baseConstructor = funcDef;
                }

                paramTypes.Clear();
                subParamTypes.Clear();

                if(stage == 1)
                {
                    if(bInternal)
                        funcDef->flags |= FUNC_INTERNAL;
                    if(bImplementable)
                        funcDef->flags |= FUNC_IMPLEMENTABLE;
                    if(bOperator)
                        funcDef->flags |= FUNC_OPERATOR;
                    if(bStatic)
                        funcDef->flags |= FUNC_STATIC;
                }

                if(stage == 3 && !curClass && !curStruct && funcDef && bInternal)
                    CreateSourceGlobalNative(funcDef);

                HandMeAToken(curToken);

                if(bInternal)
                {
                    if(stage == 1 && curToken[0] != ';')
                        AddErrorExpecting(TEXT(";"), curToken);
                }
                else
                {
                    if(curToken[0] != '{')
                    {
                        if(stage == 1)
                            AddErrorExpecting(TEXT("{"), curToken);
                        if(!GotoToken(TEXT("{")))
                            {AddErrorEndOfCode(); return FALSE;}
                    }
                    else
                        --lpTemp;

                    if(stage != 2 || !funcDef)
                    {
                        if(!PassBracers(lpTemp))  {AddErrorEndOfCode(); return FALSE;}
                    }
                    else //stage == 2, function compilation stage
                    {
                        int lastErrorCount = errorCount;

                        if(!CompileCode(*funcDef))
                        {
                            if(!GetNextToken(curToken, TRUE))
                                AddErrorEndOfCode();
                            else
                                AddErrorUnrecoverable();

                            return FALSE;
                        }

                        AlignOffset(funcDef->LocalVariableStackSize, funcDef->localVarAlign);

                        if(errorCount > lastErrorCount)
                            funcDef->ByteCode.Clear();
                    }
                }
            }
        }
    }

    if(!curClass && !curStruct && bGeneratingBytecodeText)
        WriteCurrentCodeOffset(NULL);

    if(curClass)
        return FALSE; //because class stuff terminates at '}', not here

    return errorCount == 0;
}
