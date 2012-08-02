/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  GraphicsSystem:  Graphics System

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


#include "..\Base.h"

//<Script module="Base" filedefs="Font.xscript">
void Font::native_LetterWidth(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    int letter = cs.GetInt(0);

    returnVal = LetterWidth(letter);
}

void Font::native_WordWidth(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    String str = cs.GetString(0);

    returnVal = WordWidth(str);
}

void Font::native_TextWidth(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    String str = cs.GetString(0);

    returnVal = TextWidth(str);
}

void Font::native_GetFontHeight(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = GetFontHeight();
}

void Font::native_DrawLetter(CallStruct &cs)
{
    int letter = cs.GetInt(0);
    int x = cs.GetInt(1);
    int y = cs.GetInt(2);
    DWORD fontColor = (DWORD)cs.GetInt(3);

    DrawLetter(letter, x, y, fontColor);
}
//</Script>

//<Script module="Base" filedefs="GraphicsSystem.xscript">
void Texture::native_Width(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = Width();
}

void Texture::native_Height(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = Height();
}

void Texture::native_HasAlpha(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = HasAlpha();
}

void CubeTexture::native_Size(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = Width();
}

void Shader::native_NumParams(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = NumParams();
}

void Shader::native_GetParameter(CallStruct &cs)
{
    HANDLE& returnVal = cs.GetHandleOut(RETURNVAL);
    int parameter = cs.GetInt(0);

    returnVal = GetParameter(parameter);
}

void Shader::native_GetParameterByName(CallStruct &cs)
{
    HANDLE& returnVal = cs.GetHandleOut(RETURNVAL);
    String strName = cs.GetString(0);

    returnVal = GetParameterByName(strName);
}

void Shader::native_SetBool(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    BOOL bValue = (BOOL)cs.GetInt(1);

    SetBool(hObject, bValue);
}

void Shader::native_SetFloat(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    float fValue = cs.GetFloat(1);

    SetFloat(hObject, fValue);
}

void Shader::native_SetInt(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    int iValue = cs.GetInt(1);

    SetInt(hObject, iValue);
}

void Shader::native_SetVector(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    const Vect &value = (const Vect&)cs.GetStruct(1);

    SetVector(hObject, value);
}

void Shader::native_SetVector2(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    const Vect2 &value = (const Vect2&)cs.GetStruct(1);

    SetVector2(hObject, value);
}

void Shader::native_SetVector4(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    const Vect4 &value = (const Vect4&)cs.GetStruct(1);

    SetVector4(hObject, value);
}

void Shader::native_SetTexture(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    BaseTexture* texture = (BaseTexture*)cs.GetObject(1);

    SetTexture(hObject, texture);
}

void Shader::native_SetColor(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    const Vect4 &value = (const Vect4&)cs.GetStruct(1);

    SetColor(hObject, value);
}

void Shader::native_SetColor_2(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    float fR = cs.GetFloat(1);
    float fB = cs.GetFloat(2);
    float fG = cs.GetFloat(3);
    float fA = cs.GetFloat(4);

    SetColor(hObject, fR, fB, fG, fA);
}

void Shader::native_SetColor_3(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    DWORD color = (DWORD)cs.GetInt(1);

    SetColor(hObject, color);
}

void Shader::native_SetColor3(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    const Vect &value = (const Vect&)cs.GetStruct(1);

    SetColor3(hObject, value);
}

void Shader::native_SetColor3_2(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    float fR = cs.GetFloat(1);
    float fB = cs.GetFloat(2);
    float fG = cs.GetFloat(3);

    SetColor3(hObject, fR, fB, fG);
}

void Shader::native_SetColor3_3(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    DWORD color = (DWORD)cs.GetInt(1);

    SetColor3(hObject, color);
}

void Shader::native_SetVector4_2(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    float fX = cs.GetFloat(1);
    float fY = cs.GetFloat(2);
    float fZ = cs.GetFloat(3);
    float fW = cs.GetFloat(4);

    SetVector4(hObject, fX, fY, fZ, fW);
}

void Shader::native_SetVector_2(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    float fX = cs.GetFloat(1);
    float fY = cs.GetFloat(2);
    float fZ = cs.GetFloat(3);

    SetVector(hObject, fX, fY, fZ);
}

void Shader::native_SetMatrix(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    const Matrix &mat = (const Matrix&)cs.GetStruct(1);

    SetMatrix(hObject, mat);
}

void Shader::native_SetMatrixIdentity(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);

    SetMatrixIdentity(hObject);
}

void Effect::native_GetTechnique(CallStruct &cs)
{
    HANDLE& returnVal = cs.GetHandleOut(RETURNVAL);
    String lpTechnique = cs.GetString(0);

    returnVal = GetTechnique(lpTechnique);
}

void Effect::native_UsableTechnique(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    HANDLE hObject = cs.GetHandle(0);

    returnVal = UsableTechnique(hObject);
}

void Effect::native_GetPass(CallStruct &cs)
{
    HANDLE& returnVal = cs.GetHandleOut(RETURNVAL);
    HANDLE hTechnique = cs.GetHandle(0);
    int i = cs.GetInt(1);

    returnVal = GetPass(hTechnique, i);
}

void Effect::native_GetPassByName(CallStruct &cs)
{
    HANDLE& returnVal = cs.GetHandleOut(RETURNVAL);
    HANDLE hTechnique = cs.GetHandle(0);
    String lpName = cs.GetString(1);

    returnVal = GetPassByName(hTechnique, lpName);
}

void Effect::native_BeginTechnique(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    HANDLE hTechnique = cs.GetHandle(0);

    returnVal = BeginTechnique(hTechnique);
}

void Effect::native_EndTechnique(CallStruct &cs)
{
    EndTechnique();
}

void Effect::native_BeginPass(CallStruct &cs)
{
    int i = cs.GetInt(0);

    BeginPass(i);
}

void Effect::native_BeginPassByName(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    String lpName = cs.GetString(0);

    returnVal = BeginPassByName(lpName);
}

void Effect::native_BeginPassByHandle(CallStruct &cs)
{
    HANDLE hPass = cs.GetHandle(0);

    BeginPassByHandle(hPass);
}

void Effect::native_EndPass(CallStruct &cs)
{
    EndPass();
}

void Effect::native_NumParams(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = NumParams();
}

void Effect::native_GetParameter(CallStruct &cs)
{
    HANDLE& returnVal = cs.GetHandleOut(RETURNVAL);
    int parameter = cs.GetInt(0);

    returnVal = GetParameter(parameter);
}

