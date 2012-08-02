//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Shadow Functions
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//=================================================================
//  Parameters
//=================================================================

uniform texture2d   spotDepthTexture;
uniform textureCube pointDepthCube;

uniform float       shadowBias = 0.02f;


//=================================================================
//  Samplers
//=================================================================

samplerstate shadowSampler
{
    AddressU  = Clamp;
    AddressV  = Clamp;
    Filter    = Linear;
};


//=================================================================
//  Filters
//=================================================================

float NoFilterProjShadow(float2 uv, float receiver,
                         float filterWidth, float numSamples)
{
    receiver -= shadowBias;

    float shade, shadMapDepth;
    shadMapDepth = spotDepthTexture.Sample(shadowSampler, uv).r;
    shade = receiver < shadMapDepth;

    return shade;
}

float PCF_FilterProj(float2 uv, float receiver,
                     float filterWidth, float numSamples)
{
    float stepSize = (2*filterWidth)/numSamples;

    filterWidth -= (stepSize*0.5f);
    uv -= float2(filterWidth,filterWidth);

    float sum = 0;

    for (int x=0; x<numSamples; x++)
    {
        float stepX = x*stepSize;

        for (int y=0; y<numSamples; y++)
        {
            float stepY = y*stepSize;

            float shadMapDepth = ShadowMap.Sample(shadowSampler, uv + float2(stepX,stepY)).r;
            float shade = receiver < shadMapDepth;

            sum += shade;
        }
    }

    return sum / (numSamples*numSamples);
}

float NoFilterCubeShadow(float3 pos, float receiver,
                         float filterWidth, float numSamples)
{
    receiver -= shadowBias;

    float shade, shadMapDepth;
    shadMapDepth = pointDepthCube.Sample(shadowSampler, pos).r;
    shade = receiver < shadMapDepth;

    return shade;
}


float PCF_FilterCube(float3 pos, float receiver,
                     float filterWidth, float numSamples)
{
    float stepSize = (2*filterWidth)/numSamples;

    receiver -= shadowBias;

    float sum = 0;
    float stepX, stepY, shade, shadMapDepth;

    filterWidth -= (stepSize*0.5f);
    pos -= float3(filterWidth,filterWidth,filterWidth);

    for (int x=0; x<numSamples; x++)
    {
        stepX = stepSize*x;

        for (int y=0; y<numSamples; y++)
        {
            stepY = stepSize*y;

            for (int z=0; z<numSamples; z++)
            {
                shadMapDepth = pointDepthCube.Sample(shadowSampler, pos + float3(stepX, stepY, stepSize*z)).r;
                shade = receiver < shadMapDepth;

                sum += shade;
            }
        }
    }

    return sum / (numSamples*numSamples*numSamples);
}
