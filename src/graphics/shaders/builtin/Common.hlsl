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

typedef float4 color;
typedef float4 hdr_color;

#endif //_COMMON_HLSL_