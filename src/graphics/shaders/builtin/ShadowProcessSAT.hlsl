#include "FullScreenQuad.hlsl"

#define RADIUS 5

cbuffer Data {
	int _Offset;
	int3 _Discard;
};

SamplerState _MainTex_sampler;
Texture2D<float2> _MainTex;

float2 fragConv(in v2f i) : SV_Target
{
	int2 p = int2(i.pixelPos.xy);
	float d = _MainTex.Load(int3(p, 0)).r;
	float2 dd = float2(ddx(d), ddy(d)); //  
	return float2(d, d * d + 0.25 * dot(dd, dd));
}

float2 fragV(in v2f i) : SV_Target
{
	int2 p = int2(i.pixelPos.xy);
	float2 accumulated = 0;

	[unroll(RADIUS * 2 + 1)]
	for (int j = -RADIUS; j <= RADIUS; j++)
	{
		accumulated += (_MainTex.Load(int3(p.x, p.y + j, 0)));
	}

	return (accumulated) / (RADIUS * 2 + 1);
}

float2 fragH(in v2f i) : SV_Target
{
	int2 p = int2(i.pixelPos.xy);
	float2 accumulated = 0;

	[unroll(RADIUS * 2 + 1)]
	for (int j = -RADIUS; j <= RADIUS; j++)
	{
		accumulated += (_MainTex.Load(int3(p.x + j, p.y, 0)));
	}

	return (accumulated) / (RADIUS * 2 + 1);
}