/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Input.h:  Input Devices

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

#ifndef INPUT_HEADER
#define INPUT_HEADER



/*=========================================================
    Input
===========================================================*/

//-----------------------------------------
// defines/typedefs

#define KBC_ESCAPE      0x0
#define KBC_1           0x1
#define KBC_2           0x2
#define KBC_3           0x3
#define KBC_4           0x4
#define KBC_5           0x5
#define KBC_6           0x6
#define KBC_7           0x7
#define KBC_8           0x8
#define KBC_9           0x9
#define KBC_0           0xA
#define KBC_MINUS       0xB
#define KBC_EQUALS      0xC
#define KBC_BACK        0xD
#define KBC_TAB         0xE
#define KBC_Q           0xF
#define KBC_W           0x10
#define KBC_E           0x11
#define KBC_R           0x12
#define KBC_T           0x13
#define KBC_Y           0x14
#define KBC_U           0x15
#define KBC_I           0x16
#define KBC_O           0x17
#define KBC_P           0x18
#define KBC_LBRACKET    0x19
#define KBC_RBRACKET    0x1A
#define KBC_RETURN      0x1B
#define KBC_LCONTROL    0x1C
#define KBC_A           0x1D
#define KBC_S           0x1E
#define KBC_D           0x1F
#define KBC_F           0x20
#define KBC_G           0x21
#define KBC_H           0x22
#define KBC_J           0x23
#define KBC_K           0x24
#define KBC_L           0x25
#define KBC_SEMICOLON   0x26
#define KBC_APOSTROPHE  0x27
#define KBC_TILDE       0x28
#define KBC_LSHIFT      0x29
#define KBC_BACKSLASH   0x2A
#define KBC_Z           0x2B
#define KBC_X           0x2C
#define KBC_C           0x2D
#define KBC_V           0x2E
#define KBC_B           0x2F
#define KBC_N           0x30
#define KBC_M           0x31
#define KBC_COMMA       0x32
#define KBC_PERIOD      0x33
#define KBC_SLASH       0x34
#define KBC_RSHIFT      0x35
#define KBC_MULTIPLY    0x36
#define KBC_LALT        0x37
#define KBC_SPACE       0x38
#define KBC_CAPSLOCK    0x39
#define KBC_F1          0x3A
#define KBC_F2          0x3B
#define KBC_F3          0x3C
#define KBC_F4          0x3D
#define KBC_F5          0x3E
#define KBC_F6          0x3F
#define KBC_F7          0x40
#define KBC_F8          0x41
#define KBC_F9          0x42
#define KBC_F10         0x43
#define KBC_NUMLOCK     0x44
#define KBC_SCROLLLOCK  0x45
#define KBC_NUMPAD7     0x46
#define KBC_NUMPAD8     0x47
#define KBC_NUMPAD9     0x48
#define KBC_SUBTRACT    0x49
#define KBC_NUMPAD4     0x4A
#define KBC_NUMPAD5     0x4B
#define KBC_NUMPAD6     0x4C
#define KBC_ADD         0x4D
#define KBC_NUMPAD1     0x4E
#define KBC_NUMPAD2     0x4F
#define KBC_NUMPAD3     0x50
#define KBC_NUMPAD0     0x51
#define KBC_DECIMAL     0x52
#define KBC_F11         0x53
#define KBC_F12         0x54
#define KBC_NUMPADENTER 0x55
#define KBC_RCONTROL    0x56
#define KBC_DIVIDE      0x57
#define KBC_SYSRQ       0x58
#define KBC_RALT        0x59
#define KBC_PAUSE       0x5A
#define KBC_HOME        0x5B
#define KBC_UP          0x5C
#define KBC_PAGEDOWN    0x5D
#define KBC_LEFT        0x5E
#define KBC_RIGHT       0x5F
#define KBC_END         0x60
#define KBC_DOWN        0x61
#define KBC_PAGEUP      0x62
#define KBC_INSERT      0x63
#define KBC_DELETE      0x64


#define MOUSE_LEFTBUTTON    0x65
#define MOUSE_MIDDLEBUTTON  0x66
#define MOUSE_RIGHTBUTTON   0x67
#define MOUSE_WHEEL         0x68
#define MOUSE_MOVE          0x69

#define KBC_CONTROL         0xFFFFFFFE
#define KBC_ALT             0xFFFFFFFD
#define KBC_SHIFT           0xFFFFFFFC

#define STATE_LBUTTONDOWN   0x001
#define STATE_RBUTTONDOWN   0x010
#define STATE_MBUTTONDOWN   0x100



//-----------------------------------------
// Input Handler Classes
class BASE_EXPORT MouseInputHandler : public Object
{
    friend class Input;
    DeclareClass(MouseInputHandler, Object);

    Input *inputCaller;

public:
    ~MouseInputHandler();

    virtual void MouseHandler(int action, DWORD buttonStates, int param);

    virtual void MouseDown(DWORD button)                                    {scriptMouseDown(button);}
    virtual void MouseUp(DWORD button)                                      {scriptMouseUp(button);}
    virtual void MouseMove(int x, int y, int x_offset, int y_offset)        {scriptMouseMove(x, y, x_offset, y_offset);}
    virtual void MouseWheel(int scroll)                                     {scriptMouseWheel(scroll);}

