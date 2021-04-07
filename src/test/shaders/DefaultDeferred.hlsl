#include "Common.hlsl"

#pragma vertex vert
#pragma fragment frag
#pragma mode deferred

struct appdata
{
    float3 Pos : ATTRIB0;
    float3 Nrm : ATTRIB1;
    float2 UV  : ATTRIB2;
};

struct v2f
{
    float4 Pos : SV_POSITION;
    float3 Nrm : TEX_COORD0;
    float2 UV  : TEX_COORD1;
    float3 WorldPos : TEX_COORD2;
};

struct psout
{
    float4 Color : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Position : SV_TARGET2;
    float4 SpecMetal : SV_TARGET3;
};

PerMaterial

float4 _Tint; // Material tint
float4 _SMXX; // Specular, Metal, _, _

End

void vert(in appdata v, out v2f o)
{
    o.Pos = mul(float4(v.Pos, 1), mul(g_Model, g_ViewProj));
    o.WorldPos = mul(float4(v.Pos, 1), g_Model).xyz;
    o.Nrm = normalize(mul(float4(v.Nrm, 0), g_Model).xyz);
    o.UV = v.UV;
}

void frag(in v2f i, out psout o)
{
    o.Color = _Tint;//lerp(0.015, 1, saturate(dot(PSIn.Nrm, normalize(float3(1, 1, -1)))));//_MainTex.Sample(_MainTex_sampler, PSIn.UV);
    o.Normal = float4(normalize(i.Nrm), 1);
    o.Position = float4(i.WorldPos, 1);
    o.SpecMetal = float4(_SMXX.rg, 0, 0);
}