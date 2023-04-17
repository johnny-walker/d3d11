//--------------------------------------------------------------------------------------
// File: borders.fx
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Geometry Shader (passthru)
//--------------------------------------------------------------------------------------
[maxvertexcount(2)]
void GS(line PS_INPUT input[2], inout LineStream<PS_INPUT> outStream)
{
    PS_INPUT gsout;
    for (int i = 0; i < 2; i++) {
        gsout.Norm = input[i].Norm;
        gsout.Tex = input[i].Tex;
        gsout.Pos = input[i].Pos;
        outStream.Append(gsout);
    }
}