void Effect::native_GetParameterByName(CallStruct &cs)
{
    HANDLE& returnVal = cs.GetHandleOut(RETURNVAL);
    String lpName = cs.GetString(0);

    returnVal = GetParameterByName(lpName);
}

void Effect::native_GetBool(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    BOOL &bValue = (BOOL&)cs.GetIntOut(1);

    GetBool(hObject, bValue);
}

void Effect::native_GetFloat(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    float &fValue = cs.GetFloatOut(1);

    GetFloat(hObject, fValue);
}

void Effect::native_GetInt(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    int &iValue = cs.GetIntOut(1);

    GetInt(hObject, iValue);
}

void Effect::native_GetVector(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    Vect &value = (Vect&)cs.GetStructOut(1);

    GetVector(hObject, value);
}

void Effect::native_GetVector2(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    Vect2 &value = (Vect2&)cs.GetStructOut(1);

    GetVector2(hObject, value);
}

void Effect::native_GetVector4(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    Vect4 &value = (Vect4&)cs.GetStructOut(1);

    GetVector4(hObject, value);
}

void Effect::native_GetString(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    String &value = cs.GetStringOut(1);

    GetString(hObject, value);
}

void Effect::native_GetDefaultBool(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    HANDLE hObject = cs.GetHandle(0);
    BOOL &bValue = (BOOL&)cs.GetIntOut(1);

    returnVal = GetDefaultBool(hObject, bValue);
}

void Effect::native_GetDefaultFloat(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    HANDLE hObject = cs.GetHandle(0);
    float &fValue = cs.GetFloatOut(1);

    returnVal = GetDefaultFloat(hObject, fValue);
}

void Effect::native_GetDefaultInt(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    HANDLE hObject = cs.GetHandle(0);
    int &iValue = cs.GetIntOut(1);

    returnVal = GetDefaultInt(hObject, iValue);
}

void Effect::native_GetDefaultVector(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    HANDLE hObject = cs.GetHandle(0);
    Vect &value = (Vect&)cs.GetStructOut(1);

    returnVal = GetDefaultVector(hObject, value);
}

void Effect::native_GetDefaultVector2(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    HANDLE hObject = cs.GetHandle(0);
    Vect2 &value = (Vect2&)cs.GetStructOut(1);

    returnVal = GetDefaultVector2(hObject, value);
}

void Effect::native_GetDefaultVector4(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    HANDLE hObject = cs.GetHandle(0);
    Vect4 &value = (Vect4&)cs.GetStructOut(1);

    returnVal = GetDefaultVector4(hObject, value);
}

void Effect::native_GetDefaultTexture(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    HANDLE hObject = cs.GetHandle(0);
    String &value = cs.GetStringOut(1);

    returnVal = GetDefaultTexture(hObject, value);
}

void Effect::native_SetBool(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    BOOL bValue = (BOOL)cs.GetInt(1);

    SetBool(hObject, bValue);
}

void Effect::native_SetFloat(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    float fValue = cs.GetFloat(1);

    SetFloat(hObject, fValue);
}

void Effect::native_SetInt(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    int iValue = cs.GetInt(1);

    SetInt(hObject, iValue);
}

void Effect::native_SetVector(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    const Vect &value = (const Vect&)cs.GetStruct(1);

    SetVector(hObject, value);
}

void Effect::native_SetVector2(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    const Vect2 &value = (const Vect2&)cs.GetStruct(1);

    SetVector2(hObject, value);
}

void Effect::native_SetVector4(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    const Vect4 &value = (const Vect4&)cs.GetStruct(1);

    SetVector4(hObject, value);
}

void Effect::native_SetTexture(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    BaseTexture* texture = (BaseTexture*)cs.GetObject(1);

    SetTexture(hObject, texture);
}

void Effect::native_GetViewProj(CallStruct &cs)
{
    HANDLE& returnVal = cs.GetHandleOut(RETURNVAL);

    returnVal = GetViewProj();
}

void Effect::native_GetWorld(CallStruct &cs)
{
    HANDLE& returnVal = cs.GetHandleOut(RETURNVAL);

    returnVal = GetWorld();
}

void Effect::native_GetScale(CallStruct &cs)
{
    HANDLE& returnVal = cs.GetHandleOut(RETURNVAL);

    returnVal = GetScale();
}

void Effect::native_GetColor(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    Vect4 &value = (Vect4&)cs.GetStructOut(1);

    GetColor(hObject, value);
}

void Effect::native_GetColor_2(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    DWORD &color = (DWORD&)cs.GetIntOut(1);

    GetColor(hObject, color);
}

void Effect::native_GetDefaultColor(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    HANDLE hObject = cs.GetHandle(0);
    Vect4 &value = (Vect4&)cs.GetStructOut(1);

    returnVal = GetDefaultColor(hObject, value);
}

void Effect::native_GetDefaultColor_2(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    HANDLE hObject = cs.GetHandle(0);
    DWORD &color = (DWORD&)cs.GetIntOut(1);

    returnVal = GetDefaultColor(hObject, color);
}

void Effect::native_SetColor(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    const Vect4 &value = (const Vect4&)cs.GetStruct(1);

    SetColor(hObject, value);
}

void Effect::native_SetColor_2(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    float fR = cs.GetFloat(1);
    float fB = cs.GetFloat(2);
    float fG = cs.GetFloat(3);
    float fA = cs.GetFloat(4);

    SetColor(hObject, fR, fB, fG, fA);
}

void Effect::native_SetColor_3(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    DWORD color = (DWORD)cs.GetInt(1);

    SetColor(hObject, color);
}

void Effect::native_GetColor3(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    Vect &value = (Vect&)cs.GetStructOut(1);

    GetColor3(hObject, value);
}

void Effect::native_GetColor3_2(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    DWORD &color = (DWORD&)cs.GetIntOut(1);

    GetColor3(hObject, color);
}

void Effect::native_GetDefaultColor3(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    HANDLE hObject = cs.GetHandle(0);
    Vect &value = (Vect&)cs.GetStructOut(1);

    returnVal = GetDefaultColor3(hObject, value);
}

void Effect::native_GetDefaultColor3_2(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    HANDLE hObject = cs.GetHandle(0);
    DWORD &color = (DWORD&)cs.GetIntOut(1);

    returnVal = GetDefaultColor3(hObject, color);
}

void Effect::native_SetColor3(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    const Vect &value = (const Vect&)cs.GetStruct(1);

    SetColor3(hObject, value);
}

void Effect::native_SetColor3_2(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    float fR = cs.GetFloat(1);
    float fB = cs.GetFloat(2);
    float fG = cs.GetFloat(3);

    SetColor3(hObject, fR, fB, fG);
}

