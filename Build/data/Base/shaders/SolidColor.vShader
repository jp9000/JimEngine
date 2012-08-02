uniform float4x4 WorldViewProj;
uniform float4 solidColor;

struct SolidOutput
{
    float4 pos : POSITION;
    float4 color : COLOR;
};

SolidOutput main(float4 pos : POSITION)
{
    SolidOutput output;

    output.pos = mul(pos, WorldViewProj);
    output.color = solidColor;
    return output;
}
