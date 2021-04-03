#pragma once

static const char* const ShadowMapShaderHlsl = R"(

cbuffer Constants
{
	float4x4 g_Model;
	float4x4 g_ViewProj;
};

struct appdata
{
	float3 pos : ATTRIB0;
	float3 nrm : ATTRIB1;
	float2 uv  : ATTRIB2;
};

struct v2f
{
	float4 pos : SV_POSITION;
	float depth : TEX_COORD0;
};

void vert(in appdata v, out v2f o)
{
	o.pos = mul(float4(v.pos, 1), mul(g_Model, g_ViewProj));
	o.depth = o.pos.z / o.pos.w;
}

float4 frag(in v2f i) : SV_TARGET
{
	return float4(i.depth, i.depth * i.depth, 0, 1);
}

)";