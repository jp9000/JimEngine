/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Math Natives

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


//<Script module="Base" filedefs="Math.xscript">
void Vect2::Native_operator_Add_Vect2(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);
    const Vect2 &v = (const Vect2&)cs.GetStruct(0);

    returnVal = (*this + v);
}

void Vect2::Native_operator_Sub_Vect2(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);
    const Vect2 &v = (const Vect2&)cs.GetStruct(0);

    returnVal = (*this - v);
}

void Vect2::Native_operator_Div_Vect2(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);
    const Vect2 &v = (const Vect2&)cs.GetStruct(0);

    returnVal = (*this / v);
}

void Vect2::Native_operator_Mul_Vect2(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);
    const Vect2 &v = (const Vect2&)cs.GetStruct(0);

    returnVal = (*this * v);
}

void Vect2::Native_operator_Add_float(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = (*this + f);
}

void Vect2::Native_operator_Sub_float(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = (*this - f);
}

void Vect2::Native_operator_Div_float(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = (*this / f);
}

void Vect2::Native_operator_Mul_float(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = (*this * f);
}

void Vect2::Native_operator_Negate(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);

    returnVal = -(*this);
}

void Vect2::Native_operator_Equal_Vect2(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    const Vect2 &v = (const Vect2&)cs.GetStruct(0);

    returnVal = (*this == v);
}

void Vect2::Native_operator_NotEqual_Vect2(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    const Vect2 &v = (const Vect2&)cs.GetStruct(0);

    returnVal = (*this != v);
}

void Vect2::native_Set(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);
    float x = cs.GetFloat(0);
    float y = cs.GetFloat(1);

    returnVal = Set(x, y);
}

void Vect2::native_Norm(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);

    returnVal = Norm();
}

void Vect2::native_GetNorm(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);

    returnVal = GetNorm();
}

void Vect2::native_Len(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = Len();
}

void Vect2::native_Dist(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    const Vect2 &v = (const Vect2&)cs.GetStruct(0);

    returnVal = Dist(v);
}

void Vect::Native_operator_Add_Vect(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);

    returnVal = (*this + v);
}

void Vect::Native_operator_Sub_Vect(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);

    returnVal = (*this - v);
}

void Vect::Native_operator_Div_Vect(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);

    returnVal = (*this / v);
}

void Vect::Native_operator_Mul_Vect(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);

    returnVal = (*this * v);
}

void Vect::Native_operator_Add_float(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = (*this + f);
}

void Vect::Native_operator_Sub_float(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = (*this - f);
}

void Vect::Native_operator_Div_float(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = (*this / f);
}

void Vect::Native_operator_Mul_float(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = (*this * f);
}

void Vect::Native_operator_Negate(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = -(*this);
}

void Vect::Native_operator_Equal_Vect(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);

    returnVal = (*this == v);
}

void Vect::Native_operator_NotEqual_Vect(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);

    returnVal = (*this != v);
}

void Vect::native_Dot(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);

    returnVal = Dot(v);
}

void Vect::native_Cross(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);

    returnVal = Cross(v);
}

void Vect::native_Set(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    float a = cs.GetFloat(0);
    float b = cs.GetFloat(1);
    float c = cs.GetFloat(2);

    returnVal = Set(a, b, c);
}

void Vect::native_CloseTo(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);
    float epsilon = cs.GetFloat(1);

    returnVal = CloseTo(v, epsilon);
}

void Vect::native_DistFromPlane(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    const Plane &p = (const Plane&)cs.GetStruct(0);

    returnVal = DistFromPlane(p);
}

void Vect::native_ClampMin(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = ClampMin(f);
}

void Vect::native_ClampMin_2(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);

    returnVal = ClampMin(v);
}

void Vect::native_ClampMax(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = ClampMax(f);
}

void Vect::native_ClampMax_2(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);

    returnVal = ClampMax(v);
}

void Vect::native_Len(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = Len();
}