void Effect::native_SetColor3_3(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    DWORD color = (DWORD)cs.GetInt(1);

    SetColor3(hObject, color);
}

void Effect::native_SetVector4_2(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    float fX = cs.GetFloat(1);
    float fY = cs.GetFloat(2);
    float fZ = cs.GetFloat(3);
    float fW = cs.GetFloat(4);

    SetVector4(hObject, fX, fY, fZ, fW);
}

void Effect::native_SetVector_2(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    float fX = cs.GetFloat(1);
    float fY = cs.GetFloat(2);
    float fZ = cs.GetFloat(3);

    SetVector(hObject, fX, fY, fZ);
}

void Effect::native_SetMatrix(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);
    const Matrix &mat = (const Matrix&)cs.GetStruct(1);

    SetMatrix(hObject, mat);
}

void Effect::native_SetMatrixIdentity(CallStruct &cs)
{
    HANDLE hObject = cs.GetHandle(0);

    SetMatrixIdentity(hObject);
}

void MeshObject::native_MeshObject(CallStruct &cs)
{
    String strResource = cs.GetString(0);
    SetMesh(strResource);
}

void MeshObject::native_Render(CallStruct &cs)
{
    Render();
}

void MeshObject::native_RenderBare(CallStruct &cs)
{
    RenderBare();
}

void MeshObject::native_SetMaterial(CallStruct &cs)
{
    int texture = cs.GetInt(0);
    Material* material = (Material*)cs.GetObject(1);

    SetMaterial(texture, material);
}

void GraphicsSystem::native_GetSize(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);

    returnVal = GetSize();
}

void GraphicsSystem::native_GetSizeX(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetSizeX();
}

void GraphicsSystem::native_GetSizeY(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetSizeY();
}

void GraphicsSystem::native_MatrixPush(CallStruct &cs)
{
    MatrixPush();
}

void GraphicsSystem::native_MatrixPop(CallStruct &cs)
{
    MatrixPop();
}

void GraphicsSystem::native_MatrixSet(CallStruct &cs)
{
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    MatrixSet(m);
}

void GraphicsSystem::native_MatrixMultiply(CallStruct &cs)
{
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    MatrixMultiply(m);
}

void GraphicsSystem::native_MatrixRotate(CallStruct &cs)
{
    float x = cs.GetFloat(0);
    float y = cs.GetFloat(1);
    float z = cs.GetFloat(2);
    float a = cs.GetFloat(3);

    MatrixRotate(x, y, z, a);
}

void GraphicsSystem::native_MatrixRotate_2(CallStruct &cs)
{
    const AxisAngle &aa = (const AxisAngle&)cs.GetStruct(0);

    MatrixRotate(aa);
}

void GraphicsSystem::native_MatrixRotate_3(CallStruct &cs)
{
    const Quat &q = (const Quat&)cs.GetStruct(0);

    MatrixRotate(q);
}

void GraphicsSystem::native_MatrixTranslate(CallStruct &cs)
{
    float x = cs.GetFloat(0);
    float y = cs.GetFloat(1);
    float z = cs.GetFloat(2);

    MatrixTranslate(x, y, z);
}

void GraphicsSystem::native_MatrixTranslate_2(CallStruct &cs)
{
    const Vect &pos = (const Vect&)cs.GetStruct(0);

    MatrixTranslate(pos);
}

void GraphicsSystem::native_MatrixTranslate_3(CallStruct &cs)
{
    const Vect2 &pos2 = (const Vect2&)cs.GetStruct(0);

    MatrixTranslate(pos2);
}

void GraphicsSystem::native_MatrixScale(CallStruct &cs)
{
    const Vect &scale = (const Vect&)cs.GetStruct(0);

    MatrixScale(scale);
}

void GraphicsSystem::native_MatrixScale_2(CallStruct &cs)
{
    float x = cs.GetFloat(0);
    float y = cs.GetFloat(1);
    float z = cs.GetFloat(2);

    MatrixScale(x, y, z);
}

void GraphicsSystem::native_MatrixTranspose(CallStruct &cs)
{
    MatrixTranspose();
}

void GraphicsSystem::native_MatrixInverse(CallStruct &cs)
{
    MatrixInverse();
}

void GraphicsSystem::native_MatrixIdentity(CallStruct &cs)
{
    MatrixIdentity();
}

void GraphicsSystem::native_SetCurFont(CallStruct &cs)
{
    String name = cs.GetString(0);

    SetCurFont(name);
}

void GraphicsSystem::native_SetCurFont_2(CallStruct &cs)
{
    Font* font = (Font*)cs.GetObject(0);

    SetCurFont(font);
}

void GraphicsSystem::native_GetFont(CallStruct &cs)
{
    Font*& returnVal = (Font*&)cs.GetObjectOut(RETURNVAL);
    String name = cs.GetString(0);

    returnVal = GetFont(name);
}

void GraphicsSystem::native_SetFontColor(CallStruct &cs)
{
    DWORD color = (DWORD)cs.GetInt(0);

    SetFontColor(color);
}

void GraphicsSystem::native_SetFontColor_2(CallStruct &cs)
{
    const Color4 &color = (const Color4&)cs.GetStruct(0);

    SetFontColor(color);
}

void GraphicsSystem::native_DrawText(CallStruct &cs)
{
    int x = cs.GetInt(0);
    int y = cs.GetInt(1);
    int cx = cs.GetInt(2);
    int cy = cs.GetInt(3);
    BOOL bWrapWords = (BOOL)cs.GetInt(4);
    String text = cs.GetString(5);

    DrawText(x, y, cx, cy, bWrapWords, text);
}

void GraphicsSystem::native_DrawTextCenter(CallStruct &cs)
{
    int x = cs.GetInt(0);
    int y = cs.GetInt(1);
    String text = cs.GetString(2);

    DrawTextCenter(x, y, text);
}

void GraphicsSystem::native_RenderStartNew(CallStruct &cs)
{
    RenderStartNew();
}

void GraphicsSystem::native_RenderStart(CallStruct &cs)
{
    RenderStart();
}

void GraphicsSystem::native_RenderStop(CallStruct &cs)
{
    GSDrawMode dwDrawMode = (GSDrawMode)cs.GetInt(0);

    RenderStop(dwDrawMode);
}

void GraphicsSystem::native_RenderSave(CallStruct &cs)
{
    VertexBuffer*& returnVal = (VertexBuffer*&)cs.GetObjectOut(RETURNVAL);

    returnVal = RenderSave();
}

