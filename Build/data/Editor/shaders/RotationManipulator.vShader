float4x4 WorldViewProj;
float3   color;
float3   camDir;

struct OutData
{
    float4 pos : POSITION;
    float4 color : COLOR;
};

OutData main(float4 pos : POSITION, float4 norm : NORMAL)
{
    OutData outData;
    
    outData.pos = mul(pos, WorldViewProj);
    outData.color.xyz = color * ((dot(norm, camDir) > 0.1) ? 0.0 : 1.0);
    outData.color.w = 1.0;
    
    return outData;
}