void Vect::native_Dist(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);

    returnVal = Dist(v);
}

void Vect::native_Norm(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = Norm();
}

void Vect::native_GetNorm(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = GetNorm();
}

void Vect::native_Abs(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = Abs();
}

void Vect::native_GetAbs(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = GetAbs();
}

void Vect::native_SetZero(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = SetZero();
}

void Vect::native_TransformPoint(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    returnVal = TransformPoint(m);
}

void Vect::native_TransformVector(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    returnVal = TransformVector(m);
}

void Vect::native_GetTransformedPoint(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    returnVal = GetTransformedPoint(m);
}

void Vect::native_GetTransformedVector(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    returnVal = GetTransformedVector(m);
}

void Vect::native_MakeFromRGB(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    DWORD colorVal = (DWORD)cs.GetInt(0);

    returnVal = MakeFromRGB(colorVal);
}

void Vect::native_GetRGB(CallStruct &cs)
{
    DWORD& returnVal = (DWORD&)cs.GetIntOut(RETURNVAL);

    returnVal = GetRGB();
}

void ENGINEAPI Vect::native_Min(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);
    float f = cs.GetFloat(1);

    returnVal = Min(v, f);
}

void ENGINEAPI Vect::native_Min_2(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect &v1 = (const Vect&)cs.GetStruct(0);
    const Vect &v2 = (const Vect&)cs.GetStruct(1);

    returnVal = Min(v1, v2);
}

void ENGINEAPI Vect::native_Max(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);
    float f = cs.GetFloat(1);

    returnVal = Max(v, f);
}

void ENGINEAPI Vect::native_Max_2(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect &v1 = (const Vect&)cs.GetStruct(0);
    const Vect &v2 = (const Vect&)cs.GetStruct(1);

    returnVal = Max(v1, v2);
}

void ENGINEAPI Vect::native_Zero(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = Zero();
}

void Vect4::Native_operator_Add_Vect4(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);
    const Vect4 &v2 = (const Vect4&)cs.GetStruct(0);

    returnVal = (*this + v2);
}

void Vect4::Native_operator_Sub_Vect4(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);
    const Vect4 &v2 = (const Vect4&)cs.GetStruct(0);

    returnVal = (*this - v2);
}

void Vect4::Native_operator_Div_Vect4(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);
    const Vect4 &v2 = (const Vect4&)cs.GetStruct(0);

    returnVal = (*this / v2);
}

void Vect4::Native_operator_Mul_Vect4(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);
    const Vect4 &v2 = (const Vect4&)cs.GetStruct(0);

    returnVal = (*this * v2);
}

void Vect4::Native_operator_Add_float(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = (*this + f);
}

void Vect4::Native_operator_Sub_float(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = (*this - f);
}

void Vect4::Native_operator_Div_float(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = (*this / f);
}

void Vect4::Native_operator_Mul_float(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = (*this * f);
}

void Vect4::Native_operator_Equal_Vect4(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    const Vect4 &v2 = (const Vect4&)cs.GetStruct(0);

    returnVal = (*this == v2);
}

void Vect4::Native_operator_NotEqual_Vect4(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    const Vect4 &v2 = (const Vect4&)cs.GetStruct(0);

    returnVal = (*this != v2);
}

void Vect4::native_CloseTo(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect4 &v = (const Vect4&)cs.GetStruct(0);
    float epsilon = cs.GetFloat(1);

    returnVal = CloseTo(v, epsilon);
}

void Vect4::native_Abs(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);

    returnVal = Abs();
}

void Vect4::native_GetAbs(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);

    returnVal = GetAbs();
}

void Vect4::native_NormXYZ(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);

    returnVal = NormXYZ();
}

void Vect4::native_GetNormXYZ(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);

    returnVal = GetNormXYZ();
}

void Vect4::native_NormFull(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);

    returnVal = NormFull();
}

void Vect4::native_GetNormFull(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);

    returnVal = GetNormFull();
}

