#include "Common.hlsl"

#define MCARLO_ITER_COUNT 4
#define MCARLO_NORMALIZE_FAC (1.0 / (MCARLO_ITER_COUNT - 1))
#define ANGLE_SEARCH_ITER 16

#define SSAO_RADIUS 1.5
#define SSAO_FALLOFF 2.5
#define SSAO_THICKNESSMIX 0.2

inline void SampleAngle(float3 d, float cur, inout float res)
{
    float falloff = saturate((SSAO_RADIUS - length(d)) / SSAO_FALLOFF);
    if (cur > res)
        res = lerp(res, cur, falloff);
    // Helps avoid overdarkening from thin objects
    res = lerp(res, cur, SSAO_THICKNESSMIX * falloff);
}

inline void SearchAngles(float3 viewDir, float3 pos, float2 uv, float2 maxStep, Texture2D<float4> positionTex, SamplerState positionSampler, out float thetaPos, out float thetaNeg)
{
    thetaPos = 0;
    thetaNeg = 0;
    
    [unroll]
    for (uint i = 1; i < ANGLE_SEARCH_ITER; i++)
    {
        float2 step = maxStep * (float(i) / (ANGLE_SEARCH_ITER - 1));
        
        float3 dPos = mul(float4(positionTex.Sample(positionSampler, uv + step).rgb, 1), g_View).xyz - pos;
        SampleAngle(dPos, saturate(dot(viewDir, normalize(dPos))), thetaPos);
        //thetaPos = max(thetaPos, saturate(dot(viewDir, normalize())));

        float3 dNeg = mul(float4(positionTex.Sample(positionSampler, uv - step).rgb, 1), g_View).xyz - pos;
        SampleAngle(dNeg, saturate(dot(viewDir, normalize(dNeg))), thetaNeg);
        //thetaNeg = max(thetaNeg, saturate(dot(viewDir, normalize())));
    }
    
    thetaPos = acos(thetaPos);
    thetaNeg = -acos(thetaNeg);
}

float IntegrateArc(float h1, float h2, float n)
{
    float cosN = cos(n);
    float sinN = sin(n);
    return 0.25 * (-cos(2.0 * h1 - n) + cosN + 2.0 * h1 * sinN - cos(2.0 * h2 - n) + cosN + 2.0 * h2 * sinN);
}

float GTAO(float2 uv, float3 viewDir, float3 normal, float distance, Texture2D<float4> positionTex, SamplerState positionSampler)
{
    float3 pos = positionTex.Sample(positionSampler, uv).xyz;
    pos = mul(float4(pos, 1), g_View).xyz;
    viewDir = normalize(-pos);
    normal = mul(float4(normal, 0), g_View).xyz;
    
    float result = 0;
    
    [unroll]
    for (uint i = 0; i < MCARLO_ITER_COUNT; i++)
    {
        float sAngle = PI * (float(i) / (MCARLO_ITER_COUNT - 1));
        float2 maxStep = float2(cos(sAngle), sin(sAngle)) * 0.35 / max(1, distance);
        
        float tPos, tNeg;
        SearchAngles(viewDir, pos, uv, maxStep, positionTex, positionSampler, tPos, tNeg);
        
        float3 d = pos + float3(maxStep * -pos.z, 0);
        float3 pn = normalize(cross(viewDir, normalize(-d)));
        float3 nn = normal - dot(normal, pn) * pn;
        
        float n = acos(dot(normalize(nn), viewDir)) - PI / 2;
        tNeg = n + min(tNeg - n, -PI / 2);
        tPos = n + min(tPos - n,  PI / 2);
        
        //float3 cNormal = normal; //dot(normal, viewDir) * viewDir;
        float cosndv = saturate(dot(normalize(nn), viewDir));
        float nrmAngle = acos(cosndv);
        float dblSinNrm = 2 * sin(nrmAngle);
        
        float a =
            0.25 * (-cos(2 * tPos - nrmAngle) + cosndv + tPos * dblSinNrm)
          + 0.25 * (-cos(2 * tNeg - nrmAngle) + cosndv + tNeg * dblSinNrm);

        //result += length(nn) * ((1 - cos(tPos)) + (1 - cos(tNeg))) * MCARLO_NORMALIZE_FAC;
        result += lerp(1, IntegrateArc(tNeg, tPos, n), saturate(length(nn))) * MCARLO_NORMALIZE_FAC;
        //result += n * MCARLO_NORMALIZE_FAC;
        
    }

    return result;
}