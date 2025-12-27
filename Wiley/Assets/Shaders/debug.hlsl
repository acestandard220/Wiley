cbuffer Constants : register(b1)
{
    float4x4 vp;
    float pad[48];
};

cbuffer ExecuteID : register(b0)
{
    uint drawID;
}

struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
};

VertexOutput VSmain(VertexInput input)
{
    VertexOutput output;
   
    output.position = mul(vp, float4(input.position, drawID));
    output.worldPos = input.position;
    output.normal = normalize(input.normal);

    return output;
}

float4 PSmain(VertexOutput input) : SV_Target
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}