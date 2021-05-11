#ifndef _COMMON_HLSL_
#define _COMMON_HLSL_

#define _SAMPLE_CMP(tex, uv, cmp) tex.SampleCmp(tex ## _sampler, uv, cmp)
#define _SAMPLE(tex, uv) tex.Sample(tex ## _sampler, uv)
#define SAMPLE(tex) _SAMPLE(tex, i.uv)

#ifndef NDC_MIN_Z
#define NDC_MIN_Z 0
#endif

#define PerMaterial cbuffer _PerMaterial {
#define End };

cbuffer Constants
{
	float4x4 g_Model;
    float4x4 g_View;
	float4x4 g_ViewProj;
	float3 g_WorldSpaceCameraPos;
};

inline float2 RadialCoords(float3 a_coords)
{
	float3 a_coords_n = normalize(a_coords);
	float lon = atan2(a_coords_n.z, a_coords_n.x);
	float lat = acos(a_coords_n.y);
	float2 sphereCoords = float2(lon, lat) * (1.0 / 3.141592653589793);
	return float2(sphereCoords.x * 0.5 + 0.5, sphereCoords.y);
}

#define PI 3.14159

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

#endif //_COMMON_HLSL_