    //<Script module="Base" classdecs="MouseInputHandler">
    void scriptMouseHandler(int action, int buttonStates, int param)
    {
        CallStruct cs;
        cs.SetNumParams(3);
        cs.SetInt(0, action);
        cs.SetInt(1, buttonStates);
        cs.SetInt(2, param);

        GetLocalClass()->CallScriptMember(this, 0, cs);
    }

    void scriptMouseDown(int button)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetInt(0, button);

        GetLocalClass()->CallScriptMember(this, 1, cs);
    }

    void scriptMouseUp(int button)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetInt(0, button);

        GetLocalClass()->CallScriptMember(this, 2, cs);
    }

    void scriptMouseMove(int x, int y, int x_offset, int y_offset)
    {
        CallStruct cs;
        cs.SetNumParams(4);
        cs.SetInt(0, x);
        cs.SetInt(1, y);
        cs.SetInt(2, x_offset);
        cs.SetInt(3, y_offset);

        GetLocalClass()->CallScriptMember(this, 3, cs);
    }

    void scriptMouseWheel(int scroll)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetInt(0, scroll);

        GetLocalClass()->CallScriptMember(this, 4, cs);
    }
    //</Script>
};

class BASE_EXPORT KeyboardInputHandler : public Object
{
    friend class Input;
    DeclareClass(KeyboardInputHandler, Object);

    Input *inputCaller;

public:
    ~KeyboardInputHandler();

    virtual void KeyboardHandler(unsigned int kbc, BOOL bDown);

    virtual void KeyDown(unsigned int kbc)                                  {scriptKeyDown(kbc);}
    virtual void KeyUp(unsigned int kbc)                                    {scriptKeyUp(kbc);}

    //<Script module="Base" classdecs="KeyboardInputHandler">
    BOOL bCharInput;

    void scriptKeyboardHandler(int kbc, BOOL bDown)
    {
        CallStruct cs;
        cs.SetNumParams(2);
        cs.SetInt(0, kbc);
        cs.SetInt(1, (int)bDown);

        GetLocalClass()->CallScriptMember(this, 0, cs);
    }

    void scriptKeyDown(int kbc)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetInt(0, kbc);

        GetLocalClass()->CallScriptMember(this, 1, cs);
    }

    void scriptKeyUp(int kbc)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetInt(0, kbc);

        GetLocalClass()->CallScriptMember(this, 2, cs);
    }

    //</Script>
};


//-----------------------------------------
// Input Class

class BASE_EXPORT Input : public Object
{
    friend class KeyboardInputHandler;
    friend class MouseInputHandler;

    DeclareClass(Input, Object);

protected:
    List<KeyboardInputHandler*> curKBHandler;
    List<MouseInputHandler*>    curMouseHandler;

public:
    virtual ~Input() {}

    void PushKBHandler(KeyboardInputHandler *kbHandler, BOOL bCharInput=FALSE);
    void PushMouseHandler(MouseInputHandler *mouseHandler);


    inline KeyboardInputHandler* GetCurKBHandler()  {return curKBHandler.Num() ? curKBHandler[0] : NULL;}
    inline MouseInputHandler* GetCurMouseHandler()  {return curMouseHandler.Num() ? curMouseHandler[0] : NULL;}

    inline  BOOL GettingCharacterInput()
    {
        if(!curKBHandler.Num())
            return FALSE;

        return curKBHandler[0]->bCharInput;
    }

    inline  void EmulateMouseInput(int action, DWORD buttonStates, LONG param)
    {
        if(curMouseHandler.Num())
            curMouseHandler[0]->MouseHandler(action, buttonStates, param);
        else
            ControlWindow::WindowMouseHandler(action, buttonStates, param, GS);
    }

    inline  void EmulateKBInput(unsigned int kbc, BOOL keydown)
    {
        if(curKBHandler.Num())
            curKBHandler[0]->KeyboardHandler(kbc, keydown);
        else
            ControlWindow::WindowKeyboardHandler(kbc, keydown, GS);
    }

    virtual BOOL GetButtonState(unsigned int key) {return 0;}

    virtual void ProcessInput();

    //<Script module="Base" classdecs="Input">
    Declare_Internal_Member(native_PushKBHandler);
    Declare_Internal_Member(native_PushMouseHandler);
    Declare_Internal_Member(native_GetCurKBHandler);
    Declare_Internal_Member(native_GetCurMouseHandler);
    Declare_Internal_Member(native_EmulateMouseInput);
    Declare_Internal_Member(native_EmulateKBInput);
    Declare_Internal_Member(native_GetButtonState);
    //</Script>
};


//<Script module="Base" globaldecs="Input.xscript">
Declare_Native_Global(NativeGlobal_GetInputCodeName);
Declare_Native_Global(NativeGlobal_PushKBHandler);
Declare_Native_Global(NativeGlobal_PushMouseHandler);
//</Script>


BASE_EXPORT CTSTR ENGINEAPI GetInputCodeName(unsigned int code);


inline void PushKBHandler(KeyboardInputHandler *kbHandler, BOOL bCharInput=FALSE) {if(GS && GS->GetInput()) GS->GetInput()->PushKBHandler(kbHandler, bCharInput);}
inline void PushMouseHandler(MouseInputHandler *mouseHandler)                     {if(GS && GS->GetInput()) GS->GetInput()->PushMouseHandler(mouseHandler);}

inline BOOL GetButtonState(unsigned int key)        {return GS ? GS->GetInput()->GetButtonState(key) : FALSE;}


extern DWORD curMouseButtonStates;


#endif
