//--------------------------------------------------------------------------------------
// https://www.braynzarsoft.net/viewtutorial/q16390-20-cube-mapping-skybox
// https://learnopengl.com/Advanced-OpenGL/Cubemaps
// File: skybox.fx (image based lighting)
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
struct SKYMAP_INPUT
{
    float4 Pos : POSITION;
};

struct SKYMAP_VS_OUTPUT    //output structure for skymap vertex shader
{
    float4 Pos : SV_POSITION;
    float3 texCoord : TEXCOORD;
};


//--------------------------------------------------------------------------------------
// Vertex Shader: VS_SkyBox
//--------------------------------------------------------------------------------------
SKYMAP_VS_OUTPUT VS_SkyBox(SKYMAP_INPUT input )
{
    //remove the translation part of the view matrix 
    //so only rotation will affect the skybox's position vectors
    matrix rotateView = View;
    rotateView[0][3] = 0.f;
    rotateView[1][3] = 0.f;
    rotateView[2][3] = 0.f;
    rotateView[3][3] = 1.f;
    rotateView[3][0] = 0.f;
    rotateView[3][1] = 0.f;
    rotateView[3][2] = 0.f;

    SKYMAP_VS_OUTPUT output = (SKYMAP_VS_OUTPUT)0;
    output.Pos = mul(input.Pos, rotateView);
    output.Pos = mul(output.Pos, Projection);
    output.Pos = output.Pos.xyww;

    output.texCoord = input.Pos;
    return output;
 }


//--------------------------------------------------------------------------------------
// Pixel Shader: PS_SkyBox
//--------------------------------------------------------------------------------------
float4 PS_SkyBox(SKYMAP_VS_OUTPUT input) : SV_Target
{
    //float3 envColor = float3(0.f, 1.f, 1.f);
    //return float4(envColor, 1.f);

    float3 envColor = txCubeMap.SampleLevel(samLinear, input.texCoord, 0).rgb;

    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + float3(1.f, 1.f, 1.f));
    envColor = pow(envColor, float3(1.f/2.2, 1.f/2.2, 1.f/2.2));

    return float4(envColor, 1.f);
}