void GraphicsSystem::native_Vertex(CallStruct &cs)
{
    float x = cs.GetFloat(0);
    float y = cs.GetFloat(1);
    float z = cs.GetFloat(2);

    Vertex(x, y, z);
}

void GraphicsSystem::native_Vertex_2(CallStruct &cs)
{
    const Vect &v = (const Vect&)cs.GetStruct(0);

    Vertex(v);
}

void GraphicsSystem::native_Normal(CallStruct &cs)
{
    float x = cs.GetFloat(0);
    float y = cs.GetFloat(1);
    float z = cs.GetFloat(2);

    Normal(x, y, z);
}

void GraphicsSystem::native_Normal_2(CallStruct &cs)
{
    const Vect &v = (const Vect&)cs.GetStruct(0);

    Normal(v);
}

void GraphicsSystem::native_Color(CallStruct &cs)
{
    DWORD dwRGBA = (DWORD)cs.GetInt(0);

    Color(dwRGBA);
}

void GraphicsSystem::native_Color_2(CallStruct &cs)
{
    const Color4 &v = (const Color4&)cs.GetStruct(0);

    Color(v);
}

void GraphicsSystem::native_TexCoord(CallStruct &cs)
{
    float u = cs.GetFloat(0);
    float v = cs.GetFloat(1);
    int idTexture = cs.GetInt(2);

    TexCoord(u, v, idTexture);
}

void GraphicsSystem::native_TexCoord_2(CallStruct &cs)
{
    const Vect2 &uv = (const Vect2&)cs.GetStruct(0);
    int idTexture = cs.GetInt(1);

    TexCoord(uv, idTexture);
}

void GraphicsSystem::native_LoadVertexBuffer(CallStruct &cs)
{
    VertexBuffer* vb = (VertexBuffer*)cs.GetObject(0);

    LoadVertexBuffer(vb);
}

void GraphicsSystem::native_LoadTexture(CallStruct &cs)
{
    BaseTexture* texture = (BaseTexture*)cs.GetObject(0);
    int idTexture = cs.GetInt(1);

    LoadTexture(texture, idTexture);
}

void GraphicsSystem::native_LoadSamplerState(CallStruct &cs)
{
    SamplerState* sampler = (SamplerState*)cs.GetObject(0);
    int idSampler = cs.GetInt(1);

    LoadSamplerState(sampler, idSampler);
}

void GraphicsSystem::native_LoadIndexBuffer(CallStruct &cs)
{
    IndexBuffer* ib = (IndexBuffer*)cs.GetObject(0);

    LoadIndexBuffer(ib);
}

void GraphicsSystem::native_LoadDefault2DSampler(CallStruct &cs)
{
    int idSampler = cs.GetInt(0);

    LoadDefault2DSampler(idSampler);
}

void GraphicsSystem::native_LoadDefault3DSampler(CallStruct &cs)
{
    int idSampler = cs.GetInt(0);

    LoadDefault3DSampler(idSampler);
}

void GraphicsSystem::native_LoadVertexShader(CallStruct &cs)
{
    Shader* vShader = (Shader*)cs.GetObject(0);

    LoadVertexShader(vShader);
}

void GraphicsSystem::native_LoadPixelShader(CallStruct &cs)
{
    Shader* pShader = (Shader*)cs.GetObject(0);

    LoadPixelShader(pShader);
}

void GraphicsSystem::native_SetFrameBufferTarget(CallStruct &cs)
{
    BaseTexture* texture = (BaseTexture*)cs.GetObject(0);
    int side = cs.GetInt(1);

    SetFrameBufferTarget(texture, side);
}

void GraphicsSystem::native_Draw(CallStruct &cs)
{
    GSDrawMode DrawMode = (GSDrawMode)cs.GetInt(0);
    int vertexOffset = cs.GetInt(1);
    int StartVert = cs.GetInt(2);
    int nVerts = cs.GetInt(3);

    Draw(DrawMode, vertexOffset, StartVert, nVerts);
}

void GraphicsSystem::native_DrawBare(CallStruct &cs)
{
    GSDrawMode DrawMode = (GSDrawMode)cs.GetInt(0);
    int vertexOffset = cs.GetInt(1);
    int StartVert = cs.GetInt(2);
    int nVerts = cs.GetInt(3);

    DrawBare(DrawMode, vertexOffset, StartVert, nVerts);
}

void GraphicsSystem::native_ReverseCullMode(CallStruct &cs)
{
    BOOL bReverse = (BOOL)cs.GetInt(0);

    ReverseCullMode(bReverse);
}

void GraphicsSystem::native_SetCullMode(CallStruct &cs)
{
    GSCullMode side = (GSCullMode)cs.GetInt(0);

    SetCullMode(side);
}

void GraphicsSystem::native_GetCullMode(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = GetCullMode();
}

void GraphicsSystem::native_EnableBlending(CallStruct &cs)
{
    BOOL bEnable = (BOOL)cs.GetInt(0);

    EnableBlending(bEnable);
}

void GraphicsSystem::native_BlendFunction(CallStruct &cs)
{
    GSBlendType srcFactor = (GSBlendType)cs.GetInt(0);
    GSBlendType destFactor = (GSBlendType)cs.GetInt(1);

    BlendFunction(srcFactor, destFactor);
}

void GraphicsSystem::native_ClearDepthBuffer(CallStruct &cs)
{
    BOOL bFullClear = (BOOL)cs.GetInt(0);

    ClearDepthBuffer(bFullClear);
}

void GraphicsSystem::native_EnableDepthTest(CallStruct &cs)
{
    BOOL bEnable = (BOOL)cs.GetInt(0);

    EnableDepthTest(bEnable);
}

void GraphicsSystem::native_DepthWriteEnable(CallStruct &cs)
{
    BOOL bEnable = (BOOL)cs.GetInt(0);

    DepthWriteEnable(bEnable);
}

void GraphicsSystem::native_DepthFunction(CallStruct &cs)
{
    GSDepthTest function = (GSDepthTest)cs.GetInt(0);

    DepthFunction(function);
}

void GraphicsSystem::native_GetDepthFunction(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = GetDepthFunction();
}

void GraphicsSystem::native_ClearColorBuffer(CallStruct &cs)
{
    BOOL bFullClear = (BOOL)cs.GetInt(0);
    DWORD color = (DWORD)cs.GetInt(1);

    ClearColorBuffer(bFullClear, color);
}

