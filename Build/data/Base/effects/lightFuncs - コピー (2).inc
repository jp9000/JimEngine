

//=================================================================
//  Initial Pass
//=================================================================

struct InitialPassData {
    float4 Position    : POSITION;
    float2 TexCoord    : TEXCOORD0;
};

InitialPassData InitialPassVS(Vertex input)
{
    InitialPassData PI;

    float4 newPos = input.Position;
    newPos.xyz    *= Scale;

    PI.TexCoord = input.TexCoord;
    PI.Position = mul(mul(newPos, World), ViewProj);

    return PI;
}

InitialPassData AnimatedInitialPassVS(AnimatedVertex input)
{
    InitialPassData PI;

    float4 newPos;
    newPos  = mul(input.Position, boneTransforms[0]) * input.Weights.x;
    newPos += mul(input.Position, boneTransforms[1]) * input.Weights.y;
    newPos += mul(input.Position, boneTransforms[2]) * input.Weights.z;
    newPos += mul(input.Position, boneTransforms[3]) * input.Weights.w;
    newPos.xyz *= Scale;

    PI.TexCoord = input.TexCoord;
    PI.Position = mul(mul(newPos, World), ViewProj);

    return PI;
}

PixelOutput InitialPassPS(InitialPassData PI)
{
    PixelOutput PO;

    float4 diffuse = diffuseTexture.Sample(textureSampler, PI.TexCoord);
    float4 illumMask = illuminationMap.Sample(textureSampler, PI.TexCoord);

    PO.color.xyz = (diffuse.xyz*ambientColor) + (diffuse.xyz*illuminationColor*illumMask);
    PO.color.a = diffuse.a;

    return PO;
}


//=================================================================
//  PointLight
//=================================================================

struct PointLightData {
    float4 Position    : POSITION;
    float2 TexCoord    : TEXCOORD0;
    float3 LightVector : TEXCOORD1;
    float3 HalfVector  : TEXCOORD2;
    float3 ScaledVert  : TEXCOORD3;
};

PointLightData PointLightVS(Vertex input,
                            float lightRangeI)
{
    PointLightData PI;
    
    float4 newPos       = input.Position;
    newPos.xyz          *= Scale;
    float4 worldPos     = mul(newPos, World);

    float3x3 TangentMatrix;
    TangentMatrix[0]    = -input.Tangent;
    TangentMatrix[2]    = input.Normal;
    TangentMatrix[1]    = cross(TangentMatrix[0], TangentMatrix[2]);
    TangentMatrix       = mul(TangentMatrix, World);

    float3 lightVector  = lightPos-worldPos;
    float3 lightVectorT = mul(TangentMatrix, lightVector);
    float3 eyeVectorT   = mul(TangentMatrix, eyePos-worldPos);

    PI.ScaledVert       = lightVector*lightRangeI;
    PI.LightVector      = lightVectorT;
    PI.HalfVector       = eyeVectorT+lightVectorT;
    PI.TexCoord         = input.TexCoord;
    PI.Position         = mul(worldPos, ViewProj);

    PI.ScaledVert = (lightVector*lightRangeI);
    PI.ScaledVert.xy = -PI.ScaledVert.xy;

    return PI;
}

