/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Controller.h

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


#ifndef CONTROLLER_HEADER
#define CONTROLLER_HEADER



/*=========================================================
    Controller: controls Characters
==========================================================*/

class Character;

class BASE_EXPORT Controller : public FrameObject
{
    friend class Character;
    DeclareClass(Controller, FrameObject);

    List<Character*> CharacterList;

public:
    void Destroy();

    inline int NumCharacters() const {return CharacterList.Num();}
    inline Character* GetCharacter(int id=0) const
    {
        if(id == 0 && NumCharacters() == 0)
            return NULL;
        return CharacterList[id];
    }

    virtual void GotControl(Character *character)       {scriptGotControl(character);}
    virtual void LosingControl(Character *character)    {scriptLosingControl(character);}

    void Serialize(Serializer &s);

    //<Script module="Base" classdecs="Controller">
    void scriptGotControl(Character* character)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetObject(0, (Object*)character);

        GetLocalClass()->CallScriptMember(this, 2, cs);
    }

    void scriptLosingControl(Character* character)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetObject(0, (Object*)character);

        GetLocalClass()->CallScriptMember(this, 3, cs);
    }

    Declare_Internal_Member(native_NumCharacters);
    Declare_Internal_Member(native_GetCharacter);
    //</Script>
};

#endif

