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

Texture2D    _Skybox;
SamplerState _Skybox_sampler;

Texture2D    _IBL;
SamplerState _IBL_sampler;

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

float SphericalGaussianApprox(float CosX, float ModifiedSpecularPower)
{
	return exp2(ModifiedSpecularPower * CosX - ModifiedSpecularPower);
}
//#define OneOnLN2_x6  // == 1/ln(2) * 6   (6 is SpecularPower of 5 + 1)
float FresnelSchlick(float SpecularColor, float3 E, float3 H)
{
	// In this case SphericalGaussianApprox(1.0f - saturate(dot(E, H)), OneOnLN2_x6) is equal to exp2(-OneOnLN2_x6 * x)
	return SpecularColor + (1 - SpecularColor) * pow(1 - dot(E, H), 5);
	//return SpecularColor + (1.0f - SpecularColor) * exp2(-8.656170 * saturate(dot(E, H)));
}

float FresnelCookTorrance(float f0, float vdoth) 
{
	float n = (1 + sqrt(f0)) / (1 - sqrt(f0));
	float c2 = vdoth * vdoth;
	float g = sqrt(n * n + c2 - 1);

	float t1 = (g - vdoth) / (g + vdoth);
	t1 *= t1;

	float gc = g * vdoth;
	float t2 = (gc + c2 - 1) / (gc - c2 + 1);
	t2 = 1 + t2 * t2;

	return (t1 * t2) / 2;
}

inline float GGXPartialShadowingTerm(float hdotv, float alpha) 
{
	float a2 = alpha * alpha;

	return 2 * hdotv / (hdotv + sqrt(a2 + (1 - a2) * hdotv * hdotv));
}

inline float GGXDistributionTerm(float ndoth, float alpha) 
{
	float ndh2 = ndoth * ndoth;
	float a2 = alpha * alpha;

	float denom = ndh2 * ndh2 * (a2 - 1) + 1;

	return a2 / (3.1415 * denom * denom);
}

// TODO: READ http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html PLES
// Also look at https://www.iquilezles.org/www/index.htm
float specular(float3 pos, float3 nrm, float3 dirToLight, float3 viewDir, float roughness, float x) 
{
	float3 halfVec = normalize(viewDir + dirToLight);
	
	float vdotn = dot(nrm, viewDir);
	float ndoth = dot(nrm, halfVec);
	float ndh2 = ndoth * ndoth;

	float vdoth = dot(viewDir, halfVec);

	float m2 = roughness/* * roughness*/;

	float kspec = exp(-(1 - ndh2) / (ndh2 * m2));
	kspec /= 3.1415 * m2 * ndh2 * ndh2;

	//float g = min(abs(2 * ndoth * dot(viewDir, nrm) / vdoth), abs(2 * ndoth * dot(viewDir, dirToLight) / vdoth));
	//g = saturate(min(g, 1));

	float g = 0;
	if (vdotn < dot(nrm, dirToLight)) {
		if (2 * vdotn * ndoth < vdoth) g = 2 * ndoth / vdoth;
		else g = 1 / vdotn;
	}
	else {
		if (2 * dot(nrm, dirToLight) * ndoth < vdoth) g = 2 * dot(nrm, dirToLight) * ndoth / (vdoth * vdotn);
		else g = 1 / vdotn;
	}
	g = saturate(g);

	g = saturate(GGXPartialShadowingTerm(vdoth, m2) * GGXPartialShadowingTerm(dot(halfVec, dirToLight), m2));
	kspec = GGXDistributionTerm(ndoth, m2);

	float n = 1.25;
	float r0 = pow((1 - n) / (1 + n), 2);
	float fresnel = r0 + (1 - r0) * pow(1 - dot(nrm, dirToLight), 5);
	fresnel = FresnelCookTorrance(r0, vdotn);

	float spec = kspec * g * fresnel / (3.1415 * vdotn * dot(dirToLight, nrm));
	return spec;//float3(kspec, g, fresnel);
}

inline float2 radialCoords(float3 a_coords)
{
	float3 a_coords_n = normalize(a_coords);
	float lon = atan2(a_coords_n.z, a_coords_n.x);
	float lat = acos(a_coords_n.y);
	float2 sphereCoords = float2(lon, lat) * (1.0 / 3.141592653589793);
	return float2(sphereCoords.x * 0.5 + 0.5, sphereCoords.y);
}

