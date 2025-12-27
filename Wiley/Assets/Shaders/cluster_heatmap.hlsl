#include "common.hlsl"
#define MAX_LIGHT_PER_CLUSTER 64


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

cbuffer Constants : register(b0)
{
    uint4 clusterCount;

    uint2 tileSize;
    
    float nearPlane;
    float farPlane;
};

Texture2D depthMap : register(t0,space1);
StructuredBuffer<ClusterData> clusterData : register(t1,space1);

float3 GetHeatMapColor(float factor);
uint GetClusterIndex(float2 pixelPosition, float linearDepth);
float4 ClusterCullHeatMapmain(VertexOutput input) : SV_Target
{
    uint2 pixelCoord = input.position.xy;
    float depth = depthMap.Load(int3(pixelCoord, 0)).r;
    float linearDepth = LinearizeDepth(depth, nearPlane, farPlane);
    uint clusterIndex = GetClusterIndex(pixelCoord, linearDepth);
    
    uint clusterLightCount = clusterData[clusterIndex].size;
    float normalizedCount = saturate(clusterLightCount / MAX_LIGHT_PER_CLUSTER);
    
    return float4(GetHeatMapColor(normalizedCount), 1.0f);
}

float3 GetHeatMapColor(float factor)
{
    float3 blue = float3(0, 0, 1);
    float3 green = float3(0, 1, 0);
    float3 yellow = float3(1, 1, 0);
    float3 red = float3(1, 0, 0);

    float3 c1 = lerp(blue, green, smoothstep(0.0, 0.33, factor));
    float3 c2 = lerp(green, yellow, smoothstep(0.33, 0.66, factor));
    float3 c3 = lerp(yellow, red, smoothstep(0.66, 1.0, factor));

    return lerp(c1, lerp(c2, c3, step(0.66, factor)), step(0.33, factor));
}

uint GetClusterIndex(float2 pixelPosition, float linearDepth)
{
    float depth_slice_scale = clusterCount.z / log2(farPlane / nearPlane);
    float depth_slice_bias = -(clusterCount.z * log2(nearPlane) / log2(farPlane / nearPlane));
    
    uint depth_slice = uint(max(log2(linearDepth) * depth_slice_scale + depth_slice_bias, 0.0f));

    float3 cluster = float3(pixelPosition / tileSize, depth_slice);

    uint clusterIndex = cluster.x +
                         cluster.y * clusterCount.x +
                         cluster.z * (clusterCount.x * clusterCount.y);

    return clusterIndex;
}

