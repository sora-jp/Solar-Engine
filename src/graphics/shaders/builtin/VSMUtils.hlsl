#ifndef _VSM_UTILS
#define _VSM_UTILS

#include "Lighting.hlsl"

// Reduces VSM light bleedning
float ReduceLightBleeding(float pMax, float amount)
{
	return saturate((pMax - amount) / (1.0 - amount));
}

float ChebyshevUpperBound(float2 f2Moments, float fMean, float fMinVariance, float fLightBleedingReduction)
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

inline float SampleShadows(float3 pos, LightData light, Texture2D shadowMap, SamplerState shadowSampler) 
{
	float4 uv = mul(float4(pos, 1), light.worldToLightSpace);
	uv /= uv.w; // Account for perspective projection in light (Spotlights etc.)
	uv.xy = (uv.xy) * 0.5 + 0.5;
	uv.y = 1 - uv.y;

	float2 grad = float2(ddx(uv.z), ddy(uv.z));
	float2 moments = shadowMap.SampleGrad(shadowSampler, uv.xy, grad.x, grad.y).rg;
	
	// Compute light contribution (fade out near shadow-map edge)
	float p = length(uv.xy - 0.5) * 2;
	p = saturate(pow(p, 15));

	return saturate(lerp(p, 1, ChebyshevUpperBound(moments, uv.z, 0.0001, 0.00)));
}

#endif