//--------------------------------------------------------------------------------------
// File: cubemap.fx
//--------------------------------------------------------------------------------------

Texture2D txHdrMap : register(t0);
SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
}


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
};


//--------------------------------------------------------------------------------------
// Vertex Shader: VS_Cubemap
//--------------------------------------------------------------------------------------
PS_INPUT VS_Cubemap( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader: PS_Cubmap_from_HDR
//--------------------------------------------------------------------------------------
float2 SampleSphericalMap(float3 v)
{
    float2 invAtan = float2(0.1591, 0.3183);
    float2 uv = float2(atan(v.z/v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
float4 PS_Cubmap_from_HDR(PS_INPUT input) : SV_Target
{
    float2 uv = SampleSphericalMap(normalize(input.Pos.xyz));
    float3 color = txHdrMap.Sample(samLinear, uv).rgb;
    return float4(color, 1.0);
}



