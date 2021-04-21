#ifndef _LIGHTING
#define _LIGHTING

#include "Common.hlsl"

struct LightData {
	float3 direction;
	float4x4 worldToLightSpace;
	float3 color;
};

PerMaterial
	LightData _MainLight;
End

#endif