void GraphicsSystem::native_ColorWriteEnable(CallStruct &cs)
{
    BOOL bRedEnable = (BOOL)cs.GetInt(0);
    BOOL bGreenEnable = (BOOL)cs.GetInt(1);
    BOOL bBlueEnable = (BOOL)cs.GetInt(2);
    BOOL bAlphaEnable = (BOOL)cs.GetInt(3);

    ColorWriteEnable(bRedEnable, bGreenEnable, bBlueEnable, bAlphaEnable);
}

void GraphicsSystem::native_ClearStencilBuffer(CallStruct &cs)
{
    BOOL bFill = (BOOL)cs.GetInt(0);

    ClearStencilBuffer(bFill);
}

void GraphicsSystem::native_EnableStencilTest(CallStruct &cs)
{
    BOOL bEnable = (BOOL)cs.GetInt(0);

    EnableStencilTest(bEnable);
}

void GraphicsSystem::native_StencilWriteEnable(CallStruct &cs)
{
    BOOL bEnable = (BOOL)cs.GetInt(0);

    StencilWriteEnable(bEnable);
}

void GraphicsSystem::native_StencilFunction(CallStruct &cs)
{
    GSDepthTest function = (GSDepthTest)cs.GetInt(0);

    StencilFunction(function);
}

void GraphicsSystem::native_StencilOp(CallStruct &cs)
{
    GSStencilOp fail = (GSStencilOp)cs.GetInt(0);
    GSStencilOp zfail = (GSStencilOp)cs.GetInt(1);
    GSStencilOp zpass = (GSStencilOp)cs.GetInt(2);

    StencilOp(fail, zfail, zpass);
}

void GraphicsSystem::native_SetPointSize(CallStruct &cs)
{
    float size = cs.GetFloat(0);

    SetPointSize(size);
}

void GraphicsSystem::native_ToggleFullScreen(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = ToggleFullScreen();
}

void GraphicsSystem::native_SetResolution(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const DisplayMode &displayMode = (const DisplayMode&)cs.GetStruct(0);
    BOOL bCenter = (BOOL)cs.GetInt(1);

    returnVal = SetResolution(displayMode, bCenter);
}

void GraphicsSystem::native_AdjustDisplayColors(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    float gamma = cs.GetFloat(0);
    float brightness = cs.GetFloat(1);
    float contrast = cs.GetFloat(2);

    returnVal = AdjustDisplayColors(gamma, brightness, contrast);
}

void GraphicsSystem::native_GetCurrentDisplayMode(CallStruct &cs)
{
    DisplayMode &displayMode = (DisplayMode&)cs.GetStructOut(0);

    GetCurrentDisplayMode(displayMode);
}

void GraphicsSystem::native_IsFullscreen(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = IsFullscreen();
}

void GraphicsSystem::native_GetInput(CallStruct &cs)
{
    Input*& returnVal = (Input*&)cs.GetObjectOut(RETURNVAL);

    returnVal = GetInput();
}

void GraphicsSystem::native_Set2DMode(CallStruct &cs)
{
    Set2DMode();
}

void GraphicsSystem::native_Set3DMode(CallStruct &cs)
{
    float fovy = cs.GetFloat(0);
    float znear = cs.GetFloat(1);
    float zfar = cs.GetFloat(2);

    Set3DMode(fovy, znear, zfar);
}

void GraphicsSystem::native_Perspective(CallStruct &cs)
{
    float fovy = cs.GetFloat(0);
    float aspect = cs.GetFloat(1);
    float znear = cs.GetFloat(2);
    float zfar = cs.GetFloat(3);

    Perspective(fovy, aspect, znear, zfar);
}

void GraphicsSystem::native_Ortho(CallStruct &cs)
{
    float left = cs.GetFloat(0);
    float right = cs.GetFloat(1);
    float top = cs.GetFloat(2);
    float bottom = cs.GetFloat(3);
    float znear = cs.GetFloat(4);
    float zfar = cs.GetFloat(5);

    Ortho(left, right, top, bottom, znear, zfar);
}

void GraphicsSystem::native_Frustum(CallStruct &cs)
{
    float left = cs.GetFloat(0);
    float right = cs.GetFloat(1);
    float top = cs.GetFloat(2);
    float bottom = cs.GetFloat(3);
    float znear = cs.GetFloat(4);
    float zfar = cs.GetFloat(5);

    Frustum(left, right, top, bottom, znear, zfar);
}

void GraphicsSystem::native_DrawSprite(CallStruct &cs)
{
    Texture* texture = (Texture*)cs.GetObject(0);
    float x = cs.GetFloat(1);
    float y = cs.GetFloat(2);
    float x2 = cs.GetFloat(3);
    float y2 = cs.GetFloat(4);

    DrawSprite(texture, x, y, x2, y2);
}

void GraphicsSystem::native_DrawSprite3D(CallStruct &cs)
{
    const Quat &cameraRot = (const Quat&)cs.GetStruct(0);
    Texture* texture = (Texture*)cs.GetObject(1);
    const Vect &pos = (const Vect&)cs.GetStruct(2);
    float sizeX = cs.GetFloat(3);
    float sizeY = cs.GetFloat(4);
    float rotation = cs.GetFloat(5);

    DrawSprite3D(cameraRot, texture, pos, sizeX, sizeY, rotation);
}

void GraphicsSystem::native_DrawSpriteCenter(CallStruct &cs)
{
    Texture* texture = (Texture*)cs.GetObject(0);
    float x = cs.GetFloat(1);
    float y = cs.GetFloat(2);
    DWORD color = (DWORD)cs.GetInt(3);

    DrawSpriteCenter(texture, x, y, color);
}

void GraphicsSystem::native_DrawCubeBackdrop(CallStruct &cs)
{
    Camera* cam = (Camera*)cs.GetObject(0);
    CubeTexture* cubetexture = (CubeTexture*)cs.GetObject(1);

    DrawCubeBackdrop(cam, cubetexture);
}

void GraphicsSystem::native_DrawCubeBackdrop_2(CallStruct &cs)
{
    Camera* cam = (Camera*)cs.GetObject(0);
    CubeTexture* cubetexture = (CubeTexture*)cs.GetObject(1);
    const Quat &customRot = (const Quat&)cs.GetStruct(2);

    DrawCubeBackdrop(cam, cubetexture, customRot);
}

void GraphicsSystem::native_DrawSpriteEx(CallStruct &cs)
{
    Texture* texture = (Texture*)cs.GetObject(0);
    DWORD color = (DWORD)cs.GetInt(1);
    float x = cs.GetFloat(2);
    float y = cs.GetFloat(3);
    float x2 = cs.GetFloat(4);
    float y2 = cs.GetFloat(5);
    float u = cs.GetFloat(6);
    float v = cs.GetFloat(7);
    float u2 = cs.GetFloat(8);
    float v2 = cs.GetFloat(9);

    DrawSpriteEx(texture, color, x, y, x2, y2, u, v, u2, v2);
}