void Vect4::native_DesaturateColor(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);

    returnVal = DesaturateColor();
}

void Vect4::native_ClampColor(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);

    returnVal = ClampColor();
}

void Vect4::native_GetDesaturatedColor(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);

    returnVal = GetDesaturatedColor();
}

void Vect4::native_GetClampedColor(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);

    returnVal = GetClampedColor();
}

void Vect4::native_MakeFromRGBA(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);
    DWORD colorVal = (DWORD)cs.GetInt(0);

    returnVal = MakeFromRGBA(colorVal);
}

void Vect4::native_GetRGBA(CallStruct &cs)
{
    DWORD& returnVal = (DWORD&)cs.GetIntOut(RETURNVAL);

    returnVal = GetRGBA();
}

void Vect4::native_SetZero(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);

    returnVal = SetZero();
}

void ENGINEAPI Vect4::native_Zero(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);

    returnVal = Zero();
}

void Vect4::native_Set(CallStruct &cs)
{
    Vect4& returnVal = (Vect4&)cs.GetStructOut(RETURNVAL);
    float x = cs.GetFloat(0);
    float y = cs.GetFloat(1);
    float z = cs.GetFloat(2);
    float w = cs.GetFloat(3);

    returnVal = Set(x, y, z, w);
}

void Quat::Native_operator_Mul_Quat(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);
    const Quat &v2 = (const Quat&)cs.GetStruct(0);

    returnVal = (*this * v2);
}

void Quat::Native_operator_Mul_AxisAngle(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);
    const AxisAngle &aa = (const AxisAngle&)cs.GetStruct(0);

    returnVal = (*this * aa);
}

void Quat::Native_operator_Negate(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);

    returnVal = -(*this);
}

void Quat::Native_operator_Assign_AxisAngle(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);
    const AxisAngle &aa = (const AxisAngle&)cs.GetStruct(0);

    returnVal = (*this = aa);
}

void Quat::native_SetIdentity(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);

    returnVal = SetIdentity();
}

void ENGINEAPI Quat::native_Identity(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);

    returnVal = Identity();
}

void Quat::native_Inv(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);

    returnVal = Inv();
}

void Quat::native_GetInv(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);

    returnVal = GetInv();
}

void Quat::native_Reverse(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);

    returnVal = Reverse();
}

void Quat::native_GetReverse(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);

    returnVal = GetReverse();
}

void Quat::native_Norm(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);

    returnVal = Norm();
}

void Quat::native_GetNorm(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);

    returnVal = GetNorm();
}

void Quat::native_Len(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = Len();
}

void Quat::native_Dist(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    const Quat &q = (const Quat&)cs.GetStruct(0);

    returnVal = Dist(q);
}

void Quat::native_MakeFromDirection(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);
    const Vect &dir = (const Vect&)cs.GetStruct(0);

    returnVal = MakeFromDirection(dir);
}

void Quat::native_SetLookDirection(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);
    const Vect &dir = (const Vect&)cs.GetStruct(0);

    returnVal = SetLookDirection(dir);
}

void ENGINEAPI Quat::native_GetLookDirection(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);
    const Vect &dir = (const Vect&)cs.GetStruct(0);

    returnVal = GetLookDirection(dir);
}

void Quat::native_CloseTo(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Quat &q = (const Quat&)cs.GetStruct(0);
    float epsilon = cs.GetFloat(1);

    returnVal = CloseTo(q, epsilon);
}

void Quat::native_GetDirectionVector(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = GetDirectionVector();
}

void Quat::native_GetInterpolationTangent(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);
    const Quat &prev = (const Quat&)cs.GetStruct(0);
    const Quat &next = (const Quat&)cs.GetStruct(1);

    returnVal = GetInterpolationTangent(prev, next);
}

void Quat::native_CreateFromMatrix(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    returnVal = CreateFromMatrix(m);
}

void Quat::native_MakeFromAxisAngle(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);
    const AxisAngle &aa = (const AxisAngle&)cs.GetStruct(0);

    returnVal = MakeFromAxisAngle(aa);
}

