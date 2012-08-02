//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Default Terrain Shader
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//=================================================================
//  Parameters
//=================================================================

float4x4 WorldView       : WorldViewProj;

float3   lightPos        : LightPosition;
float3   lightDir        : LightDirection;
float3   eyePos          : EyePosition;
float    lightRange      : LightRange;

float3   ambientColor    : AmbientColor;

float3   lightColor      : LightColor;
texture  attenuationMap  : AttenuationMap;

texture  spotlightMap    : SpotlightMap;
float4x4 spotlightMatrix : SpotlightMatrix;

texture  fogTexture      : FogTable;
float    fogDist         : FogDist;

float3   blockPos        : BlockPos;
float3   blockSize       : BlockSize;

float3   map1X           : map1X;
float3   map1Y           : map1Y;
float3   map2X           : map2X;
float3   map2Y           : map2Y;

float2   map1Scale       : map1Scale;
float2   map2Scale       : map2Scale;

texture  mixMap          : MixMap;

texture diffuseTexture1  : MainTexture1
      <string Name = "Texture1";
       string Type = "Texture";
       string DefaultValue = "";>;

texture diffuseTexture2  : MainTexture2
      <string Name = "Texture2";
       string Type = "Texture";
       string DefaultValue = "";>;

float specularLevel1
      <string Name = "Specular Level 1";
       string Type = "FloatScroller";
       float Min = 0.0f;
       float Max = 100.0f;
       float Precision = 1.0f;
       float MultiplyOutput = 0.01f;
       float DefaultValue = 0.0f;>;

float specularLevel2
      <string Name = "Specular Level 2";
       string Type = "FloatScroller";
       float Min = 0.0f;
       float Max = 100.0f;
       float Precision = 1.0f;
       float MultiplyOutput = 0.01f;
       float DefaultValue = 0.0f;>;

float specularPower1
      <string Name = "Specular Power 1";
       string Type = "FloatScroller";
       float Min = 8.0f;
       float Max = 64.0f;
       float Precision = 1.0f;
       float DefaultValue = 10.0f;>;

float specularPower2
      <string Name = "Specular Power 2";
       string Type = "FloatScroller";
       float Min = 8.0f;
       float Max = 64.0f;
       float Precision = 1.0f;
       float DefaultValue = 10.0f;>;

float3 illuminationColor1
       <string Name = "Illumination1";
        string Type = "Color";
        float3 DefaultValue = {0.0f, 0.0f, 0.0f};>;

float3 illuminationColor2
       <string Name = "Illumination2";
        string Type = "Color";
        float3 DefaultValue = {0.0f, 0.0f, 0.0f};>;

//=================================================================
//  Vertex Input
//=================================================================

struct Vertex {
    float4 Position : POSITION;
    float3 Tangent  : TANGENT;
    float3 Normal   : NORMAL;
};

struct PixelOutput {
    float4 color : COLOR;
};

//=================================================================
//  Lighting Function
//=================================================================

float4 DoLighting(float2 TexCoord,
                  float3 LightVector,   //these two need to be normalized before-hand
                  float3 HalfVector,
                  uniform sampler2D DiffuseMap,
                  uniform sampler2D NormalMap,
                  float3 lightColor,
                  float specularLevel,
                  float specularPower)
{
    float4 output;

    float4 diffuse      = tex2D(DiffuseMap, TexCoord);
    float4 normalIn     = tex2D(NormalMap, TexCoord);

    float3 normal = 2.0*(normalIn.xyz-0.5);

    float3 shade = dot(LightVector, normal);

    float3 specular = dot(HalfVector, normal);
    specular  = pow(specular, specularPower);
    specular  = saturate(specular*specularLevel);

    output.xyz = ((diffuse.xyz*shade)+specular)*lightColor;
    output.a = diffuse.a;

    return output;
}


//=================================================================
//  Initial Pass
//=================================================================

struct InitialPassData {
    float4 Position    : POSITION;
    float2 TexCoord1   : TEXCOORD0;
    float2 TexCoord2   : TEXCOORD1;
    float3 FogCoord    : TEXCOORD2;
    float2 MixCoord    : TEXCOORD3;
};

