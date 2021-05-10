#ifndef _PBR_UTILS
#define _PBR_UTILS

#include "Common.hlsl"

inline float PerceptualRoughnessFromRoughness(float roughness)
{
	return sqrt(roughness);
}

inline float RoughnessFromPerceptualRoughness(float perceptualRoughness)
{
	return perceptualRoughness * perceptualRoughness;
}

inline half OneMinusReflectivityFromMetallic(half metallic)
{
	// We'll need oneMinusReflectivity, so
	//   1-reflectivity = 1-lerp(dielectricSpec, 1, metallic) = lerp(1-dielectricSpec, 0, metallic)
	// store (1-dielectricSpec) in unity_ColorSpaceDielectricSpec.a, then
	//   1-reflectivity = lerp(alpha, 0, metallic) = alpha + metallic*(0 - alpha) =
	//                  = alpha - metallic * alpha
	half oneMinusDielectricSpec = 1 - 0.04;
	return oneMinusDielectricSpec - metallic * oneMinusDielectricSpec;
}

inline float3 DiffuseAndSpecularFromMetallic(float3 albedo, float metallic, out float3 specColor, out float oneMinusReflectivity)
{
	specColor = lerp(0.04, albedo, metallic);
	oneMinusReflectivity = OneMinusReflectivityFromMetallic(metallic);
	return albedo * oneMinusReflectivity;
}

inline float3 FresnelLerp(float3 F0, float3 F90, float cosA)
{
	return lerp(F0, F90, pow(1 - cosA, 5));
}

inline float3 Fresnel(float3 F0, float cosA)
{
	return FresnelLerp(F0, 1, cosA);
}

#endif