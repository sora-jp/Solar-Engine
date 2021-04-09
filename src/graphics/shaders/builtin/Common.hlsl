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

#endif //_COMMON_HLSL_