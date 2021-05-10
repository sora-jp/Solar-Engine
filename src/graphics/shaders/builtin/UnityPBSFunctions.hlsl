#ifndef _UNITY_PBS_FUNCTIONS
#define _UNITY_PBS_FUNCTIONS



// Ref: http://jcgt.org/published/0003/02/03/paper.pdf
inline float SmithJointGGXVisibilityTerm(float NdotL, float NdotV, float roughness)
{
	// Original formulation:
	//  lambda_v    = (-1 + sqrt(a2 * (1 - NdotL2) / NdotL2 + 1)) * 0.5f;
	//  lambda_l    = (-1 + sqrt(a2 * (1 - NdotV2) / NdotV2 + 1)) * 0.5f;
	//  G           = 1 / (1 + lambda_v + lambda_l);

	// Reorder code to be more optimal
	half a = roughness;
	half a2 = a * a;

	half lambdaV = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
	half lambdaL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);

	// Simplify visibility term: (2.0f * NdotL * NdotV) /  ((4.0f * NdotL * NdotV) * (lambda_v + lambda_l + 1e-5f));
	return 0.5f / (lambdaV + lambdaL + 1e-5f);
}

inline float GGXTerm(float NdotH, float roughness)
{
	float a2 = roughness * roughness;
	float d = (NdotH * a2 - NdotH) * NdotH + 1.0f; // 2 mad
	return a2 / (3.1415 * (d * d + 1e-7f)); // This function is not intended to be running on Mobile,
}

inline float PerceptualRoughnessToSpecPower(float perceptualRoughness)
{
	float m = PerceptualRoughnessToRoughness(perceptualRoughness);   // m is the true academic roughness.
	float sq = max(1e-4f, m * m);
	float n = (2.0 / sq) - 2.0;                          // https://dl.dropboxusercontent.com/u/55891920/papers/mm_brdf.pdf
	n = max(n, 1e-4f);                                  // prevent possible cases of pow(0,0), which could happen when roughness is 1.0 and NdotH is zero
	return n;
}

#endif