PointLightData AnimatedPointLightVS(AnimatedVertex input,
                                    float lightRangeI)
{
    PointLightData PI;
    
    float3 newNorm, newTan;
    newNorm  = mul(input.Normal, boneTransforms[0]) * input.Weights.x;
    newNorm += mul(input.Normal, boneTransforms[1]) * input.Weights.y;
    newNorm += mul(input.Normal, boneTransforms[2]) * input.Weights.z;
    newNorm += mul(input.Normal, boneTransforms[3]) * input.Weights.w;

    newTan  = mul(input.Tangent, boneTransforms[0]) * input.Weights.x;
    newTan += mul(input.Tangent, boneTransforms[1]) * input.Weights.y;
    newTan += mul(input.Tangent, boneTransforms[2]) * input.Weights.z;
    newTan += mul(input.Tangent, boneTransforms[3]) * input.Weights.w;

    float4 newPos;
    newPos  = mul(input.Position, boneTransforms[0]) * input.Weights.x;
    newPos += mul(input.Position, boneTransforms[1]) * input.Weights.y;
    newPos += mul(input.Position, boneTransforms[2]) * input.Weights.z;
    newPos += mul(input.Position, boneTransforms[3]) * input.Weights.w;

    newPos.xyz          *= Scale;
    float4 worldPos     = mul(newPos, World);

    float3x3 TangentMatrix;
    TangentMatrix[0]    = -newTan;
    TangentMatrix[2]    = newNorm;
    TangentMatrix[1]    = cross(TangentMatrix[0], TangentMatrix[2]);
    TangentMatrix       = mul(TangentMatrix, World);

    float3 lightVector  = lightPos-worldPos;
    float3 lightVectorT = mul(TangentMatrix, lightVector);
    float3 eyeVectorT   = mul(TangentMatrix, eyePos-worldPos);

    PI.ScaledVert       = lightVector*lightRangeI;
    PI.LightVector      = lightVectorT;
    PI.HalfVector       = eyeVectorT+lightVectorT;
    PI.TexCoord         = input.TexCoord;
    PI.Position         = mul(worldPos, ViewProj);

    PI.ScaledVert = (lightVector*lightRangeI);
    PI.ScaledVert.xy = -PI.ScaledVert.xy;

    return PI;
}

PixelOutput PointLightPS(PointLightData PI)
{
    PixelOutput PO;

    float len          = length(PI.ScaledVert);
    float attenuation  = attenuationMap.Sample(clampedSampler, len).r;
    clip(attenuation-0.003);

    PO.color = DoLighting(PI.TexCoord, PI.LightVector, PI.HalfVector,
                          lightColor, specularLevel, specularPower);

    PO.color.xyz *= attenuation;

    return PO;
}

PixelOutput PointLightShadowPS(PointLightData PI)
{
    PixelOutput PO;

    float len          = length(PI.ScaledVert);
    float attenuation  = attenuationMap.Sample(clampedSampler, len).r;
    clip(attenuation-0.003);

    float shadow = NoFilterCubeShadow(PI.ScaledVert, len, 0.0078125, 3);
    clip(shadow-0.003);

    PO.color = DoLighting(PI.TexCoord, PI.LightVector, PI.HalfVector,
                          lightColor, specularLevel, specularPower);

    PO.color.xyz *= attenuation;

    //PO.color.xyz = pointDepthCube.Sample(shadowSampler, PI.ScaledVert).r;
    //PO.color.w = 1.0f;

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
    float3 ScaledVert  : TEXCOORD3;
    float4 SpotCoords  : TEXCOORD4;
};

SpotLightData SpotLightVS(Vertex input,
                          uniform float lightRangeI)
{
    SpotLightData PI;

    float4 newPos       = input.Position;
    newPos.xyz          *= Scale;
    float4 worldPos     = mul(newPos, World);

    float3x3 TangentMatrix;
    TangentMatrix[0]    = -input.Tangent;
    TangentMatrix[2]    = input.Normal;
    TangentMatrix[1]    = cross(TangentMatrix[0], TangentMatrix[2]);
    TangentMatrix       = mul(TangentMatrix, World);

    float3 lightVector  = lightPos-worldPos;
    float3 lightVectorT = mul(TangentMatrix, lightVector);
    float3 eyeVectorT   = mul(TangentMatrix, eyePos-worldPos);

    PI.ScaledVert       = lightVector*lightRangeI;
    PI.LightVector      = lightVectorT;
    PI.HalfVector       = eyeVectorT+lightVectorT;
    PI.TexCoord         = input.TexCoord;
    PI.Position         = mul(worldPos, ViewProj);
    PI.SpotCoords       = mul(spotlightMatrix, newPos);

    PI.SpotCoords.z *= lightRangeI;
    PI.SpotCoords.z -= shadowBias;

    return PI;
}

