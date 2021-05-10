#include "CubemapUtils.hlsl"

TextureCube<float4> _MainTex;
SamplerState sampler_MainTex;

#define PI 3.14159
float4 convolve(float3 normal)
{
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));
    
    float3 irradiance = 0;
    
    float sampleDelta = 0.015;
    float nrSamples = 0.0;
    
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += _MainTex.Sample(sampler_MainTex, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    return float4(irradiance, 1);
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