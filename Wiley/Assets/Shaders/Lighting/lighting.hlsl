#include "common_lighting.hlsl"

struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

static const float2 fullscreenQuad[6] =
{
    { -1.0f, -1.0f },
    { 1.0f, -1.0f },
    { 1.0f, 1.0f },
    { 1.0f, 1.0f },
    { -1.0f, 1.0f },
    { -1.0f, -1.0f }
};


VertexOutput VSmain(uint vId : SV_VertexID)
{
    VertexOutput output;
    output.position = float4(fullscreenQuad[vId], 0.0f, 1.0f);
    output.uv = output.position.xy * 0.5f + 0.5f;
    output.uv.y = 1.0f - output.uv.y;
    
    return output;
}


//// Pixel Shader /////

Texture2D positionMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D colorMap : register(t2);
Texture2D armMap : register(t3);
SamplerState samplerState : register(s4);
TextureCube irrandiance : register(t5);

StructuredBuffer<Light> lights : register(t1, space1);

float4 PSmain(VertexOutput input) : SV_Target
{
    float3 position = positionMap.Sample(samplerState, input.uv).xyz;
    float3 normal = normalMap.Sample(samplerState, input.uv).xyz;
    float3 albedo = colorMap.Sample(samplerState, input.uv).xyz;
    float4 arm = armMap.Sample(samplerState, input.uv);
    
    float ambientOcclustion = arm.x;
    float roughness = arm.y;
    float metallic = arm.z;
 
    float3 Lo = float3(0.0f, 0.0f, 0.0f);

    float3 N = normalize(normal);

    for (int i = 0; i < lightCompCount; i++)
    {
        Light light = lights[i];
        
        if(light.type == 0)
            continue;
        
        float type = light.type - 1;
        float isSpotLight = type;
        float isPoint = 1.0f - isSpotLight;

        Lo += (isPoint * ComputePointLight(light, position, N, albedo, arm.rgb))
            + (isSpotLight * ComputeSpotLight(light, position, N, albedo, arm.rgb));
    }
    
    if (doIBL)
    {
        float3 V = normalize(cameraPosition.xyz - position);
        float3 F0 = float3(0.04, 0.04, 0.04);
        F0 = lerp(F0, albedo, metallic);
    
        float3 kS = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
        float3 kD = (1.0 - kS) * (1.0 - metallic);

        float3 irradiance = irrandiance.Sample(depthSampler, N).rgb;
        float3 diffuse = irradiance * albedo;
        float3 ambient = kD * diffuse * ambientOcclustion;
    
        float3 _color = ambient + Lo;
        return float4(_color, 1.0f);
    }
    else
    {
        float3 ambient = float3(0.03, 0.03f, 0.03f) * albedo * ambientOcclustion;
        float3 _color = ambient + Lo;
        return float4(_color, 1.0f);
    }
}