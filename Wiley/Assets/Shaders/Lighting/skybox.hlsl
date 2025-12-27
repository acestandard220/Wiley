struct SkyboxOutput
{
    float4 position : SV_Position;
    float3 texCoord : TEXCOORD0;
};

cbuffer ViewProjection : register(b0)
{
    float4x4 view;
    float4x4 projection;
    
    float4 bgColor;
};

static const float3 cubeVertices[36] =
{
    // Back face
    { -1, -1, -1 },
    { 1, -1, -1 },
    { 1, 1, -1 },
    { 1, 1, -1 },
    { -1, 1, -1 },
    { -1, -1, -1 },
    // Front face
    { -1, -1, 1 },
    { 1, -1, 1 },
    { 1, 1, 1 },
    { 1, 1, 1 },
    { -1, 1, 1 },
    { -1, -1, 1 },
    // Left face
    { -1, -1, -1 },
    { -1, 1, -1 },
    { -1, 1, 1 },
    { -1, 1, 1 },
    { -1, -1, 1 },
    { -1, -1, -1 },
    // Right face
    { 1, -1, -1 },
    { 1, 1, -1 },
    { 1, 1, 1 },
    { 1, 1, 1 },
    { 1, -1, 1 },
    { 1, -1, -1 },
    // Bottom face
    { -1, -1, -1 },
    { -1, -1, 1 },
    { 1, -1, 1 },
    { 1, -1, 1 },
    { 1, -1, -1 },
    { -1, -1, -1 },
    // Top face
    { -1, 1, -1 },
    { -1, 1, 1 },
    { 1, 1, 1 },
    { 1, 1, 1 },
    { 1, 1, -1 },
    { -1, 1, -1 },
};

SkyboxOutput VSmain(uint vId : SV_VertexID)
{
    float3 pos = cubeVertices[vId];
    SkyboxOutput output;
    
    output.position = float4(pos, 1.0f);
    
    float4x4 vp = mul(projection, view);
    output.position = mul(vp,output.position).xyww;
    output.texCoord = normalize(pos);  
    return output;
}

TextureCube environmentMap : register(t0,space1);
SamplerState samplerState : register(s1,space1);

float4 PSmain(SkyboxOutput input) : SV_Target
{
    if(bgColor.w)
    {
        float3 color = environmentMap.Sample(samplerState, input.texCoord).rgb;
        return float4(color, 1.0);
    }
    else
    {
        return float4(bgColor.rgb, 1.0f);
    }
}
