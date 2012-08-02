/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  UndoRedoStack.cpp

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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#include "Xed.h"


void UndoRedoStack::Push(Action &action, BOOL bClearRedo)
{
    stack.Insert(0, action);

    zero(&action, sizeof(Action));

    if(this == curUndoStack)
    {
        if(bClearRedo)
            curRedoStack->Clear();
    }

    updateEditMenu();

    if((this == editor->undoStack) || (this == editor->redoStack))
        levelInfo->bModified = TRUE;
}

void UndoRedoStack::Pop(DWORD num)
{
    assert(num);

    BOOL bIsUndoStack = (this == curUndoStack);

    if(num)
    {
        for(int i=num-1; i>=0; i--)
        {
            if(!stack.Num())
                break;

            BufferInputSerializer s(stack[0].data);

            Action reverseAction;
            reverseAction.strName      << stack[0].strName;
            reverseAction.actionProc    = stack[0].actionProc;

            BufferOutputSerializer sOut(reverseAction.data);

            stack[0].actionProc(s, sOut, bIsUndoStack);
            stack[0].data.Clear();
            stack[0].strName.Clear();
            stack.Remove(0);

            if(bIsUndoStack)
                curRedoStack->Push(reverseAction, FALSE);
            else
                curUndoStack->Push(reverseAction, FALSE);
        }
    }

    updateEditMenu();
}

UndoRedoStack::~UndoRedoStack()
{
    for(int i=0; i<stack.Num(); i++)
    {
        stack[i].strName.Clear();
        stack[i].data.Clear();
    }
    stack.Clear();
}

void UndoRedoStack::Clear()
{
    for(int i=0; i<stack.Num(); i++)
    {
        stack[i].strName.Clear();
        stack[i].data.Clear();
    }
    stack.Clear();

    updateEditMenu();
}

void ENGINEAPI CreateUndoRedoStacks(UPDATEEDITMENUPROC updateEditMenu, UndoRedoStack *& newUndoStack, UndoRedoStack *& newRedoStack)
{
    newUndoStack = new UndoRedoStack;
    newRedoStack = new UndoRedoStack;

    newRedoStack->curUndoStack = newUndoStack->curUndoStack = newUndoStack;
    newRedoStack->curRedoStack = newUndoStack->curRedoStack = newRedoStack;

    newUndoStack->updateEditMenu = newRedoStack->updateEditMenu = updateEditMenu;
}