void GraphicsSystem::native_GetMouseOver(CallStruct &cs)
{
    ControlWindow*& returnVal = (ControlWindow*&)cs.GetObjectOut(RETURNVAL);
    int x = cs.GetInt(0);
    int y = cs.GetInt(1);

    returnVal = GetMouseOver(x, y);
}

void GraphicsSystem::native_GetLocalMousePos(CallStruct &cs)
{
    int &x = cs.GetIntOut(0);
    int &y = cs.GetIntOut(1);

    GetLocalMousePos(x, y);
}

void GraphicsSystem::native_SetLocalMousePos(CallStruct &cs)
{
    int x = cs.GetInt(0);
    int y = cs.GetInt(1);

    SetLocalMousePos(x, y);
}

void ENGINEAPI NativeGlobal_GS(CallStruct &cs)
{
    GraphicsSystem*& returnVal = (GraphicsSystem*&)cs.GetObjectOut(RETURNVAL);

    returnVal = GS;
}

void ENGINEAPI NativeGlobal_MatrixPush(CallStruct &cs)
{
    MatrixPush();
}

void ENGINEAPI NativeGlobal_MatrixPop(CallStruct &cs)
{
    MatrixPop();
}

void ENGINEAPI NativeGlobal_MatrixSet(CallStruct &cs)
{
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    MatrixSet(m);
}

void ENGINEAPI NativeGlobal_MatrixMultiply(CallStruct &cs)
{
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    MatrixMultiply(m);
}

void ENGINEAPI NativeGlobal_MatrixRotate(CallStruct &cs)
{
    float x = cs.GetFloat(0);
    float y = cs.GetFloat(1);
    float z = cs.GetFloat(2);
    float a = cs.GetFloat(3);

    MatrixRotate(x, y, z, a);
}

void ENGINEAPI NativeGlobal_MatrixRotate_2(CallStruct &cs)
{
    const AxisAngle &aa = (const AxisAngle&)cs.GetStruct(0);

    MatrixRotate(aa);
}

void ENGINEAPI NativeGlobal_MatrixRotate_3(CallStruct &cs)
{
    const Quat &q = (const Quat&)cs.GetStruct(0);

    MatrixRotate(q);
}

void ENGINEAPI NativeGlobal_MatrixTranslate(CallStruct &cs)
{
    float x = cs.GetFloat(0);
    float y = cs.GetFloat(1);
    float z = cs.GetFloat(2);

    MatrixTranslate(x, y, z);
}

void ENGINEAPI NativeGlobal_MatrixTranslate_2(CallStruct &cs)
{
    const Vect &pos = (const Vect&)cs.GetStruct(0);

    MatrixTranslate(pos);
}

void ENGINEAPI NativeGlobal_MatrixTranslate_3(CallStruct &cs)
{
    const Vect2 &pos2 = (const Vect2&)cs.GetStruct(0);

    MatrixTranslate(pos2);
}

void ENGINEAPI NativeGlobal_MatrixScale(CallStruct &cs)
{
    const Vect &scale = (const Vect&)cs.GetStruct(0);

    MatrixScale(scale);
}

void ENGINEAPI NativeGlobal_MatrixScale_2(CallStruct &cs)
{
    float x = cs.GetFloat(0);
    float y = cs.GetFloat(1);
    float z = cs.GetFloat(2);

    MatrixScale(x, y, z);
}

void ENGINEAPI NativeGlobal_MatrixTranspose(CallStruct &cs)
{
    MatrixTranspose();
}

void ENGINEAPI NativeGlobal_MatrixInverse(CallStruct &cs)
{
    MatrixInverse();
}

void ENGINEAPI NativeGlobal_MatrixIdentity(CallStruct &cs)
{
    MatrixIdentity();
}

void ENGINEAPI NativeGlobal_SetCurFont(CallStruct &cs)
{
    String name = cs.GetString(0);

    SetCurFont(name);
}

void ENGINEAPI NativeGlobal_SetCurFont_2(CallStruct &cs)
{
    Font* font = (Font*)cs.GetObject(0);

    SetCurFont(font);
}

void ENGINEAPI NativeGlobal_GetFont(CallStruct &cs)
{
    Font*& returnVal = (Font*&)cs.GetObjectOut(RETURNVAL);
    String name = cs.GetString(0);

    returnVal = GetFont(name);
}

void ENGINEAPI NativeGlobal_SetFontColor(CallStruct &cs)
{
    DWORD color = (DWORD)cs.GetInt(0);

    SetFontColor(color);
}

void ENGINEAPI NativeGlobal_SetFontColor_2(CallStruct &cs)
{
    const Color4 &color = (const Color4&)cs.GetStruct(0);

    SetFontColor(color);
}

void ENGINEAPI NativeGlobal_DrawText(CallStruct &cs)
{
    int x = cs.GetInt(0);
    int y = cs.GetInt(1);
    int cx = cs.GetInt(2);
    int cy = cs.GetInt(3);
    BOOL bWrapWords = (BOOL)cs.GetInt(4);
    String text = cs.GetString(5);

    DrawText(x, y, cx, cy, bWrapWords, text);
}

void ENGINEAPI NativeGlobal_DrawTextCenter(CallStruct &cs)
{
    int x = cs.GetInt(0);
    int y = cs.GetInt(1);
    String text = cs.GetString(2);

    DrawTextCenter(x, y, text);
}

void ENGINEAPI NativeGlobal_RenderStartNew(CallStruct &cs)
{
    RenderStartNew();
}

void ENGINEAPI NativeGlobal_RenderStart(CallStruct &cs)
{
    RenderStart();
}

void ENGINEAPI NativeGlobal_RenderStop(CallStruct &cs)
{
    GSDrawMode dwDrawMode = (GSDrawMode)cs.GetInt(0);

    RenderStop(dwDrawMode);
}

void ENGINEAPI NativeGlobal_RenderSave(CallStruct &cs)
{
    VertexBuffer*& returnVal = (VertexBuffer*&)cs.GetObjectOut(RETURNVAL);

    returnVal = RenderSave();
}

void ENGINEAPI NativeGlobal_Vertex(CallStruct &cs)
{
    float x = cs.GetFloat(0);
    float y = cs.GetFloat(1);
    float z = cs.GetFloat(2);

    Vertex(x, y, z);
}

