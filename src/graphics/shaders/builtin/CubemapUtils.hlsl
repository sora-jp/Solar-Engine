#include "FullScreenQuad.hlsl"

struct fout
{
    float4 posX : SV_Target0;
    float4 negX : SV_Target1;
    float4 posY : SV_Target2;
    float4 negY : SV_Target3;
    float4 posZ : SV_Target4;
    float4 negZ : SV_Target5;
};

struct cubecoords
{
    float3 posX;
    float3 negX;
    float3 posY;
    float3 negY;
    float3 posZ;
    float3 negZ;
};

cubecoords GetCoords(v2f i)
{
    cubecoords o = (cubecoords)0;
    float2 p = i.normalizedXY;
    
    o.posX = normalize(float3(1, p.y, -p.x));
    o.negX = normalize(float3(-1, p.yx));
    
    o.posY = normalize(float3(p.x, 1, -p.y));
    o.negY = normalize(float3(p.x, -1, p.y));
    
    o.posZ = normalize(float3(p, 1));
    o.negZ = normalize(float3(-p.x, p.y, -1));
    
    return o;
}