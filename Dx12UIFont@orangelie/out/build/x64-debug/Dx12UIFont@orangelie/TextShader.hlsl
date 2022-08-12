void main() { }


#if defined(MAXLIGHTS)
#undef MAXLIGHTS
#endif
#define MAXLIGHTS 16

struct Light
{
	float3 Strength;
	float FallofStart;
	float3 Direction;
	float FallofEnd;
	float3 Position;
	float SpotPower;
};

Texture2D gTextures[2] : register(t0);

SamplerState gSamPointWrap : register(s0);
SamplerState gSamPointClamp : register(s1);
SamplerState gSamLinearWrap : register(s2);
SamplerState gSamLinearClamp : register(s3);
SamplerState gSamAnisotropicWrap : register(s4);
SamplerState gSamAnisotropicClamp : register(s5);

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gTexTransform;

	uint gMatIndex;
	float3 cbPerObjPadding1;
};

cbuffer cbPerPass : register(b1)
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float4x4 gViewOrtho;

	float4 gAmbientLight;
	float3 gEyePos;
	float gTotalTime;

	Light gLights[MAXLIGHTS];

	float gDeltaTime;
	float3 cbPerPassPadding1;

	float4 gColor;
};

struct VertexIn
{
	float3 PosL		: POSITION;
	float2 TexCoord : TEXCOORD;
};

struct VertexOut
{
	float4 PosNDC	: SV_POSITION;
	float3 PosW		: POSITION;
	float2 TexCoord : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	// Position
	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosW = posW.xyz;
	vout.PosNDC = mul(posW, gViewOrtho);

	// TexCoord
	vout.TexCoord = vin.TexCoord;

	return vout;
}

float4 PS(VertexOut vin) : SV_TARGET
{
	float4 color = gTextures[gMatIndex].Sample(gSamAnisotropicWrap, vin.TexCoord);
	float4 wood = gTextures[1].Sample(gSamAnisotropicWrap, vin.TexCoord);

	if (color.r == 0.0f)
	{
		clip(color.a);
	}
	else
	{
		color.rgb = color.rgb;
		color.a = 1.0f;
	}

	return color;
}