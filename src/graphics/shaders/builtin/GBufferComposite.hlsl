#include "Common.hlsl"
#include "FullScreenQuad.hlsl"

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

float2 sampleShadows(float2 coord, float2 grad) : SV_Target
{
	return _ShadowMap.SampleGrad(_ShadowMap_sampler, coord, grad.x, grad.y).rg;
}

// Reduces VSM light bleedning
float ReduceLightBleeding(float pMax, float amount)
{
	// Remove the [0, amount] tail and linearly rescale (amount, 1].
	 return saturate((pMax - amount) / (1.0 - amount));
}

float chebyshevUpperBound(float2 f2Moments, float fMean, float fMinVariance, float fLightBleedingReduction)
{
	float Variance = (f2Moments.y) - (f2Moments.x * f2Moments.x);
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

	float p = length(uv.xy);
	p = saturate(pow(p * 1.015, 15) + 0.1);

	uv.xy = (uv.xy) * 0.5 + 0.5;
	uv.y = 1 - uv.y;
	float2 moments = sampleShadows(uv.xy, float2(ddx(uv.z), ddy(uv.z)));
	float shadow = lerp(p, 1, chebyshevUpperBound(moments, uv.z, 0.0002, 0.75));
	//shadow += saturate(pow(p * 1.05, 5));

	//return shadow;
	return float4((color * lerp(0.05, 1, saturate(dot(nrm, normalize(float3(-1, 1, -1)))) * shadow)).rgb, 1);
	return float4(p, p, p, (color * lerp(0.1, 1, saturate(dot(nrm, normalize(float3(-1, 1, -1)))) * shadow)).r);
	//return float4(shadow, uv.z, uv.z - shadow, abs(shadow - uv.z));
}