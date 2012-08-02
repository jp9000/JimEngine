
#include "ShadowFunctions.inc"


//=================================================================
//  Internal Parameters
//=================================================================

uniform float4x4    ViewProj;
uniform float4x4    World =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

uniform float3      Scale = {1.0f, 1.0f, 1.0f};

uniform float3      eyePos;

uniform float3      lightPos;
uniform float3      lightDir;
uniform float       lightRange;

uniform float3      ambientColor = {0.0f, 0.0f, 0.0f};
uniform float3      lightColor;

uniform float4x4    boneTransforms[4];

uniform texture2d   spotlightMap;
uniform float4x4    spotlightMatrix;

uniform texture2d   lightmapU;
uniform texture2d   lightmapV;
uniform texture2d   lightmapW;

uniform texture1d   attenuationMap;


//=================================================================
//  Property Parameters
//=================================================================

property uniform texture2d diffuseTexture
      <name = "Color Texture">;

property uniform texture2d normalMap = "Base:default/nobump.tga"
      <name = "Normal Map">;

property uniform texture2d specularMap = "Base:default/white.tga"
      <name = "Specular Map">;

property uniform texture2d illuminationMap = "Base:default/white.tga"
      <name = "Illumination Map">;

property uniform float specularLevel = 0.0f
      <name = "Specular Level">
      <type = scroller(0.0f, 100.0f, 1.0f, 0.01f)>;

property uniform float specularPower = 10.0f
      <name = "Specular Power">
      <type = scroller(8.0f, 64.0f, 1.0f)>;

property uniform float3 illuminationColor = {0.0f, 0.0f, 0.0f}
      <name = "Illumination Color">;


//=================================================================
//  Sampler States
//=================================================================

samplerstate textureSampler
{
    AddressU  = Wrap;
    AddressV  = Wrap;
    Filter    = Anisotropic;
};

samplerstate clampedSampler
{
    AddressU  = Clamp;
    AddressV  = Clamp;
    Filter    = Linear;
};

samplerstate testSampler
{
    AddressU  = Clamp;
    AddressV  = Clamp;
    Filter    = Anisotropic;
};


//=================================================================
//  Vertex Input
//=================================================================

struct Vertex {
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent  : TANGENT;
    float3 Normal   : NORMAL;
};

struct AnimatedVertex {
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Weights  : TEXCOORD1;
    float3 Tangent  : TANGENT;
    float3 Normal   : NORMAL;
};

struct PixelOutput {
    float4 color : COLOR;
};
