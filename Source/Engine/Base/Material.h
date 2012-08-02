/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Material.h

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

#ifndef MATERIAL_HEADER
#define MATERIAL_HEADER


class PhyObject;


//-----------------------------------------
// Material flags
#define MATERIAL_TWOSIDED       1
#define MATERIAL_TRANSPARENT    2
#define MATERIAL_EDITING        (1<<31)


/*=========================================================
    Material
==========================================================*/

//-----------------------------------------
// Material Parameter

struct MaterialParameter
{
    HANDLE handle;
    ShaderParameterType type;
    BYTE data[256];
};

//-----------------------------------------
// Material
class BASE_EXPORT Material : public Object
{
    friend class  MaterialEditor;
    friend class  MaterialItem;
    friend class  MaterialWindow;
    friend class  MeshBrowser;
    friend struct MeshWindow;
    friend class  PrefabBrowser;
    friend struct PrefabWindow;
    friend class  EditorLevelInfo;
    friend class  Level;
    friend class  MeshEntity;
    friend struct Brush;

    DeclareClass(Material, Object);

    MaterialParameter *GetParam(CTSTR lpName);

    Effect *effect;
    List<MaterialParameter> Params;
    DWORD flags;

    float friction, restitution;

    String strSoundSoft;
    String strSoundHard;

    void FreeParameters();

public:
   Material() : friction(0.5f) {}
    ~Material();

    BOOL LoadFromFile(CTSTR lpFile);

    void SetCurrentEffect(Effect *effectIn);
    Effect* GetCurrentEffect();

    void SetFloat(CTSTR paramName, float fValue);
    void SetColor(CTSTR paramName, DWORD color);
    void SetCurrentTexture(CTSTR paramName, BaseTexture *texture);

    float GetFloat(CTSTR paramName);
    DWORD GetColor(CTSTR paramName);
    BaseTexture* GetCurrentTexture(CTSTR paramName);

    inline void SetSoftHitSound(CTSTR softSound)        {strSoundSoft = softSound; if(!strSoundHard.IsValid()) strSoundHard = softSound;}
    inline void SetHardHitSound(CTSTR hardSound)        {strSoundHard = hardSound; if(!strSoundSoft.IsValid()) strSoundSoft = hardSound;}
    inline CTSTR GetSoftHitSound() const                {return strSoundSoft;}
    inline CTSTR GetHardHitSound() const                {return strSoundHard;}
    inline void ClearSounds()                           {strSoundSoft.Clear(); strSoundHard.Clear();}

    inline Effect* GetEffect() const                    {return effect;}
    BOOL LoadParameters();

    DWORD GetFlags() const                          {return flags;}

    inline void SetFriction(float newFriction)          {friction = newFriction;}
    inline void SetRestitution(float newRestitution)    {restitution = newRestitution;}
    inline float GetFriction() const                    {return friction;}
    inline float GetRestitution() const                 {return restitution;}

    void ProcessCollision(PhyObject *obj, PhyObject *collider, float appliedImpulse, const Vect &hitPos);

    int GetParamID(CTSTR paramName);

    void SetFloat(int param, float fValue);
    void SetColor(int param, DWORD color);
    void SetCurrentTexture(int param, BaseTexture *texture);

    float GetFloat(int param);
    DWORD GetColor(int param);
    BaseTexture* GetCurrentTexture(int param);

    //<Script module="Base" classdecs="Material">
    Declare_Internal_Member(native_LoadFromFile);
    Declare_Internal_Member(native_SetCurrentEffect);
    Declare_Internal_Member(native_GetCurrentEffect);
    Declare_Internal_Member(native_SetFloat);
    Declare_Internal_Member(native_SetColor);
    Declare_Internal_Member(native_SetCurrentTexture);
    Declare_Internal_Member(native_GetFloat);
    Declare_Internal_Member(native_GetColor);
    Declare_Internal_Member(native_GetCurrentTexture);
    Declare_Internal_Member(native_GetParamID);
    Declare_Internal_Member(native_SetFloat_2);
    Declare_Internal_Member(native_SetColor_2);
    Declare_Internal_Member(native_SetCurrentTexture_2);
    Declare_Internal_Member(native_GetFloat_2);
    Declare_Internal_Member(native_GetColor_2);
    Declare_Internal_Member(native_GetCurrentTexture_2);
    //</Script>
};






#endif