void ENGINEAPI NativeGlobal_Vertex_2(CallStruct &cs)
{
    const Vect &v = (const Vect&)cs.GetStruct(0);

    Vertex(v);
}

void ENGINEAPI NativeGlobal_Normal(CallStruct &cs)
{
    float x = cs.GetFloat(0);
    float y = cs.GetFloat(1);
    float z = cs.GetFloat(2);

    Normal(x, y, z);
}

void ENGINEAPI NativeGlobal_Normal_2(CallStruct &cs)
{
    const Vect &v = (const Vect&)cs.GetStruct(0);

    Normal(v);
}

void ENGINEAPI NativeGlobal_Color(CallStruct &cs)
{
    DWORD dwRGBA = (DWORD)cs.GetInt(0);

    Color(dwRGBA);
}

void ENGINEAPI NativeGlobal_Color_2(CallStruct &cs)
{
    const Color4 &v = (const Color4&)cs.GetStruct(0);

    Color(v);
}

void ENGINEAPI NativeGlobal_TexCoord(CallStruct &cs)
{
    float u = cs.GetFloat(0);
    float v = cs.GetFloat(1);
    int idTexture = cs.GetInt(2);

    TexCoord(u, v, idTexture);
}

void ENGINEAPI NativeGlobal_TexCoord_2(CallStruct &cs)
{
    const Vect2 &uv = (const Vect2&)cs.GetStruct(0);
    int idTexture = cs.GetInt(1);

    TexCoord(uv, idTexture);
}

void ENGINEAPI NativeGlobal_LoadVertexBuffer(CallStruct &cs)
{
    VertexBuffer* vb = (VertexBuffer*)cs.GetObject(0);

    LoadVertexBuffer(vb);
}

void ENGINEAPI NativeGlobal_LoadTexture(CallStruct &cs)
{
    BaseTexture* texture = (BaseTexture*)cs.GetObject(0);
    int idTexture = cs.GetInt(1);

    LoadTexture(texture, idTexture);
}

void ENGINEAPI NativeGlobal_LoadSamplerState(CallStruct &cs)
{
    SamplerState* sampler = (SamplerState*)cs.GetObject(0);
    int idSampler = cs.GetInt(1);

    LoadSamplerState(sampler, idSampler);
}

void ENGINEAPI NativeGlobal_LoadIndexBuffer(CallStruct &cs)
{
    IndexBuffer* ib = (IndexBuffer*)cs.GetObject(0);

    LoadIndexBuffer(ib);
}

void ENGINEAPI NativeGlobal_LoadDefault2DSampler(CallStruct &cs)
{
    int idSampler = cs.GetInt(0);

    LoadDefault2DSampler(idSampler);
}

void ENGINEAPI NativeGlobal_LoadDefault3DSampler(CallStruct &cs)
{
    int idSampler = cs.GetInt(0);

    LoadDefault3DSampler(idSampler);
}

void ENGINEAPI NativeGlobal_LoadVertexShader(CallStruct &cs)
{
    Shader* vShader = (Shader*)cs.GetObject(0);

    LoadVertexShader(vShader);
}

void ENGINEAPI NativeGlobal_LoadPixelShader(CallStruct &cs)
{
    Shader* pShader = (Shader*)cs.GetObject(0);

    LoadPixelShader(pShader);
}

void ENGINEAPI NativeGlobal_SetFrameBufferTarget(CallStruct &cs)
{
    BaseTexture* texture = (BaseTexture*)cs.GetObject(0);
    int side = cs.GetInt(1);

    SetFrameBufferTarget(texture, side);
}

void ENGINEAPI NativeGlobal_Draw(CallStruct &cs)
{
    GSDrawMode DrawMode = (GSDrawMode)cs.GetInt(0);
    int vertexOffset = cs.GetInt(1);
    int StartVert = cs.GetInt(2);
    int nVerts = cs.GetInt(3);

    Draw(DrawMode, vertexOffset, StartVert, nVerts);
}

void ENGINEAPI NativeGlobal_DrawBare(CallStruct &cs)
{
    GSDrawMode DrawMode = (GSDrawMode)cs.GetInt(0);
    int vertexOffset = cs.GetInt(1);
    int StartVert = cs.GetInt(2);
    int nVerts = cs.GetInt(3);

    GS->DrawBare(DrawMode, vertexOffset, StartVert, nVerts);
}

void ENGINEAPI NativeGlobal_SetCullMode(CallStruct &cs)
{
    GSCullMode side = (GSCullMode)cs.GetInt(0);

    SetCullMode(side);
}

void ENGINEAPI NativeGlobal_GetCullMode(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = GetCullMode();
}

void ENGINEAPI NativeGlobal_EnableBlending(CallStruct &cs)
{
    BOOL bEnable = (BOOL)cs.GetInt(0);

    EnableBlending(bEnable);
}

void ENGINEAPI NativeGlobal_BlendFunction(CallStruct &cs)
{
    GSBlendType srcFactor = (GSBlendType)cs.GetInt(0);
    GSBlendType destFactor = (GSBlendType)cs.GetInt(1);

    BlendFunction(srcFactor, destFactor);
}

void ENGINEAPI NativeGlobal_ClearDepthBuffer(CallStruct &cs)
{
    BOOL bFullClear = (BOOL)cs.GetInt(0);

    ClearDepthBuffer(bFullClear);
}

void ENGINEAPI NativeGlobal_EnableDepthTest(CallStruct &cs)
{
    BOOL bEnable = (BOOL)cs.GetInt(0);

    EnableDepthTest(bEnable);
}

void ENGINEAPI NativeGlobal_DepthWriteEnable(CallStruct &cs)
{
    BOOL bEnable = (BOOL)cs.GetInt(0);

    DepthWriteEnable(bEnable);
}

void ENGINEAPI NativeGlobal_DepthFunction(CallStruct &cs)
{
    GSDepthTest function = (GSDepthTest)cs.GetInt(0);

    DepthFunction(function);
}

void ENGINEAPI NativeGlobal_GetDepthFunction(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = GetDepthFunction();
}

void ENGINEAPI NativeGlobal_ClearColorBuffer(CallStruct &cs)
{
    BOOL bFullClear = (BOOL)cs.GetInt(0);
    DWORD color = (DWORD)cs.GetInt(1);

    ClearColorBuffer(bFullClear, color);
}

