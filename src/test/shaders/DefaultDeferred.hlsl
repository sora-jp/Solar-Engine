#include "Common.hlsl"

//#pragma vertex vert
//#pragma fragment frag
//#pragma mode deferred

struct appdata
{
    float3 Pos : ATTRIB0;
    float3 Nrm : ATTRIB1;
    float3 Tangent : ATTRIB2;
    float2 UV  : ATTRIB3;
};

struct v2f
{
    float4 Pos : SV_POSITION;
    float3 Nrm : TEX_COORD0;
    float3 Tangent : TEX_COORD1;
    float2 UV : TEX_COORD2;
    float3 WorldPos : TEX_COORD3;
};

struct psout
{
    float4 ColorRough : SV_TARGET0;
    float4 EmissMetal : SV_TARGET1;
    float4 Normal : SV_TARGET2;
    float4 Position : SV_TARGET3;
};

PerMaterial

float4    _Tint; // Material tint
float4    _Emission; // Material emission
float2    _SmoothnessMetal; // Specular, Metal, _, _
float3    _TexturesPresent;

End

Texture2D<float4> _MainTex;
SamplerState sampler_MainTex;

Texture2D<float4> _NormalTex;
SamplerState sampler_NormalTex;

Texture2D<float4> _MRTex;
SamplerState sampler_MRTex;

void vert(in appdata v, out v2f o)
{
    o.Pos = mul(float4(v.Pos, 1), mul(g_Model, g_ViewProj));
    o.WorldPos = mul(float4(v.Pos, 1), g_Model).xyz;
    o.Nrm = normalize(mul(float4(v.Nrm, 0), g_Model).xyz);
    o.Tangent = normalize(mul(float4(v.Tangent, 0), g_Model).xyz);
    o.UV = v.UV;
}

void frag(in v2f i, in bool ff : SV_IsFrontFace, out psout o)
{
    float3 nrm = normalize(i.Nrm);
    float3 tangent = normalize(i.Tangent);
    float3 bitangent = normalize(cross(nrm, tangent));
    
    float3 ppNrm = lerp(normalize(_NormalTex.Sample(sampler_NormalTex, frac(i.UV)).xyz * 2 - 1), float3(0, 0, 1), 0.5);
    ppNrm.z = abs(ppNrm.z);
    ppNrm = normalize(lerp(nrm, ppNrm.z * nrm + -ppNrm.x * tangent + ppNrm.y * bitangent, _TexturesPresent.y));
    
    float2 sm = lerp(_SmoothnessMetal, _MRTex.Sample(sampler_MRTex, frac(i.UV)).gb, _TexturesPresent.z);
    
    //float4 tc = _MainTex.Sample(sampler_MainTex, frac(i.UV));
    //clip(tc - _TexturesPresent.x * 0.001);
    
    o.ColorRough = float4(_Tint.rgb * lerp(1, _MainTex.Sample(sampler_MainTex, frac(i.UV)).rgb, _TexturesPresent.x), sm.r); //lerp(0.015, 1, saturate(dot(PSIn.Nrm, normalize(float3(1, 1, -1)))));//_MainTex.Sample(_MainTex_sampler, PSIn.UV);
    o.EmissMetal = float4(_Emission.rgb, sm.g);
    
    o.Normal = float4(normalize(ppNrm), 1);

    o.Position = float4(i.WorldPos, 1);
}