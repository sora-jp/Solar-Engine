#pragma once

static const char* const GBufferCompositeHlsl = R"(

#define _SAMPLE_CMP(tex, uv, cmp) tex.SampleCmp(tex ## _sampler, uv, cmp)
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

float2 sampleShadows(float2 coord) : SV_Target
{
    float2 f2Moments = float2(0.0, 0.0);
    float range = 0.05;
	int samples = 16;
    float fTotalWeight = 0.0;

    for (int i = 0; i <= 16; ++i)
    {
		float o1 = (i * range - range * samples / 2) / samples;
  //      float fWeight = GetSampleWeight(i, g_Attribs.fHorzFilterRadius);
  //      float fDepth = _SAMPLE(_ShadowMap, coord + float2(o, 0)).r;
  //      f2Moments += float2(fDepth, fDepth*fDepth) * fWeight;
  //      fTotalWeight += fWeight;

	    for (int j = 0; j <= 16; ++j)
	    {
			float o2 = (j * range - range * samples / 2) / samples;
			float2 nc = coord + float2(o1, o2);
	        float fWeight = 1 - saturate(2 * length(nc - coord) / range);
	        float fDepth = _SAMPLE(_ShadowMap, nc).r;
			float2 d = float2(ddx(fDepth), ddy(fDepth));
	        f2Moments += float2(fDepth, fDepth*fDepth + 0.25 * dot(d, d)) * fWeight;
	        fTotalWeight += fWeight;
	    }
    }

    return float2(f2Moments / fTotalWeight);
}

// Reduces VSM light bleedning
float ReduceLightBleeding(float pMax, float amount)
{
    // Remove the [0, amount] tail and linearly rescale (amount, 1].
     return saturate((pMax - amount) / (1.0 - amount));
}

float chebyshevUpperBound(float2 f2Moments, float fMean, float fMinVariance, float fLightBleedingReduction)
{
    float Variance = f2Moments.y - (f2Moments.x * f2Moments.x);
    Variance = max(Variance, fMinVariance);

    // Probabilistic upper bound
    float d = fMean - f2Moments.x;
    float pMax = Variance / (Variance + (d * d));

    pMax = ReduceLightBleeding(pMax, fLightBleedingReduction);

    // One-tailed Chebyshev
    return (fMean <= f2Moments.x ? 1.0 : pMax);
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
	float2 moments = sampleShadows(uv.xy);
	float shadow = chebyshevUpperBound(moments, uv.z, 0.000, 0.00);

	//return shadow;
	return color * lerp(0.25, 1, saturate(dot(nrm, normalize(float3(-1, 1, -1))))) * lerp(0.05, 1, shadow);
	return float4((color * lerp(0.25, 1, saturate(dot(nrm, normalize(float3(-1, 1, -1))))) * lerp(0.05, 1, shadow)).r, moments, abs(shadow));
	//return float4(shadow, uv.z, uv.z - shadow, abs(shadow - uv.z));
}

)";