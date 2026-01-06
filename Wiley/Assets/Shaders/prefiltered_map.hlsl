

static const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;
    
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
    
    float3 up = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
    
    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

float3 CubemapDirection(uint face, float2 uv)
{
    float2 coords = uv * 2.0 - 1.0;
    
    float3 dir;
    switch (face)
    {
        case 0:
            dir = float3(1.0, -coords.y, -coords.x);
            break; 
        case 1:
            dir = float3(-1.0, -coords.y, coords.x);
            break;
        case 2:
            dir = float3(coords.x, 1.0, coords.y);
            break;
        case 3:
            dir = float3(coords.x, -1.0, -coords.y);
            break; 
        case 4:
            dir = float3(coords.x, -coords.y, 1.0);
            break;
        case 5:
            dir = float3(-coords.x, -coords.y, -1.0);
            break;
    }
    return normalize(dir);
}

cbuffer PrefilterParams : register(b0)
{
    float roughness;
    uint outputSize;
    uint sampleCount;
    uint currentMip;
};

TextureCube<float4> inputEnvMap : register(t1);
RWTexture2DArray<float4> outputCubemap : register(u2);
SamplerState linearSampler : register(s3);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= outputSize || DTid.y >= outputSize)
        return;
    
    uint face = DTid.z;
    
    float2 uv = (float2(DTid.xy) + 0.5) / float(outputSize);
    
    float3 N = CubemapDirection(face, uv);
    float3 R = N; 
    float3 V = R;
    
    float3 prefilteredColor = float3(0, 0, 0);
    float totalWeight = 0.0;
    
    for (uint i = 0; i < sampleCount; i++)
    {
        float2 Xi = Hammersley(i, sampleCount);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);
        
        float NdotL = max(dot(N, L), 0.0);
        
        if (NdotL > 0.0)
        {
            float3 sampleColor = inputEnvMap.SampleLevel(linearSampler, L, 0).rgb;
            
            prefilteredColor += sampleColor * NdotL;
            totalWeight += NdotL;
        }
    }
    
    prefilteredColor = prefilteredColor / totalWeight;
    
    outputCubemap[uint3(DTid.xy, face)] = float4(prefilteredColor, 1.0);
}