#include "FullScreenQuad.hlsl"

Texture2D<float4> _MainTex;
SamplerState _MainTex_sampler;

float4 frag(v2f i) : SV_Target {
    return _MainTex.Sample(_MainTex_sampler, i.uv);
}