SpotLightData AnimatedSpotLightVS(AnimatedVertex input,
                                  uniform float lightRangeI)
{
    SpotLightData PI;

    float3 newNorm, newTan;
    newNorm  = mul(input.Normal, boneTransforms[0]) * input.Weights.x;
    newNorm += mul(input.Normal, boneTransforms[1]) * input.Weights.y;
    newNorm += mul(input.Normal, boneTransforms[2]) * input.Weights.z;
    newNorm += mul(input.Normal, boneTransforms[3]) * input.Weights.w;

    newTan  = mul(input.Tangent, boneTransforms[0]) * input.Weights.x;
    newTan += mul(input.Tangent, boneTransforms[1]) * input.Weights.y;
    newTan += mul(input.Tangent, boneTransforms[2]) * input.Weights.z;
    newTan += mul(input.Tangent, boneTransforms[3]) * input.Weights.w;

    float4 newPos;
    newPos  = mul(input.Position, boneTransforms[0]) * input.Weights.x;
    newPos += mul(input.Position, boneTransforms[1]) * input.Weights.y;
    newPos += mul(input.Position, boneTransforms[2]) * input.Weights.z;
    newPos += mul(input.Position, boneTransforms[3]) * input.Weights.w;

    newPos.xyz          *= Scale;
    float4 worldPos     = mul(newPos, World);

    float3x3 TangentMatrix;
    TangentMatrix[0]    = -newTan;
    TangentMatrix[2]    = newNorm;
    TangentMatrix[1]    = cross(TangentMatrix[0], TangentMatrix[2]);
    TangentMatrix       = mul(TangentMatrix, World);

    float3 lightVector  = lightPos-worldPos;
    float3 lightVectorT = mul(TangentMatrix, lightVector);
    float3 eyeVectorT   = mul(TangentMatrix, eyePos-worldPos);

    PI.ScaledVert       = lightVector*lightRangeI;
    PI.LightVector      = lightVectorT;
    PI.HalfVector       = eyeVectorT+lightVectorT;
    PI.TexCoord         = input.TexCoord;
    PI.Position         = mul(worldPos, ViewProj);
    PI.SpotCoords       = mul(spotlightMatrix, newPos);

    PI.SpotCoords.z *= lightRangeI;
    PI.SpotCoords.z -= shadowBias;

    return PI;
}

PixelOutput SpotLightPS(SpotLightData PI)
{
    PixelOutput PO;

    float2 spotUV     = (PI.SpotCoords.xy/PI.SpotCoords.w);

    float len          = length(PI.ScaledVert);
    float attenuation  = attenuationMap.Sample(clampedSampler, len).r;
    float spotMask     = spotlightMap.Sample(clampedSampler, spotUV).r;

    attenuation *= spotMask;
    attenuation = (PI.SpotCoords.w >= 0.0f) ? attenuation : 0.0f;
    clip(attenuation-0.003);

    PO.color = DoLighting(PI.TexCoord, PI.LightVector, PI.HalfVector,
                          lightColor, specularLevel, specularPower);

    PO.color.xyz *= attenuation;

    return PO;
}