void Quat::native_GetAxisAngle(CallStruct &cs)
{
    AxisAngle& returnVal = (AxisAngle&)cs.GetStructOut(RETURNVAL);

    returnVal = GetAxisAngle();
}

void AxisAngle::Native_operator_Assign_Quat(CallStruct &cs)
{
    AxisAngle& returnVal = (AxisAngle&)cs.GetStructOut(RETURNVAL);
    const Quat &v2 = (const Quat&)cs.GetStruct(0);

    returnVal = (*this = v2);
}

void AxisAngle::native_CloseTo(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const AxisAngle &aa2 = (const AxisAngle&)cs.GetStruct(0);
    float epsilon = cs.GetFloat(1);

    returnVal = CloseTo(aa2, epsilon);
}

void AxisAngle::native_Set(CallStruct &cs)
{
    AxisAngle& returnVal = (AxisAngle&)cs.GetStructOut(RETURNVAL);
    float x = cs.GetFloat(0);
    float y = cs.GetFloat(1);
    float z = cs.GetFloat(2);
    float a = cs.GetFloat(3);

    returnVal = Set(x, y, z, a);
}

void AxisAngle::native_Clear(CallStruct &cs)
{
    AxisAngle& returnVal = (AxisAngle&)cs.GetStructOut(RETURNVAL);

    returnVal = Clear();
}

void AxisAngle::native_MakeFromQuat(CallStruct &cs)
{
    AxisAngle& returnVal = (AxisAngle&)cs.GetStructOut(RETURNVAL);
    const Quat &q = (const Quat&)cs.GetStruct(0);

    returnVal = MakeFromQuat(q);
}

void AxisAngle::native_GetQuat(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);

    returnVal = GetQuat();
}

void Bounds::native_GetPoint(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    int i = cs.GetInt(0);

    returnVal = GetPoint(i);
}

void Bounds::native_GetTransformedBounds(CallStruct &cs)
{
    Bounds& returnVal = (Bounds&)cs.GetStructOut(RETURNVAL);
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    returnVal = GetTransformedBounds(m);
}

void Bounds::native_Transform(CallStruct &cs)
{
    Bounds& returnVal = (Bounds&)cs.GetStructOut(RETURNVAL);
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    returnVal = Transform(m);
}

void Bounds::native_GetCenter(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = GetCenter();
}

void Bounds::native_Intersects(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Bounds &test = (const Bounds&)cs.GetStruct(0);
    float epsilon = cs.GetFloat(1);

    returnVal = Intersects(test, epsilon);
}

void Bounds::native_PointInside(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &point = (const Vect&)cs.GetStruct(0);

    returnVal = PointInside(point);
}

void Bounds::native_GetDiamater(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetDiamater();
}

void Bounds::native_RayIntersection(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &rayOrig = (const Vect&)cs.GetStruct(0);
    const Vect &rayDir = (const Vect&)cs.GetStruct(1);
    float &fT = cs.GetFloatOut(2);

    returnVal = RayIntersection(rayOrig, rayDir, fT);
}

void Bounds::native_LineIntersection(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &v1 = (const Vect&)cs.GetStruct(0);
    const Vect &v2 = (const Vect&)cs.GetStruct(1);
    float &fT = cs.GetFloatOut(2);

    returnVal = LineIntersection(v1, v2, fT);
}

void Bounds::native_RayIntersects(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &rayOrig = (const Vect&)cs.GetStruct(0);
    const Vect &rayDir = (const Vect&)cs.GetStruct(1);

    returnVal = RayIntersects(rayOrig, rayDir);
}

void Bounds::native_LineIntersects(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &v1 = (const Vect&)cs.GetStruct(0);
    const Vect &v2 = (const Vect&)cs.GetStruct(1);

    returnVal = LineIntersects(v1, v2);
}

