/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Window.h:  2D Objects

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

#ifndef WINDOW_HEADER
#define WINDOW_HEADER


enum OffsetType
{
    Offset_TopLeft,
    Offset_TopCenter,
    Offset_TopRight,
    Offset_CenterLeft,
    Offset_Center,
    Offset_CenterRight,
    Offset_BottomLeft,
    Offset_BottomCenter,
    Offset_BottomRight
};

enum {Window_Command=0x1000};


class BASE_EXPORT Window : public FrameObject
{
    DeclareClass(Window, FrameObject);

    Window *Parent;
    List<Window*> Children;

    GraphicsSystem *curGraphicsSystem;

    BOOL bFullScreenObj;

    OffsetType posOffset;

protected:
    Vect2 Pos;
    Vect2 Size;

public:
    Window();
    virtual ~Window();

    void Destroy();

    void PreFrame();

    inline void SetPos(const Vect2 &newPos) {Pos = newPos;}
    inline void SetPos(float x, float y)    {Pos.Set(x, y);}
    inline void SetPosX(float x)            {Pos.x = x;}
    inline void SetPosY(float y)            {Pos.y = y;}

    inline const Vect2& GetLocalPos() const {return Pos;}
    inline float GetLocalX() const {return Pos.x;}
    inline float GetLocalY() const {return Pos.y;}
    virtual Vect2 GetRealPos();

    inline const Vect2& GetSize() const {return Size;}
    inline float GetSizeX() const {return Size.x;}
    inline float GetSizeY() const {return Size.y;}
    inline void SetSize(const Vect2 &newSize) {Size = newSize;}
    inline void SetSize(float x, float y) {Size.Set(x, y);}

    inline void SetOffsetType(OffsetType newOffset)  {posOffset = newOffset;}
    inline OffsetType GetOffsetType() {return posOffset;}

    void SetPosOffset(const Vect2 &offsetPos, BOOL bCentered=FALSE);
    inline void SetPosOffset(float x, float y, BOOL bCentered=FALSE)  {SetPosOffset(Vect2(x, y), bCentered);}

    Vect2 GetOffsetPoint(OffsetType offset);

    void SetFullScreen(BOOL bSet);

    void Attach(Window *new_parent);
    void Detach();

    inline Window* GetParent()              {return Parent;}
    inline unsigned int NumChildren()       {return Children.Num();}
    inline Window* GetChild(unsigned int i) {return Children[i];}

    void SetTopLevel();

    void SetSystem(GraphicsSystem *graphicsSystem);
    inline GraphicsSystem* GetSystem() {return curGraphicsSystem;}

    inline UINT SendParentMessage(UINT message, Object *param=NULL) {return (Parent != NULL) ? Parent->OnMessage(this, message, param) : 0;}
    virtual UINT OnMessage(Window *control, UINT message, Object* param=NULL) {return (UINT)scriptOnMessage(control, (int)message, param);}

    //<Script module="Base" classdecs="Window">
    int id;

    int scriptOnMessage(Window* child, int message, Object* param)
    {
        CallStruct cs;
        cs.SetNumParams(3);
        cs.SetObject(0, (Object*)child);
        cs.SetInt(1, message);
        cs.SetObject(2, (Object*)param);

        GetLocalClass()->CallScriptMember(this, 25, cs);

        return cs.GetInt(RETURNVAL);
    }

    Declare_Internal_Member(native_SetPos);
    Declare_Internal_Member(native_SetSize);
    Declare_Internal_Member(native_GetRealPos);
    Declare_Internal_Member(native_GetLocalPos);
    Declare_Internal_Member(native_GetSize);
    Declare_Internal_Member(native_SetOffsetType);
    Declare_Internal_Member(native_GetOffsetType);
    Declare_Internal_Member(native_SetPosOffset);
    Declare_Internal_Member(native_GetOffsetPoint);
    Declare_Internal_Member(native_SetFullScreen);
    Declare_Internal_Member(native_Attach);
    Declare_Internal_Member(native_Detach);
    Declare_Internal_Member(native_GetParent);
    Declare_Internal_Member(native_NumChildren);
    Declare_Internal_Member(native_GetChild);
    Declare_Internal_Member(native_SetTopLevel);
    Declare_Internal_Member(native_SetSystem);
    Declare_Internal_Member(native_GetSystem);
    //</Script>
};

struct CursorPosInfo
{
    int x; int y;
};

class BASE_EXPORT ControlWindow : public Window
{
    DeclareClass(ControlWindow, Window);

    static ControlWindow *focusedWindow;

