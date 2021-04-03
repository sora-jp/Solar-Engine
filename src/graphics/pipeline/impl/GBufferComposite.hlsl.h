#pragma once

static const char* const GBufferCompositeHlsl = R"(

#define _SAMPLE(tex, uv) tex.Sample(tex ## _sampler, uv)
#define SAMPLE(tex) _SAMPLE(tex, i.uv)

cbuffer Constants
{
	float4x4 g_Model;
	float4x4 g_ViewProj;
};

Texture2D    _GBufferDiffuse;
SamplerState _GBufferDiffuse_sampler;

Texture2D    _GBufferPosition;
SamplerState _GBufferPosition_sampler;

Texture2D    _GBufferNormal;
SamplerState _GBufferNormal_sampler;

Texture2D    _GBufferSpecMetal;
SamplerState _GBufferSpecMetal_sampler;

Texture2D    _GBufferDepth;
SamplerState _GBufferDepth_sampler;

Texture2D    _ShadowMap;
SamplerState _ShadowMap_sampler;

struct appdata
{
	float3 pos : ATTRIB0;
	float3 nrm : ATTRIB1;
	float2 uv  : ATTRIB2;
};

struct v2f
{
	float4 pos : SV_POSITION;
	float2 uv  : TEX_COORD0;
};

void vert(in appdata v, out v2f o)
{
	o.pos = float4(v.pos.xzy, 1);
	o.uv = v.uv;
}

float4 frag(in v2f i) : SV_TARGET
{
	float4 color = SAMPLE(_GBufferDiffuse);
	float3 pos = SAMPLE(_GBufferPosition).xyz;
	float3 nrm = SAMPLE(_GBufferNormal).xyz;

	float4 uv = mul(float4(pos, 1), g_Model);
	uv /= uv.w;
	uv.xy = (uv.xy) * 0.5 + 0.5;
	uv.y = 1 - uv.y;
	float shadow = _SAMPLE(_ShadowMap, uv.xy).r;

	return color * lerp(0.25, 1, saturate(dot(nrm, normalize(float3(-1, 1, -1))))) * lerp(1, 0.05, shadow < uv.z - 0.0001);
	//return float4(shadow, uv.z, uv.z - shadow, abs(shadow - uv.z));
}

)";