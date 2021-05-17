#include "Common.hlsl"
#include "FullScreenQuad.hlsl"

#define PI_HALF 1.5707963267948966192313216916398
#define SSAO_LIMIT 150
#define SSAO_SAMPLES 3
#define SSAO_RADIUS 2.5
#define SSAO_FALLOFF 1.5
#define SSAO_THICKNESSMIX 0
#define SSAO_MAX_STRIDE 32

Texture2D<float4> _GBPosition;
Texture2D<float4> _GBNormal;

SamplerState smpPoint
{
    AddressU = CLAMP;
    AddressV = CLAMP;
    Filter = MIN_MAG_MIP_LINEAR;
};

PerMaterial
    // Used to get vector from camera to pixel
    float2 _InvRTSize;
    float _Aspect;
    
    // These are offsets that change every frame, results are accumulated using temporal filtering in a separate shader
    float _AngleOffset;
    float _SpatialOffset;
    float3 _CameraUp;
    float3 _CameraRight;
End
 
// [Eberly2014] GPGPU Programming for Games and Science
inline half GTAOFastAcos(half x)
{
    half res = -0.156583 * abs(x) + PI_HALF;
    res *= sqrt(1.0 - abs(x));
    return x >= 0 ? res : PI - res;
}
 
inline half IntegrateArc(half h1, half h2, half n)
{
    half cosN = cos(n);
    half sinN = sin(n);
    return 0.25 * (-cos(2.0 * h1 - n) + cosN + 2.0 * h1 * sinN - cos(2.0 * h2 - n) + cosN + 2.0 * h2 * sinN);
}
    
inline half3 GetCameraPos(half2 uv, half mip)
{
    // Returns the vector from camera to the specified position on the camera plane (uv argument), located one unit away from the camera
    // This vector is not normalized.
    // The nice thing about this setup is that the returned vector from this function can be simply multiplied with the linear depth to get pixel's position relative to camera position.
    // This particular function does not account for camera rotation or position or FOV at all (since we don't need it for AO)
    // TODO: AO is dependent on FOV, this function is not!
    // The outcome of using this simplified function is that the effective AO range is larger when using larger FOV
    // Use something more accurate to get proper FOV-independent world-space range, however you will likely also have to adjust the SSAO constants below
    return _GBPosition.SampleLevel(smpPoint, uv, mip).xyz;
}

inline half3 GetViewPos(half2 uv, half mip) 
{
    return mul(half4(GetCameraPos(uv, mip), 1), g_View).xyz;
}
 
inline void SliceSample(half2 tc_base, half2 aoDir, int i, half targetMip, half3 ray, half3 v, inout half closest)
{
    half2 uv = tc_base + aoDir * i;
    //half depth = _GBPosition.SampleLevel(_GBPosition_sampler, uv, targetMip).w;
    // Vector from current pixel to current slice sample
    half3 p = GetCameraPos(uv, targetMip) - ray;
    // Cosine of the horizon angle of the current sample
    half current = dot(v, normalize(p));
    // Linear falloff for samples that are too far away from current pixel
    half falloff = saturate((SSAO_RADIUS - length(p)) / SSAO_FALLOFF);
    closest = lerp(closest, current, falloff * (current > closest));
    // Helps avoid overdarkening from thin objects
    //closest = lerp(closest, current, SSAO_THICKNESSMIX * falloff);
}

float4 frag(in v2f i) : SV_Target
{
    uint2 pixel = i.pixelPos.xy;
    half2 tc_original = i.uv;
    
    // Depth of the current pixel
    //half dhere = _GBPosition.Load(int3(pixel, 0)).w;
    // Vector from camera to the current pixel's position
    half3 ray = GetCameraPos(tc_original, 0) /** dhere*/;
    half3 vsRay = mul(half4(ray, 1), g_View).xyz;
    
    half4 rawnormal = half4(_GBNormal.Sample(smpPoint, i.uv).rgb, 0); //normalize(normal1 + normal2);
    half3 normal = normalize(mul(rawnormal, g_View).xyz);
    
    // Calculate the distance between samples (direction vector scale) so that the world space AO radius remains constant but also clamp to avoid cache trashing
    // _InvRTSize = half2(1.0 / sreenWidth, 1.0 / screenHeight)
    half stride = min((1.0 / length(ray)) * SSAO_LIMIT, SSAO_MAX_STRIDE);
    half2 dirMult = _InvRTSize.xy * stride;
    // Get the view vector (normalized vector from pixel to camera)
    half3 v = normalize(g_WorldSpaceCameraPos - ray);
    half3 vsv = normalize(-vsRay);
    
    // Calculate slice direction from pixel's position
    half dirAngle = (PI / 16.0) * ((((pixel.x + pixel.y) & 3) << 2) + (pixel.x & 3)) +_AngleOffset;
    half2 aoDir = dirMult * half2(sin(dirAngle), cos(dirAngle));
    
    // Project world space normal to the slice plane
    half3 toDir = normalize(GetViewPos(tc_original + aoDir, 0));
    half3 planeNormal = normalize(cross(vsv, -toDir));
    half3 projectedNormal = normal - planeNormal * dot(normal, planeNormal);
    
    // Calculate angle n between view vector and projected normal vector
    half3 projectedDir = normalize(normalize(toDir) + vsv);
    half n = GTAOFastAcos(dot(-projectedDir, normalize(projectedNormal))) - PI_HALF;
    
    // Init variables
    half c1 = -1.0;
    half c2 = -1.0;
    
    half2 tc_base = tc_original + aoDir * (0.25 * ((pixel.y - pixel.x) & 3) - 0.375 + _SpatialOffset);
    
    const half minMip = 0.0;
    const half maxMip = 3.0;
    const half mipScale = 1.0 / 12.0;
    
    half targetMip = 0; // floor(clamp(pow(stride, 1.3) * mipScale, minMip, maxMip));
    
    [unroll]
    for (int i = 1; i <= SSAO_SAMPLES; i++)
    {
        SliceSample(tc_base, aoDir, i, targetMip, ray, v, c2);
        SliceSample(tc_base, aoDir, -i, targetMip, ray, v, c1);
    }
    
    // Finalize
    half h1a = -GTAOFastAcos(c1);
    half h2a = GTAOFastAcos(c2);
    
    // Clamp horizons to the normal hemisphere
    half h1 = n + max(h1a - n, -PI_HALF);
    half h2 = n + min(h2a - n, PI_HALF);
    
    half ao = lerp(1.0, IntegrateArc(h1, h2, n), length(projectedNormal));
    
    return float4(saturate(ao), length(ray), 0, 1);
}