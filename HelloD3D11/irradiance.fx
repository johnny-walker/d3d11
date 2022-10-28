//--------------------------------------------------------------------------------------
// File: irradiance.fx 
// IBL Diffuse Lighting
//--------------------------------------------------------------------------------------
SamplerState samLinear : register(s0);
TextureCube txCubeMap : register(t0);

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
struct IRR_INPUT
{
    float4 Pos : POSITION;
};

struct IRR_VS_OUTPUT    //output structure for skymap vertex shader
{
    float4 Pos      : SV_POSITION;
    float3 texCoord : TEXCOORD;
};


//--------------------------------------------------------------------------------------
// Vertex Shader: VS_IrrConv
//--------------------------------------------------------------------------------------
IRR_VS_OUTPUT VS_IrrConv(IRR_INPUT input )
{
    IRR_VS_OUTPUT output = (IRR_VS_OUTPUT)0;
    output.Pos = mul( input.Pos, View);
    output.Pos = mul( output.Pos, Projection );

    output.texCoord = input.Pos;
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader: PS_IrrConv 
//--------------------------------------------------------------------------------------
float4 PS_IrrConv(IRR_VS_OUTPUT input) : SV_Target
{
    float PI = 3.14159265359;
    float3 N = normalize(input.texCoord);

    float3 irradiance = float3(0.f, 0.f, 0.f);

    // tangent space calculation from origin point
    float3 up = float3(0.f, 1.f, 0.f);
    float3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.f;
    for (float phi = 0.f; phi < 2.f * PI; phi += sampleDelta)
    {
        for (float theta = 0.f; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            float3 rgb = txCubeMap.Sample(samLinear, sampleVec).rgb;
            irradiance += rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / nrSamples);

    return float4(irradiance, 1.0);
}



