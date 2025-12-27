struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

static const float2 fullscreenQuad[6] =
{
    { -1.0f, -1.0f },
    {  1.0f, -1.0f },
    {  1.0f,  1.0f },
    {  1.0f,  1.0f },
    { -1.0f,  1.0f },
    { -1.0f, -1.0f }
};

VertexOutput VSmain(uint vId : SV_VertexID )
{
    VertexOutput output;
    output.position = float4(fullscreenQuad[vId], 0.0f, 1.0f);
    
    output.uv = saturate(output.position.xy * 0.5f + 0.5f);
    output.uv.y = 1.0f - output.uv.y;
    return output;
}

cbuffer Constants
{
    float3 whitePoint;
    float exposure;
    float gamma;
};

Texture2D positionMap : register(t1);
SamplerState samplerState : register(s2);

float4 PSmain(VertexOutput input) : SV_Target
{
    float3 color = positionMap.Sample(samplerState, input.uv).xyz;

    //color = color * exposure;
    //color = color / (color + whitePoint);
    
    //float invGamma = 1.0 / gamma;
    //color = pow(color, invGamma);
    
    return float4(color, 1.0f);
}