    static BOOL bIgnoreMove;
    static List<CursorPosInfo>  CursorPosStack;

    static List<ControlWindow*> InputControlStack;

public:
    void Destroy();

    //focus functions
    BOOL KeepingFocus()                         {return bKeepFocus;}
    BOOL HasFocus()                             {return (this==focusedWindow);}

    virtual void GotFocus()                     {scriptGotFocus();}
    virtual void LostFocus()                    {scriptLostFocus();}

    //takes control of keyboard/joystick input regardless of window focus (mouse input not included)
    void TakeInputControl(BOOL bTake);

    //keyboard stuff
    virtual BOOL KeyDown(unsigned int kbc)      {return scriptKeyDown(kbc);}
    virtual BOOL KeyUp(unsigned int kbc)        {return scriptKeyUp(kbc);}

    //mouse stuff (does not count as regular input)
    virtual void MouseDown(DWORD button)        {scriptMouseDown(button);}
    virtual void MouseUp(DWORD button)          {scriptMouseUp(button);}
    virtual void MouseWheel(short scroll)       {scriptMouseWheel(scroll);}
    virtual void MouseMove(int x, int y, short x_offset, short y_offset)    {scriptMouseMove(x, y, x_offset, y_offset);}

    //static stuff
    static void SetFocusWindow(ControlWindow *window);

    static ControlWindow* CurrentInputWindow()  {return InputControlStack.Num() ? InputControlStack[0] : focusedWindow;}
    static ControlWindow* CurrentFocusWindow()  {return focusedWindow;}

    static void PushCursorPos();
    static void PopCursorPos(BOOL bSendMouseMove=FALSE);

    static void SetUnechoedCursorPos(int x, int y);

    static void ENGINEAPI WindowKeyboardHandler(int kbc, BOOL bKeyDown, GraphicsSystem *curGraphicsSystem);
    static void ENGINEAPI WindowMouseHandler(int action, PARAM param1, PARAM param2, GraphicsSystem *curGraphicsSystem);

    //<Script module="Base" classdecs="ControlWindow">
    BOOL bKeepFocus;

    void scriptGotFocus()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 2, cs);
    }

    void scriptLostFocus()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 3, cs);
    }

    BOOL scriptKeyDown(int kbc)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetInt(0, kbc);

        GetLocalClass()->CallScriptMember(this, 4, cs);

        return cs.GetInt(RETURNVAL);
    }

    BOOL scriptKeyUp(int kbc)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetInt(0, kbc);

        GetLocalClass()->CallScriptMember(this, 5, cs);

        return cs.GetInt(RETURNVAL);
    }

    void scriptMouseDown(int button)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetInt(0, button);

        GetLocalClass()->CallScriptMember(this, 6, cs);
    }

    void scriptMouseUp(int button)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetInt(0, button);

        GetLocalClass()->CallScriptMember(this, 7, cs);
    }

    void scriptMouseMove(int x, int y, int x_offset, int y_offset)
    {
        CallStruct cs;
        cs.SetNumParams(4);
        cs.SetInt(0, x);
        cs.SetInt(1, y);
        cs.SetInt(2, x_offset);
        cs.SetInt(3, y_offset);

        GetLocalClass()->CallScriptMember(this, 8, cs);
    }

    void scriptMouseWheel(int scroll)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetInt(0, scroll);

        GetLocalClass()->CallScriptMember(this, 9, cs);
    }

    Declare_Internal_Member(native_TakeInputControl);
    Declare_Internal_Member(native_HasFocus);
    //</Script>
};

inline void PushCursorPos()                             {ControlWindow::PushCursorPos();}
inline void PopCursorPos(BOOL bSendMouseMove=FALSE)     {ControlWindow::PopCursorPos(bSendMouseMove);}
inline ControlWindow* CurrentInputWindow()              {return ControlWindow::CurrentInputWindow();}
inline ControlWindow* CurrentFocusWindow()              {return ControlWindow::CurrentFocusWindow();}
inline void SetUnechoedCursorPos(int x, int y)          {ControlWindow::SetUnechoedCursorPos(x, y);}

//<Script module="Base" globaldecs="Window.xscript">
Declare_Native_Global(NativeGlobal_PushCursorPos);
Declare_Native_Global(NativeGlobal_PopCursorPos);
Declare_Native_Global(NativeGlobal_CurrentInputWindow);
Declare_Native_Global(NativeGlobal_CurrentFocusWindow);
Declare_Native_Global(NativeGlobal_SetUnechoedCursorPos);
//</Script>


#endif
