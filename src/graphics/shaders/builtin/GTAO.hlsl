#include "Common.hlsl"
#include "FullScreenQuad.hlsl"

#define PI_HALF 1.5707963267948966192313216916398
#define SSAO_LIMIT 150
#define SSAO_SAMPLES 2
#define SSAO_RADIUS 2
#define SSAO_FALLOFF 1.5
#define SSAO_THICKNESSMIX 0
#define SSAO_MAX_STRIDE 32

Texture2D<float4> _GBPosition;
Texture2D<float4> _GBNormal;

SamplerState smpPoint
{
    AddressU = CLAMP;
    AddressV = CLAMP;
    Filter = MIN_MAG_MIP_POINT;
};

PerMaterial
    // Used to get vector from camera to pixel
    float2 _InvRTSize;
    float _Aspect;
    
    // These are offsets that change every frame, results are accumulated using temporal filtering in a separate shader
    float _AngleOffset;
    float _SpatialOffset;
End
 
// [Eberly2014] GPGPU Programming for Games and Science
inline float GTAOFastAcos(float x)
{
    float res = -0.156583 * abs(x) + PI_HALF;
    res *= sqrt(1.0 - abs(x));
    return x >= 0 ? res : PI - res;
}
 
inline float IntegrateArc(float h1, float h2, float n)
{
    float cosN = cos(n);
    float sinN = sin(n);
    return 0.25 * (-cos(2.0 * h1 - n) + cosN + 2.0 * h1 * sinN - cos(2.0 * h2 - n) + cosN + 2.0 * h2 * sinN);
}
    
inline float3 GetCameraPos(float2 uv, float mip)
{
    // Returns the vector from camera to the specified position on the camera plane (uv argument), located one unit away from the camera
    // This vector is not normalized.
    // The nice thing about this setup is that the returned vector from this function can be simply multiplied with the linear depth to get pixel's position relative to camera position.
    // This particular function does not account for camera rotation or position or FOV at all (since we don't need it for AO)
    // TODO: AO is dependent on FOV, this function is not!
    // The outcome of using this simplified function is that the effective AO range is larger when using larger FOV
    // Use something more accurate to get proper FOV-independent world-space range, however you will likely also have to adjust the SSAO constants below
    return mul(float4(_GBPosition.SampleLevel(smpPoint, uv, mip).xyz, 1), g_View).xyz; //normalize(float3(uv.x * 2.0 - 1.0, uv.y * -2.0 * _Aspect + _Aspect, 1.0));
}
 
inline void SliceSample(float2 tc_base, float2 aoDir, int i, float targetMip, float3 ray, float3 v, Texture2D<float4> _GBPosition, SamplerState _GBPosition_sampler, inout float closest)
{
    float2 uv = tc_base + aoDir * i;
    //float depth = _GBPosition.SampleLevel(_GBPosition_sampler, uv, targetMip).w;
    // Vector from current pixel to current slice sample
    float3 p = GetCameraPos(uv, targetMip) - ray;
    // Cosine of the horizon angle of the current sample
    float current = dot(v, normalize(p));
    // Linear falloff for samples that are too far away from current pixel
    float falloff = saturate((SSAO_RADIUS - length(p)) / SSAO_FALLOFF);
    if (current > closest)
        closest = lerp(closest, current, falloff);
    // Helps avoid overdarkening from thin objects
    //closest = lerp(closest, current, SSAO_THICKNESSMIX * falloff);
}

