#include "FullScreenQuad.hlsl"

Texture2D<float4> _MainTex;
SamplerState sampler_MainTex;

float4 frag(v2f i) : SV_Target {
    return _MainTex.Sample(sampler_MainTex, i.uv);
}