InitialPassData InitialPassVS(Vertex input,
                              uniform float4x4 WorldView,
                              uniform float fogDistInv,
                              uniform float blockSizeInv,
                              uniform float3 blockPos,
                              uniform float3 eyePos,
                              uniform float3 map1X,
                              uniform float3 map1Y,
                              uniform float3 map2X,
                              uniform float3 map2Y,
                              uniform float2 map1Scale,
                              uniform float2 map2Scale)
{
    InitialPassData PI;

    PI.TexCoord1.x = dot(map1X, input.Position);
    PI.TexCoord1.y = dot(map1Y, input.Position);
    PI.TexCoord1 *= map1Scale*0.0625f;

    PI.TexCoord2.x = dot(map2X, input.Position);
    PI.TexCoord2.y = dot(map2Y, input.Position);
    PI.TexCoord2 *= map2Scale*0.0625f;

    PI.MixCoord  = input.Position.xz*blockSizeInv;

    PI.FogCoord  = (eyePos-input.Position)*fogDistInv;
    PI.Position  = mul(input.Position, WorldView);

    return PI;
}

PixelOutput InitialPass1TexPS(InitialPassData PI,
                              uniform sampler2D DiffuseMap1,
                              uniform sampler2D mixMap,
                              uniform sampler1D FogTable,
                              uniform float3 AmbientColor,
                              uniform float3 IlluminationColor)
{
    PixelOutput PO;

    float4 diffuse = tex2D(DiffuseMap1, PI.TexCoord1);
    float fog = tex1D(FogTable, length(PI.FogCoord)).r;
 
    float4 mixVal = tex2D(mixMap, PI.MixCoord);

    float3 tex1Val = diffuse.xyz + (diffuse.xyz*IlluminationColor);

    PO.color.xyz = (tex1Val*mixVal.r)*AmbientColor;
    PO.color.a = fog;

    return PO;
}

PixelOutput InitialPass2TexPS(InitialPassData PI,
                              uniform sampler2D DiffuseMap1,
                              uniform sampler2D DiffuseMap2,
                              uniform sampler2D mixMap,
                              uniform sampler1D FogTable,
                              uniform float3 AmbientColor,
                              uniform float3 IlluminationColor1,
                              uniform float3 IlluminationColor2)
{
    PixelOutput PO;

    float fog = tex1D(FogTable, length(PI.FogCoord)).r;
    clip(fog-0.003f);
    
    float4 mixVal = tex2D(mixMap, PI.MixCoord);

    float4 diffuse1 = tex2D(DiffuseMap1, PI.TexCoord1);
    float4 diffuse2 = tex2D(DiffuseMap2, PI.TexCoord2);

    float3 tex1Val = diffuse1.xyz + (diffuse1.xyz*IlluminationColor1);
    float3 tex2Val = diffuse2.xyz + (diffuse2.xyz*IlluminationColor2);

    PO.color.xyz = ((tex1Val*mixVal.r)+(tex2Val*mixVal.a))*AmbientColor;
    PO.color.a = fog;

    return PO;
}

//=================================================================
//  PointLight
//=================================================================