void Bounds::native_PlaneTest(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    const Plane &p = (const Plane&)cs.GetStruct(0);

    returnVal = PlaneTest(p);
}

void Plane::native_Transform(CallStruct &cs)
{
    Plane& returnVal = (Plane&)cs.GetStructOut(RETURNVAL);
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    returnVal = Transform(m);
}

void Plane::native_GetTransform(CallStruct &cs)
{
    Plane& returnVal = (Plane&)cs.GetStructOut(RETURNVAL);
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    returnVal = GetTransform(m);
}

void Plane::native_GetRayIntersection(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &rayOrigin = (const Vect&)cs.GetStruct(0);
    const Vect &rayDir = (const Vect&)cs.GetStruct(1);
    float &fT = cs.GetFloatOut(2);

    returnVal = GetRayIntersection(rayOrigin, rayDir, fT);
}

void Plane::native_GetIntersection(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &p1 = (const Vect&)cs.GetStruct(0);
    const Vect &p2 = (const Vect&)cs.GetStruct(1);
    float &fT = cs.GetFloatOut(2);

    returnVal = GetIntersection(p1, p2, fT);
}

void Matrix::Native_operator_Assign_Quat(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);
    const Quat &v = (const Quat&)cs.GetStruct(0);

    returnVal = (*this = v);
}

void Matrix::Native_operator_Mul_Matrix(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);
    const Matrix &m = (const Matrix&)cs.GetStruct(0);

    returnVal = (*this * m);
}

void Matrix::Native_operator_Mul_Vect(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);

    returnVal = (*this * v);
}

void Matrix::Native_operator_Mul_Quat(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);
    const Quat &v = (const Quat&)cs.GetStruct(0);

    returnVal = (*this * v);
}

void Matrix::Native_operator_Mul_AxisAngle(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);
    const AxisAngle &v = (const AxisAngle&)cs.GetStruct(0);

    returnVal = (*this * v);
}

void Matrix::native_SetIdentity(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);

    returnVal = SetIdentity();
}

void ENGINEAPI Matrix::native_Identity(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);

    returnVal = Identity();
}

void Matrix::native_Translate(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);
    const Vect &v = (const Vect&)cs.GetStruct(0);

    returnVal = Translate(v);
}

void Matrix::native_Rotate(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);
    const Quat &v = (const Quat&)cs.GetStruct(0);

    returnVal = Rotate(v);
}

void Matrix::native_Rotate_2(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);
    const AxisAngle &v = (const AxisAngle&)cs.GetStruct(0);

    returnVal = Rotate(v);
}

void Matrix::native_Transpose(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);

    returnVal = Transpose();
}

void Matrix::native_GetTranspose(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);

    returnVal = GetTranspose();
}

void ENGINEAPI NativeGlobal_DEG(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float fRadians = cs.GetFloat(0);

    returnVal = DEG(fRadians);
}

void ENGINEAPI NativeGlobal_RAD(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float fDegrees = cs.GetFloat(0);

    returnVal = RAD(fDegrees);
}

void ENGINEAPI NativeGlobal_acos(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = acos(f);
}

void ENGINEAPI NativeGlobal_asin(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = asin(f);
}

void ENGINEAPI NativeGlobal_atan(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = atan(f);
}

void ENGINEAPI NativeGlobal_atan2(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f1 = cs.GetFloat(0);
    float f2 = cs.GetFloat(1);

    returnVal = atan2(f1, f2);
}

void ENGINEAPI NativeGlobal_cos(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = cos(f);
}

void ENGINEAPI NativeGlobal_sin(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = sin(f);
}

void ENGINEAPI NativeGlobal_tan(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = tan(f);
}

void ENGINEAPI NativeGlobal_tanh(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = tanh(f);
}

void ENGINEAPI NativeGlobal_atof(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    String strFloat = cs.GetString(0);

    returnVal = strFloat.ToFloat();
}

void ENGINEAPI NativeGlobal_atoi(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    String strInt = cs.GetString(0);

    returnVal = strInt.ToInt();
}