#ifndef NDC_MAX_DEPTH
#define NDC_MAX_DEPTH 0.01
#endif
float3 ACESFilm(float3 x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}
//TODO: Reference http://www.cse.chalmers.se/edu/year/2018/course/TDA361/Advanced%20Computer%20Graphics/SurfaceScattering.pdf for GGX and other models
float4 frag(in v2f i) : SV_TARGET
{
	float4 color = _GBufferDiffuse.Load(int3(i.pixelPos.xy, 0));
	float3 pos = _GBufferPosition.Load(int3(i.pixelPos.xy, 0)).xyz;
	float3 nrm = normalize(_GBufferNormal.Load(int3(i.pixelPos.xy, 0)).xyz);

	float4 uv = mul(float4(pos, 1), g_Model);
	uv /= uv.w;

	float p = length(uv.xy);
	p = saturate(pow(p * 1.015, 15) + 0.1);

	uv.xy = (uv.xy) * 0.5 + 0.5;
	uv.y = 1 - uv.y;

	float2 moments = sampleShadows(uv.xy, float2(ddx(uv.z), ddy(uv.z)));
	float shadow = lerp(p, 1, chebyshevUpperBound(moments, uv.z, 0.0001, 0.5));
	shadow = saturate(shadow + pow(p * 1.05, 5));

	float3 lightDir = normalize(float3(-1, 1, -1));
	float3 viewDir = normalize(g_WorldSpaceCameraPos - pos);

	float4 rv = mul(float4(i.normalizedXY, NDC_MAX_DEPTH, 1), g_ViewProj);
	float3 rawVDir = normalize(g_WorldSpaceCameraPos - rv.xyz / rv.w);

	float roughness = .075;
	float specularContrib = saturate(specular(pos, nrm, lightDir, rawVDir, roughness * roughness, i.normalizedXY.x));

	float kdiff = 1 - roughness;
	float a = 1 - 0.5 * (kdiff / (kdiff + 0.33));
	float b = 0.45 * (kdiff / (kdiff + 0.09));
	float alpha = max(dot(nrm, lightDir), dot(nrm, viewDir));
	float beta = min(dot(nrm, lightDir), dot(nrm, viewDir));
	
	float sa = sqrt(1 - alpha * alpha);
	float tb = sqrt(1 - beta * beta) / beta;
	
	float diffuseContrib = saturate((1 / 3.1415) * dot(nrm, lightDir) * (a + (b * max(0, dot(lightDir, viewDir)) * sa * tb)));
	float3 refl = reflect(rawVDir, nrm);

	float3 reflCol = _Skybox.SampleLevel(_Skybox_sampler, radialCoords(refl), 0).rgb;

	float3 cdown = float3(186, 179, 168) / 255;
	float3 cmid = float3(92, 107, 64) / 255;
	float3 cup = float3(191, 236, 254) / 255;

	//float2 auv = radialCoords(nrm * float3(1, -1, 1));
	//float dx = ddx(auv);
	//float dy = ddy(auv);
	float3 ambientCol = lerp(cmid, nrm.y > 0 ? cup : cdown, abs(nrm.y));//_IBL.Sample(_IBL_sampler, auv).rgb;//_Skybox.SampleLevel(_Skybox_sampler, radialCoords(normalize(nrm * float3(1, -1, 1))), 6).rgb;
	
	//return shadow;
	//return _SAMPLE(_Skybox, radialCoords(refl));
	//return float4(ambientCol / (ambientCol + 1), 1);
	//return float4(ambientCol, 1);
	float fres = FresnelCookTorrance(0.025, dot(nrm, rawVDir));
	float3 final = color * (ambientCol + saturate(diffuseContrib * shadow)).rgb + specularContrib * shadow + reflCol * fres * lerp(color, 1, fres) /* * lerp(saturate(dot(refl, lightDir)), 1, shadow)*/;
	return float4((final), 1);
	//return float4(p, p, p, (color * lerp(0.1, 1, saturate(dot(nrm, lightDir)) * shadow)).r);
	//return float4(shadow, uv.z, uv.z - shadow, abs(shadow - uv.z));
}