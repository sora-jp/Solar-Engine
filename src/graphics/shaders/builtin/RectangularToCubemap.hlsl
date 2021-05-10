#include "CubemapUtils.hlsl"

Texture2D<float4> _MainTex;
SamplerState sampler_MainTex;

fout frag(in v2f i)
{
    fout o = (fout) 0;
    
    cubecoords c = GetCoords(i);
    
    o.posX = pow(_MainTex.Sample(sampler_MainTex, RadialCoords(c.posX)), 1) * 1;
    o.negX = pow(_MainTex.Sample(sampler_MainTex, RadialCoords(c.negX)), 1) * 1;
    o.posY = pow(_MainTex.Sample(sampler_MainTex, RadialCoords(c.posY)), 1) * 1;
    o.negY = pow(_MainTex.Sample(sampler_MainTex, RadialCoords(c.negY)), 1) * 1;
    o.posZ = pow(_MainTex.Sample(sampler_MainTex, RadialCoords(c.posZ)), 1) * 1;
    o.negZ = pow(_MainTex.Sample(sampler_MainTex, RadialCoords(c.negZ)), 1) * 1;
    
    return o;
}