#include "Common.hlsl"
#include "CubemapUtils.hlsl"

TextureCube<float4> _MainTex;
SamplerState sampler_MainTex;

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space floattor to world-space sample floattor
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
	
    float3 samplefloat = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(samplefloat);
}

PerMaterial
    float _Roughness;
End

float4 convolve(float3 N)
{
    float3 R = N;
    float3 V = R;

    const uint SAMPLE_COUNT = 512u;
    float totalWeight = 0.0;
    float3 prefilteredColor = 0;
    
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, _Roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            prefilteredColor += _MainTex.SampleLevel(sampler_MainTex, L, 0).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    
    prefilteredColor = prefilteredColor / totalWeight;
    return float4(prefilteredColor, 1.0);
}

fout frag(in v2f i)
{
    fout o = (fout) 0;
    
    cubecoords c = GetCoords(i);
    
    o.posX = convolve(c.posX);
    o.negX = convolve(c.negX);
    o.posY = convolve(c.posY);
    o.negY = convolve(c.negY);
    o.posZ = convolve(c.posZ);
    o.negZ = convolve(c.negZ);
    
    return o;
}