float4 frag(in v2f i) : SV_Target
{
    uint2 pixel = i.pixelPos.xy;
    float2 tc_original = i.uv;
    
    // Depth of the current pixel
    //float dhere = _GBPosition.Load(int3(pixel, 0)).w;
    // Vector from camera to the current pixel's position
    float3 ray = GetCameraPos(tc_original, 0) /** dhere*/;
    
    //const float normalSampleDist = 1.0;
    
    // Calculate normal from the 4 neighbourhood pixels
    //float2 uv = tc_original + float2(_InvRTSize.x * normalSampleDist, 0.0);
    //float3 p1 = ray - GetCameraVec(uv) * _GBPosition.SampleLevel(_GBPosition_sampler, uv, 0.0).w;
    //
    //uv = tc_original + float2(0.0, _InvRTSize.y * normalSampleDist);
    //float3 p2 = ray - GetCameraVec(uv) * _GBPosition.SampleLevel(_GBPosition_sampler, uv, 0.0).w;
    //
    //uv = tc_original + float2(-_InvRTSize.x * normalSampleDist, 0.0);
    //float3 p3 = ray - GetCameraVec(uv) * _GBPosition.SampleLevel(_GBPosition_sampler, uv, 0.0).w;
    //
    //uv = tc_original + float2(0.0, -_InvRTSize.y * normalSampleDist);
    //float3 p4 = ray - GetCameraVec(uv) * _GBPosition.SampleLevel(_GBPosition_sampler, uv, 0.0).w;
    //
    //float3 normal1 = normalize(cross(p1, p2));
    //float3 normal2 = normalize(cross(p3, p4));
    
    float4 rawnormal = float4(_GBNormal.Load(int3(pixel, 0)).rgb, 0); //normalize(normal1 + normal2);
    float3 normal = normalize(mul(rawnormal, g_View).xyz);
    
    // Calculate the distance between samples (direction vector scale) so that the world space AO radius remains constant but also clamp to avoid cache trashing
    // _InvRTSize = float2(1.0 / sreenWidth, 1.0 / screenHeight)
    float stride = min((1.0 / length(ray)) * SSAO_LIMIT, SSAO_MAX_STRIDE);
    float2 dirMult = _InvRTSize.xy * stride;
    // Get the view vector (normalized vector from pixel to camera)
    float3 v = normalize(-ray);
    
    // Calculate slice direction from pixel's position
    float dirAngle = (PI / 16.0) * ((((pixel.x + pixel.y) & 3) << 2) + (pixel.x & 3)) +_AngleOffset;
    float2 aoDir = dirMult * float2(sin(dirAngle), cos(dirAngle));
    
    // Project world space normal to the slice plane
    float3 toDir = normalize(GetCameraPos(tc_original + aoDir, 0));
    float3 planeNormal = normalize(cross(v, -toDir));
    float3 projectedNormal = normal - planeNormal * dot(normal, planeNormal);
    
    // Calculate angle n between view vector and projected normal vector
    float3 projectedDir = normalize(normalize(toDir) + v);
    float n = GTAOFastAcos(dot(-projectedDir, normalize(projectedNormal))) - PI_HALF;
    
    // Init variables
    float c1 = -1.0;
    float c2 = -1.0;
    
    float2 tc_base = tc_original + aoDir * (0.25 * ((pixel.y - pixel.x) & 3) - 0.375 + _SpatialOffset);
    
    const float minMip = 0.0;
    const float maxMip = 3.0;
    const float mipScale = 1.0 / 12.0;
    
    float targetMip = 0; // floor(clamp(pow(stride, 1.3) * mipScale, minMip, maxMip));
    
    [unroll]
    for (int i = 1; i <= SSAO_SAMPLES; i++)
    {
        SliceSample(tc_base, aoDir, i, targetMip, ray, v, _GBPosition, smpPoint, c2);
        SliceSample(tc_base, aoDir, -i, targetMip, ray, v, _GBPosition, smpPoint, c1);
    }
    
    // Finalize
    float h1a = -GTAOFastAcos(c1);
    float h2a = GTAOFastAcos(c2);
    
    // Clamp horizons to the normal hemisphere
    float h1 = n + max(h1a - n, -PI_HALF);
    float h2 = n + min(h2a - n, PI_HALF);
    
    float ao = lerp(1.0, IntegrateArc(h1, h2, n), length(projectedNormal));
    
    return float4(saturate(ao), length(ray), 0, 1);
}