PixelOutput SpotLightShadowPS(SpotLightData PI)
{
    PixelOutput PO;

    float2 spotUV     = (PI.SpotCoords.xy/PI.SpotCoords.w);

    float len          = length(PI.ScaledVert);
    float attenuation  = attenuationMap.Sample(clampedSampler, len).r;
    float spotMask     = spotlightMap.Sample(clampedSampler, spotUV).r;

    attenuation *= spotMask;
    attenuation = (PI.SpotCoords.w >= 0.0f) ? attenuation : 0.0f;
    clip(attenuation-0.003);

    float shadow = NoFilterProjShadow(spotUV, PI.SpotCoords.z, 0.00390625, 4);
    clip(shadow-0.003);

    PO.color = DoLighting(PI.TexCoord, PI.LightVector, PI.HalfVector,
                          lightColor, specularLevel, specularPower);

    PO.color.xyz *= (attenuation*shadow);

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

DirectionalLightData DirectionalLightVS(Vertex input,
                                        uniform float3 lightDir,
                                        uniform float3 eyePos)
{
    DirectionalLightData PI;

    float4 newPos       = input.Position;
    newPos.xyz          *= Scale;

    float3x3 TangentMatrix;
    TangentMatrix[0]    = -input.Tangent;
    TangentMatrix[2]    = input.Normal;
    TangentMatrix[1]    = cross(TangentMatrix[0], TangentMatrix[2]);

    float3 lightVectorT = mul(TangentMatrix, lightDir);
    float3 eyeVectorT   = normalize(mul(TangentMatrix, eyePos-newPos));

    PI.LightVector      = lightVectorT;
    PI.HalfVector       = eyeVectorT+lightVectorT;
    PI.TexCoord         = input.TexCoord;
    PI.Position         = mul(mul(newPos, World), ViewProj);

    return PI;
}

DirectionalLightData AnimatedDirectionalLightVS(AnimatedVertex input,
                                                uniform float3 lightDir,
                                                uniform float3 eyePos)
{
    DirectionalLightData PI;

    float3 newNorm, newTan;
    newNorm  = mul(input.Normal, boneTransforms[0]) * input.Weights.x;
    newNorm += mul(input.Normal, boneTransforms[1]) * input.Weights.y;
    newNorm += mul(input.Normal, boneTransforms[2]) * input.Weights.z;
    newNorm += mul(input.Normal, boneTransforms[3]) * input.Weights.w;

    newTan  = mul(input.Tangent, boneTransforms[0]) * input.Weights.x;
    newTan += mul(input.Tangent, boneTransforms[1]) * input.Weights.y;
    newTan += mul(input.Tangent, boneTransforms[2]) * input.Weights.z;
    newTan += mul(input.Tangent, boneTransforms[3]) * input.Weights.w;

    float4 newPos;
    newPos  = mul(input.Position, boneTransforms[0]) * input.Weights.x;
    newPos += mul(input.Position, boneTransforms[1]) * input.Weights.y;
    newPos += mul(input.Position, boneTransforms[2]) * input.Weights.z;
    newPos += mul(input.Position, boneTransforms[3]) * input.Weights.w;

    newPos.xyz          *= Scale;

    float3x3 TangentMatrix;
    TangentMatrix[0]    = -newTan;
    TangentMatrix[2]    = newNorm;
    TangentMatrix[1]    = cross(TangentMatrix[0], TangentMatrix[2]);

    float3 lightVectorT = mul(TangentMatrix, lightDir);
    float3 eyeVectorT   = normalize(mul(TangentMatrix, eyePos-newPos));

    PI.LightVector      = lightVectorT;
    PI.HalfVector       = eyeVectorT+lightVectorT;
    PI.TexCoord         = input.TexCoord;
    PI.Position         = mul(mul(newPos, World), ViewProj);

    return PI;
}

PixelOutput DirectionalLightPS(DirectionalLightData PI)
{
    PixelOutput PO;

    PO.color = DoLighting(PI.TexCoord, PI.LightVector, PI.HalfVector,
                          lightColor, specularLevel, specularPower);

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
};

struct LightmapData
{
    float4 Position      : POSITION;
    float2 TexCoord      : TEXCOORD0;
    float2 LightmapCoord : TEXCOORD1;
};

LightmapData LightmapVS(LightmapVertex input)
{
    LightmapData PI;

    PI.Position      = mul(mul(input.Position, World), ViewProj);
    PI.TexCoord      = input.TexCoord;
    PI.LightmapCoord = input.LightmapCoord;

    return PI;
}

PixelOutput LightmapPS(LightmapData PI)
{
    PixelOutput PO;

    const float3 UVec = {-0.70710678f, -0.40824829f, 0.57735026f};
    const float3 VVec = { 0.70710678f, -0.40824829f, 0.57735026f};
    const float3 WVec = {        0.0f,  0.81649658f, 0.57735026f};

    float3 diffuse = diffuseTexture.Sample(textureSampler, PI.TexCoord);
    float3 normal  = normalMap.Sample(textureSampler, PI.TexCoord);
    float3 illumMap= illuminationMap.Sample(textureSampler, PI.TexCoord);
    normal = 2.0*(normal-0.5);

    float3 LU      = lightmapU.Sample(textureSampler, PI.LightmapCoord);
    float3 LV      = lightmapV.Sample(textureSampler, PI.LightmapCoord);
    float3 LW      = lightmapW.Sample(textureSampler, PI.LightmapCoord);

    float3 shade = (LU*(dot(normal, UVec))) +
                   (LV*(dot(normal, VVec))) +
                   (LW*(dot(normal, WVec)));

    PO.color.xyz = (diffuse*shade)+(diffuse*ambientColor)+(diffuse*illumMap*illuminationColor);
    PO.color.a = 1.0f;

    return PO;
}


//=================================================================
//  Texture Only
//=================================================================

struct TexOnlyInOut {
    float4 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

TexOnlyInOut TexOnlyVS(TexOnlyInOut input)
{
    TexOnlyInOut output;
    
    float4 newPos = input.Position;
    newPos.xyz    *= Scale;

    output.Position = mul(mul(newPos, World), ViewProj);
    output.TexCoord = input.TexCoord;
    return output;
}

float4 TexOnlyPS(TexOnlyInOut input) : COLOR
{
    return diffuseTexture.Sample(textureSampler, input.TexCoord);//float4(1.0, 1.0, 1.0, 1.0);//
}


//=================================================================
//  techniques
//=================================================================

technique TextureOnly
{
    pass Normal
    {
        VertexShader = TexOnlyVS();
        PixelShader  = TexOnlyPS();
    }
}

technique InitialPass
{
    pass Normal
    {
        VertexShader = InitialPassVS();
        PixelShader  = InitialPassPS();
    }
    pass Animated
    {
        VertexShader = AnimatedInitialPassVS();
        PixelShader  = InitialPassPS();
    }
}

technique DirectionalLight
{
    pass Normal
    {
        VertexShader = DirectionalLightVS(lightDir, eyePos);
        PixelShader  = DirectionalLightPS();
    }
    pass Animated
    {
        VertexShader = AnimatedDirectionalLightVS(lightDir, eyePos);
        PixelShader  = DirectionalLightPS();
    }
}

technique PointLight
{
    pass Normal
    {
        VertexShader = PointLightVS(1.0f/lightRange);
        PixelShader  = PointLightPS();
    }
    pass Animated
    {
        VertexShader = AnimatedPointLightVS(1.0f/lightRange);
        PixelShader  = PointLightPS();
    }
}

technique PointLightShadow
{
    pass Normal
    {
        VertexShader = PointLightVS(1.0f/lightRange);
        PixelShader  = PointLightShadowPS();
    }
    pass Animated
    {
        VertexShader = AnimatedPointLightVS(1.0f/lightRange);
        PixelShader  = PointLightShadowPS();
    }
}

technique SpotLight
{
    pass Normal
    {
        VertexShader = SpotLightVS(1.0f/lightRange);
        PixelShader  = SpotLightPS();
    }
    pass Animated
    {
        VertexShader = AnimatedSpotLightVS(1.0f/lightRange);
        PixelShader  = SpotLightPS();
    }
}

technique SpotLightShadow
{
    pass Normal
    {
        VertexShader = SpotLightVS(1.0f/lightRange);
        PixelShader  = SpotLightShadowPS();
    }
    pass Animated
    {
        VertexShader = AnimatedSpotLightVS(1.0f/lightRange);
        PixelShader  = SpotLightShadowPS();
    }
}

technique Lightmap
{
    pass Normal
    {
        VertexShader = LightmapVS();
        PixelShader  = LightmapPS();
    }
}
