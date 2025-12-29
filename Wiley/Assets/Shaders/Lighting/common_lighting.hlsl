#define PI 3.14159265

#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct Light
{
    uint type;

	//Base Parameters
    float3 position;
    float3 color;
    float intensity;
    float radius;

    //Spot Parameters
    float innerRadius;
    float outerRadius;
    float3 spotDirection;
    
    uint textureIndex;
    uint srvIndex;
};

cbuffer Constant : register(b0, space1)
{
    float4 cameraPosition;
    uint doIBL;
    uint lightCompCount;
};

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

float3 ComputeDirectionalLight(Light light, float3 position, float3 N, float3 albedo, float3 arm)
{
    float ambientOcclustion = arm.x;
    float roughness = arm.y;
    float metallic = arm.z;
    
    float3 radiance = light.color.rgb;
    
    float3 V = normalize(cameraPosition.xyz - position);
    float3 L = normalize(light.position - position);
    float3 H = normalize(V + L);
    
    float NdotL = max(dot(N, L), 0.0f);
    float NdotV = max(dot(N, V), 0.0f);
    float NdotH = max(dot(N, H), 0.0f);
    float VdotH = max(dot(V, H), 0.0f);
    
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
    
    float3 F = FresnelSchlick(VdotH, F0);
    float G = GeometrySmith(N, V, L, roughness);
    float NDF = DistributionGGX(N, H, roughness);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);
    float3 specular = numerator / max(denominator, 0.001f);

    float3 Ks = F;
    float3 Kd = (1.0f - Ks) * (1.0f - metallic);
    float3 diffuse = albedo / PI;
    
    float3 Lo = float3((Kd * diffuse + specular) * NdotL * radiance);
    return Lo;
}

float3 ComputePointLight(Light light, float3 position, float3 N, float3 albedo, float3 arm)
{
    float ambientOcclustion = arm.x;
    float roughness = arm.y;
    float metallic = arm.z;
    
    float distance = length(light.position - position.rgb);
    float attenuation = light.intensity / (distance * distance);
    float3 radiance = light.color.rgb * attenuation;
    
    float3 V = normalize(cameraPosition.xyz - position);
    float3 L = normalize(light.position - position);
    float3 H = normalize(V + L);
    
    float NdotL = max(dot(N, L), 0.0f);
    float NdotV = max(dot(N, V), 0.0f);
    float NdotH = max(dot(N, H), 0.0f);
    float VdotH = max(dot(V, H), 0.0f);
    
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
    
    float3 F = FresnelSchlick(VdotH, F0);
    float G = GeometrySmith(N, V, L, roughness);
    float NDF = DistributionGGX(N, H, roughness);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);
    float3 specular = numerator / max(denominator, 0.001f);

    float3 Ks = F;
    float3 Kd = (1.0f - Ks) * (1.0f - metallic);
    float3 diffuse = albedo / PI;
    
    float3 Lo = float3((Kd * diffuse + specular) * NdotL * radiance);
    return Lo;
}

float3 ComputeSpotLight(Light light, float3 position, float3 N, float3 albedo, float3 arm)
{
    float ambientOcclustion = arm.x;
    float roughness = arm.y;
    float metallic = arm.z;
    
    float3 V = normalize(cameraPosition.xyz - position);
    float3 L = normalize(light.position - position);
    float3 H = normalize(V + L);
    
    float theta = dot(L, normalize(-light.spotDirection.rgb));
    float epsilon = light.innerRadius - light.outerRadius;
    float intensity = clamp((theta - light.outerRadius) / epsilon, 0.0, 1.0);
    
    float distance = length(light.position - position.rgb);
    float attenuation = light.intensity / (distance * distance);
    float3 radiance = light.color.rgb * attenuation * intensity;
    
    float NdotL = max(dot(N, L), 0.0f);
    float NdotV = max(dot(N, V), 0.0f);
    float NdotH = max(dot(N, H), 0.0f);
    float VdotH = max(dot(V, H), 0.0f);
    
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metallic);
    
    float3 F = FresnelSchlick(VdotH, F0);
    float G = GeometrySmith(N, V, L, roughness);
    float NDF = DistributionGGX(N, H, roughness);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);
    float3 specular = numerator / max(denominator, 0.001f);

    float3 Ks = F;
    float3 Kd = (1.0f - Ks) * (1.0f - metallic);
    float3 diffuse = albedo / PI;
    
    float3 Lo = float3((Kd * diffuse + specular) * NdotL * radiance);
    return Lo;
}

float3 LinearToHDR10(float3 color)
{
    float3 L = pow(color, 0.1593017578125);
    return pow((0.8359375 + 18.8515625 * L) / (1.0 + 18.6875 * L), 78.84375);
}