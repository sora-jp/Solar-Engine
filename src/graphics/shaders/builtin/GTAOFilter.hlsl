#include "Common.hlsl"

RWTexture2D<float2> _AOOut;
 
//#include sharedUniformsBuffer.glsl
/*
The only uniforms needed from this UBO are:
 
float3 cameraRay00
float3 cameraRay01
float3 cameraRay10
float3 cameraRay11
 
The are the rays that make up the "corners" of the screen
They are not normalized, but rather each is a vector from camera to a plane that is parallel to the camera near and far planes and is placed exactly one unit away from the camera.
This way one can just interpolate these rays to get such a ray for the current pixel
And by mutliplying this result with linear depth we get the world space position (relative to the camera)
See line 123
*/

Texture2D<float2> _AODepthCur;
Texture2D<float2> _AODepthHist;

#define _DepthMip 0

PerMaterial
    //int _DepthMip;

    float4x4 _CameraVPDelta;
    float4x4 _CameraRays;
End
 
// A trick to avoid integer division
// 5958 = (2^16) / 11
int IntegerDivideBy_11(int i)
{
    return (i * 5958) >> 16;
}
 
int2 IntModAndDiv_11(int i)
{
    int2 v = int2(i, IntegerDivideBy_11(i));
    v.x -= v.y * 11;
    return v;
}
 
float CheckRange(float2 tc)
{
    if (tc.x <= 0.0 || tc.y <= 0.0 || tc.x >= 1.0 || tc.y >= 1.0)
        return 0.0;
    return 1.0;
}
 
#define SIZEXY 8
#define OFFSET_FILTER int2(-1, -1)
#define OFFSET_TEXEL int2(0, 0)
 
groupshared float aoSamples[SIZEXY + 3][SIZEXY + 3];
groupshared float depthSamples[SIZEXY + 3][SIZEXY + 3];
 
[numthreads(SIZEXY, SIZEXY, 1)]
void compute(int3 threadOffset : SV_GroupThreadID, int3 globalId : SV_DispatchThreadID, int3 group : SV_GroupID)
{
    int2 texel = globalId.xy;
    
    // Load texels for spacial filter
    int threadID = threadOffset.y * 8 + threadOffset.x;
    
    // Preload all needed texels into shared memory to save texture reads
    // Load 11x11 texels because the thread group size is 8 and we need
    // extra 3 border texels because of the 4x4 spacial filter
    
    // Load first 64 samples
    int2 groupTexel = int2(group.xy * 8);
    int2 local = IntModAndDiv_11(threadID);
    
    aoSamples[local.x][local.y] = _AODepthCur.Load(int3(groupTexel + OFFSET_FILTER + local, 0)).x;
    depthSamples[local.x][local.y] = _AODepthCur.Load(int3(((groupTexel + OFFSET_FILTER + local)) + OFFSET_TEXEL, _DepthMip)).y;
 
    // Load the remaining 57 samples (57 = 11 * 11 - 64)
    if (threadID < 57)
    {
        local = IntModAndDiv_11(threadID + 64);
        aoSamples[local.x][local.y] = _AODepthCur.Load(int3(groupTexel + OFFSET_FILTER + local, 0)).x;
        depthSamples[local.x][local.y] = _AODepthCur.Load(int3(((groupTexel + OFFSET_FILTER + local)) + OFFSET_TEXEL, _DepthMip)).y;
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    // Spatial filter
    
    // Get the depth of the "center" sample - this reference depth is used to weight the other samples
    float depth = depthSamples[threadOffset.x - OFFSET_FILTER.x][threadOffset.y - OFFSET_FILTER.y];
    float weightsSpacial = 0.0;
    float aoLocal = 0.0;
    
    [unroll]
    for (int y = 0; y < 4; y++)
    {
        [unroll]
        for (int x = 0; x < 4; x++)
        {
            // Weight each sample by its distance from the refrence depth - but also scale the weight by 1/10 of the reference depth so that the further from the camera the samples are, the higher the tolerance for depth differences is
            
            float localWeight = max(0.0, 5.0 - abs(depthSamples[threadOffset.x + x][threadOffset.y + y] - depth) / (depth * 0.1));
            weightsSpacial += localWeight;
            aoLocal += aoSamples[threadOffset.x + x][threadOffset.y + y].x * localWeight;
        }
    }
    
    aoLocal /= weightsSpacial;
    
    //// Temporal filter
    //uint3 sz;
    //_AODepthHist.GetDimensions(_DepthMip, sz.x, sz.y, sz.z);
    
    //// Get history tc and depth
    //float2 textureCoords = (texel + 0.5) / sz.xy;
    //depth = _AODepthCur.Load(int3(texel, _DepthMip)).y;
    
    //// Reconstruct position from depth
    //// Note that the position is relative to the camera position (not an absolute world space position)
    //float4 pos = float4(lerp(lerp(_CameraRays[1].xyz, _CameraRays[3].xyz, textureCoords.x), lerp(_CameraRays[0].xyz, _CameraRays[2].xyz, textureCoords.x), textureCoords.y) * depth, 1.0);
    
    //// Get the linear depth of the projected position in the last frame
    ////float depthProjected = abs(dot(pos, cameraPlanePrevious));
    //// Project the position using last frame's projection
    //// Note that the matrix should not contain camera translation (becuase of the lack of absolute world space position)
    //// Instead the matrix must contain the relative camera translation since the last frame
    //pos = mul(pos, _CameraVPDelta);
    //pos /= pos.w;
    //float2 tcProjected = pos.xy * 0.5 + 0.5;
    //tcProjected.y = 1 - tcProjected.y;
 
    //float depthProjected = depth;
    
    //float ao = 0.0;
    //float temporalWeight = CheckRange(tcProjected);
    //// Reject history samples that are too far from current sample - same as in spacial filter
    
    //float2 lastAODepth = _AODepthHist.Load(int3(tcProjected * sz.xy, _DepthMip));
    //temporalWeight *= max(0.0, 1.0 - abs(lastAODepth.y - depthProjected) / (depthProjected * 0.1));
    //ao = lastAODepth.x;
    //ao = lerp(aoLocal, ao, temporalWeight);
    
    //ao = lerp(aoLocal, ao, 0.9);
    
    _AOOut[texel] = float2(aoLocal, depth);
}