void ENGINEAPI NativeGlobal_finite(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = _finite(f);
}

void ENGINEAPI NativeGlobal_fabs(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = fabs(f);
}

void ENGINEAPI NativeGlobal_floor(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = floor(f);
}

void ENGINEAPI NativeGlobal_ceil(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = ceil(f);
}

void ENGINEAPI NativeGlobal_fmod(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f1 = cs.GetFloat(0);
    float f2 = cs.GetFloat(1);

    returnVal = fmod(f1, f2);
}

void ENGINEAPI NativeGlobal_MIN(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f1 = cs.GetFloat(0);
    float f2 = cs.GetFloat(1);

    returnVal = MIN(f1, f2);
}

void ENGINEAPI NativeGlobal_MAX(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f1 = cs.GetFloat(0);
    float f2 = cs.GetFloat(1);

    returnVal = MAX(f1, f2);
}

void ENGINEAPI NativeGlobal_MIN_2(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    int i1 = cs.GetInt(0);
    int i2 = cs.GetInt(1);

    returnVal = MIN(i1, i2);
}

void ENGINEAPI NativeGlobal_MAX_2(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    int i1 = cs.GetInt(0);
    int i2 = cs.GetInt(1);

    returnVal = MAX(i1, i2);
}

void ENGINEAPI NativeGlobal_pow(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f = cs.GetFloat(0);
    float f2 = cs.GetFloat(1);

    returnVal = pow(f, f2);
}

void ENGINEAPI NativeGlobal_rand(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    int maxint = cs.GetInt(0);

    returnVal = rand()%maxint;
}

void ENGINEAPI NativeGlobal_sqrt(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = sqrt(f);
}

void ENGINEAPI NativeGlobal_CloseFloat(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    float f1 = cs.GetFloat(0);
    float f2 = cs.GetFloat(1);
    float epsilon = cs.GetFloat(2);

    returnVal = CloseFloat(f1, f2, epsilon);
}

void ENGINEAPI NativeGlobal_FloatString(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    float f = cs.GetFloat(0);

    returnVal = FloatString(f);
}

void ENGINEAPI NativeGlobal_FormattedFloat(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    String format = cs.GetString(0);
    float f = cs.GetFloat(1);

    TSTR lpTemp = format;
    int specifierCount = 0;

    while(lpTemp = schr(lpTemp, '%'))
    {
        if(lpTemp[1] == '%')
            lpTemp += 2;
        else
        {
            ++specifierCount;
            if(specifierCount > 1)
                break;

            ++lpTemp;
        }
    }

    if(specifierCount != 1)
    {
        AppWarning(TEXT("FormattedFloat: Error in format: \"%s\""), format.Array());
        return;
    }

    returnVal = FormattedString(format, f);
}

void ENGINEAPI NativeGlobal_IntString(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    int i = cs.GetInt(0);
    int radix = cs.GetInt(1);

    returnVal = IntString(i, radix);
}

void ENGINEAPI NativeGlobal_UIntString(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    int ui = cs.GetInt(0);
    int radix = cs.GetInt(1);

    returnVal = UIntString(ui, radix);
}

void ENGINEAPI NativeGlobal_LerpFloat(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    float f1 = cs.GetFloat(0);
    float f2 = cs.GetFloat(1);
    float t = cs.GetFloat(2);

    returnVal = Lerp<float>(f1, f2, t);
}

void ENGINEAPI NativeGlobal_LerpVect2(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect2 &v1 = (const Vect2&)cs.GetStruct(0);
    const Vect2 &v2 = (const Vect2&)cs.GetStruct(1);
    float t = cs.GetFloat(2);

    returnVal = Lerp<Vect2>(v1, v2, t);
}

void ENGINEAPI NativeGlobal_LerpVect(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect &v1 = (const Vect&)cs.GetStruct(0);
    const Vect &v2 = (const Vect&)cs.GetStruct(1);
    float t = cs.GetFloat(2);

    returnVal = Lerp<Vect>(v1, v2, t);
}

