//--------------------------------------------------------------------------------------
// File: cubemap.fx
//--------------------------------------------------------------------------------------
SamplerState samLinear : register(s0);
Texture2D txHdrMap : register(t0);

//--------------------------------------------------------------------------------------
// Constant Cube Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstCubeBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
}

//--------------------------------------------------------------------------------------
struct SKYMAP_INPUT
{
    float4 Pos : POSITION;
};

struct SKYMAP_VS_OUTPUT    //output structure for skymap vertex shader
{
    float4 Pos      : SV_POSITION;
    float3 texCoord : TEXCOORD;
};


//--------------------------------------------------------------------------------------
// Vertex Shader: VS_Cubemap
//--------------------------------------------------------------------------------------
SKYMAP_VS_OUTPUT VS_Cubemap(SKYMAP_INPUT input )
{
    SKYMAP_VS_OUTPUT output = (SKYMAP_VS_OUTPUT)0;
    output.Pos = mul( input.Pos, View);
    output.Pos = mul( output.Pos, Projection );

    output.texCoord = input.Pos.xyz;
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader: PS_Cubmap (from_HDR)
//--------------------------------------------------------------------------------------
// https://huailiang.github.io/blog/2019/ibl/
float2 SampleSphericalMap(float3 v)
{
    float2 invAtan = float2(0.1591, 0.3183);
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float4 PS_Cubmap(SKYMAP_VS_OUTPUT input) : SV_Target
{
    float3 nrmlPos = normalize(input.texCoord.xyz);
    float2 uv = SampleSphericalMap(nrmlPos);
    float3 color = txHdrMap.Sample(samLinear, uv).rgb;
    return float4(color, 1.f);
}



