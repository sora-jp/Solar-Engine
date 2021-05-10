#include "Common.hlsl"
#include "FullScreenQuad.hlsl"
#include "BRDF.hlsl"
#include "VSMUtils.hlsl"
#include "DeferredUtils.hlsl"

Texture2D    _GBDiffuseRough;
Texture2D    _GBEmissionMetal;
Texture2D    _GBPosition;
Texture2D    _GBNormal;
Texture2D    _GBDepth;

Texture2D    _ShadowMap;
SamplerState _ShadowMap_sampler;

Texture2D    _Skybox;
SamplerState _Skybox_sampler;

Texture2D    _IBL;
SamplerState _IBL_sampler;

float3 ACESFilm(float3 x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

//TODO: Reference	http://www.cse.chalmers.se/edu/year/2018/course/TDA361/Advanced%20Computer%20Graphics/SurfaceScattering.pdf for GGX and other models
//					http://advances.realtimerendering.com/other/2016/naughty_dog/NaughtyDog_TechArt_Final.pdf for cool effects n shit
//					https://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf for stuff about BRDFs
//					https://media.contentapi.ea.com/content/dam/eacom/frostbite/files/s2014-pbs-frostbite-slides.pdf

float4 frag(in v2f i) : SV_TARGET
{
	float4 rv = mul(float4(i.normalizedXY, 0.01, 1), g_ViewProj);
	float3 viewDir = normalize(g_WorldSpaceCameraPos - rv.xyz / rv.w);
	
	LightData light = _MainLight;
	GBufferData data = UnpackGBuffer(i.pixelPos.xy, _GBDiffuseRough, _GBEmissionMetal, _GBPosition, _GBNormal);
	
	[branch]
	if (!data.hasData) return _Skybox.SampleLevel(_Skybox_sampler, RadialCoords(viewDir), 0);

	float shadow = SampleShadows(data.position, light, _ShadowMap, _ShadowMap_sampler);

	float3 specularContrib = SpecularBRDF(data.position, data.normal, light.direction, viewDir, data.roughness * data.roughness) * Fresnel(data.specular, dot(data.normal, viewDir));
	float3 diffuseContrib = DiffuseBRDF(data.position, data.normal, light.direction, viewDir, data.roughness);

	float3 refl = reflect(viewDir, data.normal);
	float3 reflCol = _Skybox.SampleLevel(_Skybox_sampler, RadialCoords(refl), 0).rgb;

	float3 cdown = float3(186, 179, 168) / 255;
	float3 cmid = float3(92, 107, 64) / 255;
	float3 cup = float3(191, 236, 254) / 255;

	float3 ambientCol = lerp(cmid, data.normal.y > 0 ? cup : cdown, abs(data.normal.y));

	float surfaceReduction = 1.0 / (data.roughness * data.roughness + 1.0);
	float grazingTerm = saturate((1 - data.roughness) + (1 - data.oneMinusReflectivity));
	float fres = FresnelLerp(data.specular, grazingTerm, dot(data.normal, viewDir));

	float3 final = data.diffuse * (ambientCol + diffuseContrib * shadow * light.color)
		+ specularContrib * light.color * shadow
		+ reflCol * fres * surfaceReduction
		+ data.emission;

	return float4(final, 1);
}