void ENGINEAPI NativeGlobal_InterpolateQuat(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);
    const Quat &from = (const Quat&)cs.GetStruct(0);
    const Quat &to = (const Quat&)cs.GetStruct(1);
    float t = cs.GetFloat(2);

    returnVal = InterpolateQuat(from, to, t);
}

void ENGINEAPI NativeGlobal_CubicInterpolateQuat(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);
    const Quat &from = (const Quat&)cs.GetStruct(0);
    const Quat &to = (const Quat&)cs.GetStruct(1);
    const Quat &qTan1 = (const Quat&)cs.GetStruct(2);
    const Quat &qTan2 = (const Quat&)cs.GetStruct(3);
    float t = cs.GetFloat(4);

    returnVal = CubicInterpolateQuat(from, to, qTan1, qTan2, t);
}

void ENGINEAPI NativeGlobal_RandomFloat(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    BOOL bPositiveOnly = (BOOL)cs.GetInt(0);

    returnVal = RandomFloat(bPositiveOnly);
}

void ENGINEAPI NativeGlobal_RandomVect(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    BOOL bPositiveOnly = (BOOL)cs.GetInt(0);

    returnVal = RandomVect(bPositiveOnly);
}

void ENGINEAPI NativeGlobal_GetHSpline(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);
    const Vect &from = (const Vect&)cs.GetStruct(0);
    const Vect &to = (const Vect&)cs.GetStruct(1);
    const Vect &vTan1 = (const Vect&)cs.GetStruct(2);
    const Vect &vTan2 = (const Vect&)cs.GetStruct(3);
    float fT = cs.GetFloat(4);

    returnVal = GetHSpline(from, to, vTan1, vTan2, fT);
}

void ENGINEAPI NativeGlobal_SphereRayCollision(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &sphereCenter = (const Vect&)cs.GetStruct(0);
    float sphereRadius = cs.GetFloat(1);
    const Vect &rayOrig = (const Vect&)cs.GetStruct(2);
    const Vect &rayDir = (const Vect&)cs.GetStruct(3);
    Vect &collision = (Vect&)cs.GetStructOut(4);
    Plane &collisionPlane = (Plane&)cs.GetStructOut(5);

    returnVal = SphereRayCollision(sphereCenter, sphereRadius, rayOrig, rayDir, &collision, &collisionPlane);
}

void ENGINEAPI NativeGlobal_CylinderRayCollision(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &cylCenter = (const Vect&)cs.GetStruct(0);
    float cylRadius = cs.GetFloat(1);
    float cylHeight = cs.GetFloat(2);
    const Vect &rayOrig = (const Vect&)cs.GetStruct(3);
    const Vect &rayDir = (const Vect&)cs.GetStruct(4);
    Vect &collision = (Vect&)cs.GetStructOut(5);
    Plane &collisionPlane = (Plane&)cs.GetStructOut(6);

    returnVal = CylinderRayCollision(cylCenter, cylRadius, cylHeight, rayOrig, rayDir, &collision, &collisionPlane);
}

void ENGINEAPI NativeGlobal_CalcTorque(CallStruct &cs)
{
    float &val1 = cs.GetFloatOut(0);
    float val2 = cs.GetFloat(1);
    float torque = cs.GetFloat(2);
    float minAdjust = cs.GetFloat(3);
    float fT = cs.GetFloat(4);

    CalcTorque(val1, val2, torque, minAdjust, fT);
}

void ENGINEAPI NativeGlobal_CalcTorque_2(CallStruct &cs)
{
    Vect &val1 = (Vect&)cs.GetStructOut(0);
    const Vect &val2 = (const Vect&)cs.GetStruct(1);
    float torque = cs.GetFloat(2);
    float minAdjust = cs.GetFloat(3);
    float fT = cs.GetFloat(4);

    CalcTorque(val1, val2, torque, minAdjust, fT);
}
//</Script>
