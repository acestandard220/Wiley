cbuffer ConstantBuffer : register(b0)
{
    float4x4 vp;
    uint arrayIndex;
    float pad[47];
}

struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
};

float4 VSmain(VertexInput input) : SV_Position
{
    float4 output = mul(vp, float4(input.position, 1.0));
    return output;
}

void PSmain()
{
    ;
}