/*struct PointLightData {
    float4 Position    : POSITION;
    float2 TexCoord    : TEXCOORD0;
    float3 LightVector : TEXCOORD1;
    float3 HalfVector  : TEXCOORD2;
    float3 Attenuation : TEXCOORD3;
};

PointLightData PointLightVS20(Vertex input,
                              uniform float4x4 WorldView,
                              uniform float3 lightPos,
                              uniform float3 eyePos,
                              uniform float3 lightRangeI)
{
    PointLightData PI;

    float3x3 TangentMatrix;
    TangentMatrix[0]    = -input.Tangent;
    TangentMatrix[2]    = input.Normal;
    TangentMatrix[1]    = cross(TangentMatrix[0], TangentMatrix[2]);

    float3 lightVector  = lightPos-input.Position;
    float3 lightVectorT = mul(TangentMatrix, lightVector);
    float3 eyeVectorT   = mul(TangentMatrix, eyePos-input.Position);

    PI.Attenuation      = ((lightVector*lightRangeI)*0.5)+0.5;
    PI.LightVector      = lightVectorT;
    PI.HalfVector       = eyeVectorT+lightVectorT;
    PI.TexCoord         = input.TexCoord;
    PI.Position         = mul(input.Position, WorldView);

    return PI;
}

PixelOutput PointLightPS20(PointLightData PI,
                           uniform sampler2D DiffuseMap,
                           uniform sampler2D NormalMap,
                           uniform sampler2D SpecularMap,
                           uniform sampler3D AttenuationMap,
                           uniform float3 lightColor,
                           uniform float specularLevel,
                           uniform float specularPower)
{
    PixelOutput PO;

    float3 attenuation  = tex3D(AttenuationMap, PI.Attenuation);
    clip(attenuation-0.003);

    float4 diffuse      = tex2D(DiffuseMap, PI.TexCoord);
    float4 normalIn     = tex2D(NormalMap, PI.TexCoord);
    float3 specularIn   = tex2D(SpecularMap, PI.TexCoord);

    float3 normal = 2.0*(normalIn.xyz-0.5);

    float3 shade = dot(normalize(PI.LightVector), normal);

    float3 specular = dot(normalize(PI.HalfVector), normal);
    specular  = pow(specular, specularPower);
    specular *= specularIn.xyz;
    specular = saturate(specular*specularLevel);

    PO.color.xyz = ((diffuse.xyz*shade)+specular)*attenuation*lightColor;
    PO.color.a = diffuse.a;

    return PO;
}

//=================================================================
//  DirectionalLight
//=================================================================

struct DirectionalLightData {
    float4 Position    : POSITION;
    float2 TexCoord    : TEXCOORD0;
    float3 LightVector : TEXCOORD1;
    float3 HalfVector  : TEXCOORD2;
};

DirectionalLightData DirectionalLightVS20(Vertex input,
                                          uniform float4x4 WorldView,
                                          uniform float3 lightDir,
                                          uniform float3 eyePos)
{
    DirectionalLightData PI;

    float3x3 TangentMatrix;
    TangentMatrix[0]    = -input.Tangent;
    TangentMatrix[2]    = input.Normal;
    TangentMatrix[1]    = cross(TangentMatrix[0], TangentMatrix[2]);

    float3 lightVectorT = mul(TangentMatrix, lightDir);
    float3 eyeVectorT   = normalize(mul(TangentMatrix, eyePos-input.Position));

    PI.LightVector      = lightVectorT;
    PI.HalfVector       = eyeVectorT+lightVectorT;
    PI.TexCoord         = input.TexCoord;
    PI.Position         = mul(input.Position, WorldView);

    return PI;
}

PixelOutput DirectionalLightPS20(DirectionalLightData PI,
                                 uniform sampler2D DiffuseMap,
                                 uniform sampler2D NormalMap,
                                 uniform sampler2D SpecularMap,
                                 uniform float3 lightColor,
                                 uniform float specularLevel,
                                 uniform float specularPower)
{
    PixelOutput PO;

    float4 diffuse      = tex2D(DiffuseMap, PI.TexCoord);
    float4 normalIn     = tex2D(NormalMap, PI.TexCoord);
    float3 specularIn   = tex2D(SpecularMap, PI.TexCoord);

    float3 normal = 2.0*(normalIn.xyz-0.5);

    float3 shade = dot(normalize(PI.LightVector), normal);

    float3 specular = dot(normalize(PI.HalfVector), normal);
    specular  = pow(specular, specularPower);
    specular *= specularIn.xyz;
    specular = saturate(specular*specularLevel);

    PO.color.xyz = ((diffuse.xyz*shade)+specular)*lightColor;
    PO.color.a = diffuse.a;

    return PO;
}

//=================================================================
//  SpotLight
//=================================================================

struct SpotLightData {
    float4 Position    : POSITION;
    float2 TexCoord    : TEXCOORD0;
    float3 LightVector : TEXCOORD1;
    float3 HalfVector  : TEXCOORD2;
    float3 Attenuation : TEXCOORD3;
    float4 SpotCoords  : TEXCOORD4;
};

SpotLightData SpotLightVS20(Vertex input,
                            uniform float4x4 WorldView,
                            uniform float4x4 spotlightMatrix,
                            uniform float3 lightPos,
                            uniform float3 eyePos,
                            uniform float3 lightRangeI)
{
    SpotLightData PI;

    float3x3 TangentMatrix;
    TangentMatrix[0]    = -input.Tangent;
    TangentMatrix[2]    = input.Normal;
    TangentMatrix[1]    = cross(TangentMatrix[0], TangentMatrix[2]);

    float3 lightVector  = lightPos-input.Position;
    float3 lightVectorT = mul(TangentMatrix, lightVector);
    float3 eyeVectorT   = mul(TangentMatrix, eyePos-input.Position);

    PI.Attenuation      = ((lightVector*lightRangeI)*0.5)+0.5;
    PI.LightVector      = lightVectorT;
    PI.HalfVector       = eyeVectorT+lightVectorT;
    PI.TexCoord         = input.TexCoord;
    PI.Position         = mul(input.Position, WorldView);
    PI.SpotCoords       = mul(spotlightMatrix, input.Position);

    return PI;
}

PixelOutput SpotLightPS20(SpotLightData PI,
                          uniform sampler2D DiffuseMap,
                          uniform sampler2D NormalMap,
                          uniform sampler2D SpecularMap,
                          uniform sampler2D SpotlightMap,
                          uniform sampler3D AttenuationMap,
                          uniform float3 lightColor,
                          uniform float specularLevel,
                          uniform float specularPower)
{
    PixelOutput PO;

    float3 attenuation  = tex3D(AttenuationMap, PI.Attenuation);
    float3 spotMask     = tex2Dproj(SpotlightMap, PI.SpotCoords);

    attenuation *= spotMask;
    attenuation = (PI.SpotCoords.w >= 0.0f) ? attenuation : 0.0f;

    clip(attenuation-0.003);

    float4 diffuse      = tex2D(DiffuseMap, PI.TexCoord);
    float4 normalIn     = tex2D(NormalMap, PI.TexCoord);
    float3 specularIn   = tex2D(SpecularMap, PI.TexCoord);

    float3 normal = 2.0*(normalIn.xyz-0.5);

    float3 shade = dot(normalize(PI.LightVector), normal);

    float3 specular = dot(normalize(PI.HalfVector), normal);
    specular  = pow(specular, specularPower);
    specular *= specularIn.xyz;
    specular = saturate(specular*specularLevel);

    PO.color.xyz = ((diffuse.xyz*shade)+specular)*attenuation*lightColor;
    PO.color.a = diffuse.a;

    return PO;
}

//=================================================================
//  Lightmap
//=================================================================

struct LightmapVertex
{
    float4 Position      : POSITION;
    float2 TexCoord      : TEXCOORD0;
    float2 LightmapCoord : TEXCOORD1;
//    float3 Tangent       : TANGENT;
//    float3 Normal        : NORMAL;
};

struct LightmapData
{
    float4 Position      : POSITION;
    float2 TexCoord      : TEXCOORD0;
    float2 LightmapCoord : TEXCOORD1;
};

LightmapData LightmapVS20(LightmapVertex input,
                          uniform float4x4 WorldView)
{
    LightmapData PI;
    
    PI.Position      = mul(input.Position, WorldView);
    PI.TexCoord      = input.TexCoord;
    PI.LightmapCoord = input.LightmapCoord;
    
    return PI;
}

PixelOutput LightmapPS20(LightmapData PI,
                         uniform sampler2D DiffuseMap,
                         uniform sampler2D NormalMap,
                         uniform sampler2D LightmapU,
                         uniform sampler2D LightmapV,
                         uniform sampler2D LightmapW,
                         uniform float3 AmbientColor,
                         uniform float3 IlluminationColor)
{
    PixelOutput PO;

    const float3 UVec = {-0.70710678f, -0.40824829f, 0.57735026f};
    const float3 VVec = { 0.70710678f, -0.40824829f, 0.57735026f};
    const float3 WVec = {        0.0f,  0.81649658f, 0.57735026f};
    
    float3 diffuse = tex2D(DiffuseMap, PI.TexCoord);
    float3 normal  = tex2D(NormalMap, PI.TexCoord);
    float3 LU      = tex2D(LightmapU, PI.LightmapCoord);
    float3 LV      = tex2D(LightmapV, PI.LightmapCoord);
    float3 LW      = tex2D(LightmapW, PI.LightmapCoord);
    
    float3 shade = (LU*(dot(normal, UVec)))+
                   (LV*(dot(normal, VVec)))+
                   (LW*(dot(normal, WVec)));

    PO.color.xyz = (diffuse*shade)+(diffuse*AmbientColor)+(diffuse*IlluminationColor);
    PO.color.a = 1.0f;

    return PO;
}*/

