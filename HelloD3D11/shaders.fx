//--------------------------------------------------------------------------------------
// File: shaders.fx
//--------------------------------------------------------------------------------------
SamplerState samLinear : register(s0);
Texture2D txDiffuse : register(t0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vLightDir[2];
	float4 vLightColor[2];
	float4 vOutputColor;
}


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    output.Norm = mul( input.Norm, World );
    output.Tex = input.Tex;
    
    return output;
}

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(3)]
void GS(triangle PS_INPUT input[3], inout TriangleStream<PS_INPUT> OutputStream)
{
    PS_INPUT gsout;

    gsout.Norm = input[0].Norm;
    gsout.Tex = input[0].Tex;
    gsout.Pos = input[0].Pos + float4(0.02, 0, 0, 0.0);
    OutputStream.Append(gsout);

    gsout.Norm = input[1].Norm;
    gsout.Tex = input[1].Tex;
    gsout.Pos = input[1].Pos + float4(0.02, 0, 0, 0.0);
    OutputStream.Append(gsout);

    gsout.Norm = input[2].Norm;
    gsout.Tex = input[2].Tex;
    gsout.Pos = input[2].Pos + float4(0.02, 0, 0, 0.0);
    OutputStream.Append(gsout);
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
    float4 finalColor = 0;
    
    float4 diffuseColor = txDiffuse.Sample(samLinear, input.Tex) ;

    //do NdotL lighting for 2 lights
    for(int i=0; i<2; i++)
    {
        finalColor += saturate( dot( (float3)vLightDir[i],input.Norm) * vLightColor[i] );
    }
    finalColor *= diffuseColor;
    finalColor.a = 1;
    return finalColor;
}


//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
float4 PSSolid( PS_INPUT input) : SV_Target
{
    return vOutputColor;
}
