#ifndef _BRDF
#define _BRDF

//Smith-GGX Shadowing term. computed as G(h, l, v) = G(h l) * G(h, v)
inline float GGXPartialShadowingTerm(float hdotv, float alpha)
{
	float a2 = alpha * alpha;
	return 2 * hdotv / (hdotv + sqrt(a2 + (1 - a2) * hdotv * hdotv));
}

//GGX distribution term, D
inline float GGXDistributionTerm(float ndoth, float alpha)
{
	float ndh2 = ndoth * ndoth;
	float a2 = alpha * alpha;
	float denom = ndh2 * ndh2 * (a2 - 1) + 1;
	return a2 / (3.1415 * denom * denom);
}

// Based on Cook-Torrance microfacet model, with GGX distribution and Smith-GGX shadowing
// Reference: http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
inline float3 SpecularBRDF(float3 pos, float3 nrm, float3 lightDir, float3 viewDir, float roughness)
{
	float3 halfVec = normalize(viewDir + lightDir);

	float ndotv = max(0, dot(nrm, viewDir));
	float ndoth = max(0, dot(nrm, halfVec));
	float ndotl = max(0, dot(lightDir, nrm));
	float hdotv = max(0, dot(viewDir, halfVec));
	float hdotl = max(0, dot(lightDir, halfVec));

	float g = saturate(GGXPartialShadowingTerm(hdotv, roughness) * GGXPartialShadowingTerm(hdotl, roughness));
	float kspec = GGXDistributionTerm(ndoth, roughness);

	float spec = (kspec * g) / (3.1415 * max(0.01, ndotv * ndotl));
	float correction = 1 - pow(1 - saturate(ndotl * ndotv), 6);

	return spec * saturate(correction); // Fresnel applied later
}

// Based on the Oren-Nayar diffuse model, with improvements suggested by https://mimosa-pudica.net/improved-oren-nayar.html
inline float3 DiffuseBRDF(float3 pos, float3 nrm, float3 lightDir, float3 viewDir, float roughness)
{
	float ldotv = max(0, dot(lightDir, viewDir));
	float ndotl = max(0, dot(nrm, lightDir));
	float ndotv = max(0, dot(nrm, viewDir));

	float s = ldotv - ndotl * ndotv;
	float t = s <= 0 ? 1 : max(ndotl, ndotv);

	float kdiff = roughness;
	float3 a = (1 - 0.5 * (kdiff / (kdiff + 0.33))) / 3.1415; //+ 0.17 * color * (kdiff / (kdiff + 0.13))) / 3.1415;
	float3 b = (0.45 * (kdiff / (kdiff + 0.09))) / 3.1415;

	return ndotl * saturate(a + b * (s / t));
}

#endif