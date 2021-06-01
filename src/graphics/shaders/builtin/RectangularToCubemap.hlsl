#include "CubemapUtils.hlsl"

Texture2D<float4> _MainTex;
SamplerState _MainTex_sampler;

fout frag(in v2f i)
{
    fout o = (fout) 0;
    
    cubecoords c = GetCoords(i);
    
    o.posX = pow(_MainTex.Sample(_MainTex_sampler, RadialCoords(c.posX)));
    o.negX = pow(_MainTex.Sample(_MainTex_sampler, RadialCoords(c.negX)));
    o.posY = pow(_MainTex.Sample(_MainTex_sampler, RadialCoords(c.posY)));
    o.negY = pow(_MainTex.Sample(_MainTex_sampler, RadialCoords(c.negY)));
    o.posZ = pow(_MainTex.Sample(_MainTex_sampler, RadialCoords(c.posZ)));
    o.negZ = pow(_MainTex.Sample(_MainTex_sampler, RadialCoords(c.negZ)));
    
    return o;
}