void ENGINEAPI NativeGlobal_ColorWriteEnable(CallStruct &cs)
{
    BOOL bRedEnable = (BOOL)cs.GetInt(0);
    BOOL bGreenEnable = (BOOL)cs.GetInt(1);
    BOOL bBlueEnable = (BOOL)cs.GetInt(2);
    BOOL bAlphaEnable = (BOOL)cs.GetInt(3);

    ColorWriteEnable(bRedEnable, bGreenEnable, bBlueEnable, bAlphaEnable);
}

void ENGINEAPI NativeGlobal_ClearStencilBuffer(CallStruct &cs)
{
    BOOL bFill = (BOOL)cs.GetInt(0);

    ClearStencilBuffer(bFill);
}

void ENGINEAPI NativeGlobal_EnableStencilTest(CallStruct &cs)
{
    BOOL bEnable = (BOOL)cs.GetInt(0);

    EnableStencilTest(bEnable);
}

void ENGINEAPI NativeGlobal_StencilWriteEnable(CallStruct &cs)
{
    BOOL bEnable = (BOOL)cs.GetInt(0);

    StencilWriteEnable(bEnable);
}

void ENGINEAPI NativeGlobal_StencilFunction(CallStruct &cs)
{
    GSDepthTest function = (GSDepthTest)cs.GetInt(0);

    StencilFunction(function);
}

void ENGINEAPI NativeGlobal_StencilOp(CallStruct &cs)
{
    GSStencilOp fail = (GSStencilOp)cs.GetInt(0);
    GSStencilOp zfail = (GSStencilOp)cs.GetInt(1);
    GSStencilOp zpass = (GSStencilOp)cs.GetInt(2);

    StencilOp(fail, zfail, zpass);
}

void ENGINEAPI NativeGlobal_SetPointSize(CallStruct &cs)
{
    float size = cs.GetFloat(0);

    SetPointSize(size);
}

void ENGINEAPI NativeGlobal_ToggleFullScreen(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = ToggleFullScreen();
}

void ENGINEAPI NativeGlobal_SetResolution(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const DisplayMode &displayMode = (const DisplayMode&)cs.GetStruct(0);
    BOOL bCenter = (BOOL)cs.GetInt(1);

    returnVal = SetResolution(displayMode, bCenter);
}

void ENGINEAPI NativeGlobal_AdjustDisplayColors(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    float gamma = cs.GetFloat(0);
    float brightness = cs.GetFloat(1);
    float contrast = cs.GetFloat(2);

    returnVal = AdjustDisplayColors(gamma, brightness, contrast);
}

void ENGINEAPI NativeGlobal_EnumDisplayModes(CallStruct &cs)
{
    List<DisplayMode> &displayModes = (List<DisplayMode>&)cs.GetListOut(0);

    OSEnumDisplayModes(displayModes);
}

void ENGINEAPI NativeGlobal_GetCurrentDisplayMode(CallStruct &cs)
{
    DisplayMode &displayMode = (DisplayMode&)cs.GetStructOut(0);

    GS->GetCurrentDisplayMode(displayMode);
}

void ENGINEAPI NativeGlobal_IsFullscreen(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = GS->IsFullscreen();
}

void ENGINEAPI NativeGlobal_Set2DMode(CallStruct &cs)
{
    Set2DMode();
}

void ENGINEAPI NativeGlobal_DrawSprite(CallStruct &cs)
{
    Texture* texture = (Texture*)cs.GetObject(0);
    float x = cs.GetFloat(1);
    float y = cs.GetFloat(2);
    float x2 = cs.GetFloat(3);
    float y2 = cs.GetFloat(4);

    DrawSprite(texture, x, y, x2, y2);
}

void ENGINEAPI NativeGlobal_DrawSprite3D(CallStruct &cs)
{
    const Quat &cameraRot = (const Quat&)cs.GetStruct(0);
    Texture* texture = (Texture*)cs.GetObject(1);
    const Vect &pos = (const Vect&)cs.GetStruct(2);
    float sizeX = cs.GetFloat(3);
    float sizeY = cs.GetFloat(4);
    float rotation = cs.GetFloat(5);

    DrawSprite3D(cameraRot, texture, pos, sizeX, sizeY, rotation);
}

void ENGINEAPI NativeGlobal_DrawSpriteCenter(CallStruct &cs)
{
    Texture* texture = (Texture*)cs.GetObject(0);
    float x = cs.GetFloat(1);
    float y = cs.GetFloat(2);
    DWORD color = (DWORD)cs.GetInt(3);

    DrawSpriteCenter(texture, x, y, color);
}

void ENGINEAPI NativeGlobal_DrawCubeBackdrop(CallStruct &cs)
{
    Camera* cam = (Camera*)cs.GetObject(0);
    CubeTexture* cubetexture = (CubeTexture*)cs.GetObject(1);

    DrawCubeBackdrop(cam, cubetexture);
}

void ENGINEAPI NativeGlobal_DrawCubeBackdrop_2(CallStruct &cs)
{
    Camera* cam = (Camera*)cs.GetObject(0);
    CubeTexture* cubetexture = (CubeTexture*)cs.GetObject(1);
    const Quat &customRot = (const Quat&)cs.GetStruct(2);

    DrawCubeBackdrop(cam, cubetexture, customRot);
}

void ENGINEAPI NativeGlobal_DrawSpriteEx(CallStruct &cs)
{
    Texture* texture = (Texture*)cs.GetObject(0);
    DWORD color = (DWORD)cs.GetInt(1);
    float x = cs.GetFloat(2);
    float y = cs.GetFloat(3);
    float x2 = cs.GetFloat(4);
    float y2 = cs.GetFloat(5);
    float u = cs.GetFloat(6);
    float v = cs.GetFloat(7);
    float u2 = cs.GetFloat(8);
    float v2 = cs.GetFloat(9);

    DrawSpriteEx(texture, color, x, y, x2, y2, u, v, u2, v2);
}

void ENGINEAPI NativeGlobal_GetMouseOver(CallStruct &cs)
{
    ControlWindow*& returnVal = (ControlWindow*&)cs.GetObjectOut(RETURNVAL);
    int x = cs.GetInt(0);
    int y = cs.GetInt(1);

    returnVal = GS->GetMouseOver(x, y);
}

void ENGINEAPI NativeGlobal_GetLocalMousePos(CallStruct &cs)
{
    int &x = cs.GetIntOut(0);
    int &y = cs.GetIntOut(1);

    GS->GetLocalMousePos(x, y);
}

void ENGINEAPI NativeGlobal_SetLocalMousePos(CallStruct &cs)
{
    int x = cs.GetInt(0);
    int y = cs.GetInt(1);

    GS->SetLocalMousePos(x, y);
}
//</Script>
