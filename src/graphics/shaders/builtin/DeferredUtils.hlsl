#ifndef _DEFERRED_UTILS
#define _DEFERRED_UTILS

#include "Common.hlsl"
#include "PBRUtils.hlsl"

struct GBufferData 
{
	float3 position, normal;
	float3 diffuse, specular, emission;
	float roughness, oneMinusReflectivity;
	bool hasData;
};

GBufferData UnpackGBuffer(int2 pixel, Texture2D _DiffuseRoughness, Texture2D _EmissionMetal, Texture2D _Position, Texture2D _Normal) 
{
	GBufferData output;
	int3 samplePos = int3(pixel, 0);

	float4 diffuseRough = _DiffuseRoughness.Load(samplePos);
	float4 emissMetal = _EmissionMetal.Load(samplePos);
	float3 pos = _Position.Load(samplePos).xyz;
	float4 rawNormal = _Normal.Load(samplePos);

	output.position = pos;
	output.normal = normalize(rawNormal.xyz);

    output.roughness = diffuseRough.a; //* diffuseRough.a;
	output.emission = emissMetal.rgb;
	output.diffuse = DiffuseAndSpecularFromMetallic(diffuseRough.rgb, emissMetal.a, output.specular, output.oneMinusReflectivity);
	output.hasData = rawNormal.a > 0.1;

	return output;
}

#endif