//=================================================================
//  Samplers
//=================================================================

sampler2D diffuseSampler1 = sampler_state
{
    Texture   = <diffuseTexture1>;
    ADDRESSU  = WRAP;
    ADDRESSV  = WRAP;
    MAGFILTER = LINEAR;
    MINFILTER = ANISOTROPIC;
    MIPFILTER = LINEAR;
};

sampler2D diffuseSampler2 = sampler_state
{
    Texture   = <diffuseTexture2>;
    ADDRESSU  = WRAP;
    ADDRESSV  = WRAP;
    MAGFILTER = LINEAR;
    MINFILTER = ANISOTROPIC;
    MIPFILTER = LINEAR;
};

sampler2D mixSampler = sampler_state
{
    Texture   = <mixMap>;
    ADDRESSU  = CLAMP;
    ADDRESSV  = CLAMP;
    MAGFILTER = LINEAR;
    MINFILTER = ANISOTROPIC;
    MIPFILTER = LINEAR;
};

sampler1D fogTableSampler = sampler_state
{
    Texture   = <fogTexture>;
    ADDRESSU  = CLAMP;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    MIPFILTER = NONE;
};

sampler3D attenuationSampler = sampler_state 
{
    Texture   = <attenuationMap>;
    ADDRESSU  = CLAMP;
    ADDRESSV  = CLAMP;
    ADDRESSW  = CLAMP;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    MIPFILTER = NONE;
};

