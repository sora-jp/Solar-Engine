#ifndef _FULLSCREEN_QUAD_HLSL_
#define _FULLSCREEN_QUAD_HLSL_

#include "Common.hlsl"

struct v2f
{
    float4 pixelPos     : SV_Position;   // Pixel position on the screen
    float2 normalizedXY : NORMALIZED_XY; // Normalized device XY coordinates [-1,1]x[-1,1]
    float2 uv : TEX_COORD0; // Texture coordinate
    float  instance : INSTANCE_ID;
};

void vert(in uint VertexId : SV_VertexID,
    in uint InstID : SV_InstanceID,
    out v2f VSOut)
{
    float2 PosXY[3];
    PosXY[0] = float2(-1.0, -1.0);
    PosXY[1] = float2(-1.0, +3.0);
    PosXY[2] = float2(+3.0, -1.0);

    float2 f2XY = PosXY[VertexId];
    VSOut.normalizedXY = f2XY;

    VSOut.uv = f2XY * 0.5 + 0.5;
    VSOut.uv.y = 1 - VSOut.uv.y;

    VSOut.instance = float(InstID);

    // Write 0 to the depth buffer
    // NDC_MIN_Z ==  0 in DX
    // NDC_MIN_Z == -1 in GL
    float z = NDC_MIN_Z;
    VSOut.pixelPos = float4(f2XY, z, 1.0);
}

#endif