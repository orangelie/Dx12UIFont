void main()
{

}

struct VertexIn
{
	float3 PosL : POSITION;
};

struct VertexOut
{
	float3 PosW : POSITION;
	float4 PosNDC : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	vout.PosW = vin.PosL;
	vout.PosNDC = float4(vout.PosW, 1.0f);

	return vout;
}

float4 PS(VertexOut vin) : SV_TARGET
{
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}