sampler2D spotlightSampler = sampler_state 
{
    Texture   = <spotlightMap>;
    ADDRESSU  = CLAMP;
    ADDRESSV  = CLAMP;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    MIPFILTER = NONE;
};


//=================================================================
//  techniques
//=================================================================

technique InitialPass1Tex20
{
    pass p0
    {
        VertexShader = compile vs_2_0 InitialPassVS(WorldView, 1.0f/fogDist, 1.0f/blockSize, blockPos, eyePos, map1X, map1Y, map2X, map2Y, map1Scale, map2Scale);
        PixelShader  = compile ps_2_0 InitialPass1TexPS(diffuseSampler1, mixSampler, fogTableSampler, ambientColor, illuminationColor1);
    }
}

technique InitialPass2Tex20
{
    pass p0
    {
        VertexShader = compile vs_2_0 InitialPassVS(WorldView, 1.0f/fogDist, 1.0f/blockSize, blockPos, eyePos, map1X, map1Y, map2X, map2Y, map1Scale, map2Scale);
        PixelShader  = compile ps_2_0 InitialPass2TexPS(diffuseSampler1, diffuseSampler2, mixSampler, fogTableSampler, ambientColor, illuminationColor1, illuminationColor2);
    }
}

/*technique DirectionalLight20
{
    pass p0
    {
        VertexShader = compile vs_2_0 DirectionalLightVS20(WorldView, lightDir, eyePos);
        PixelShader  = compile ps_2_0 DirectionalLightPS20(diffuseSampler, normalSampler, specularSampler, lightColor, specularLevel, specularPower);
    }
}

technique PointLight20
{
    pass p0
    {
        VertexShader = compile vs_2_0 PointLightVS20(WorldView, lightPos, eyePos, lightRangeI);
        PixelShader  = compile ps_2_0 PointLightPS20(diffuseSampler, normalSampler, specularSampler, attenuationSampler, lightColor, specularLevel, specularPower);
    }
}

technique SpotLight20
{
    pass p0
    {
        VertexShader = compile vs_2_0 SpotLightVS20(WorldView, spotlightMatrix, lightPos, eyePos, lightRangeI);
        PixelShader  = compile ps_2_0 SpotLightPS20(diffuseSampler, normalSampler, specularSampler, spotlightSampler, attenuationSampler, lightColor, specularLevel, specularPower);
    }
}

technique Lightmap20
{
    pass p0
    {
        VertexShader = compile vs_2_0 LightmapVS20(WorldView);
        PixelShader  = compile ps_2_0 LightmapPS20(diffuseSampler, normalSampler, lightmapUSampler, lightmapVSampler, lightmapWSampler, ambientColor, illuminationColor);
    }
}*/
