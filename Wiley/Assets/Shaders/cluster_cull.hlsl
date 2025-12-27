#include "common.hlsl"
#define TILE_SIZE 32

cbuffer DispatchParams : register(b0)
{
    uint4 clusterCount;

    uint2 tileSize;
    uint2 screenDimension;
    
    float nearPlane;
    float farPlane;
};

RWStructuredBuffer<bool> activeClusters : register(u0, space1);
Texture2D depthMap : register(t1, space1);
SamplerState samplerState : register(s2, space1);

uint GetClusterIndex(float2 pixelPosition, float linearDepth);

[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void ClusterCulling(ComputeInput input)
{   
    uint2 pixelCoord = input.dispatchID.xy;
    float depth = depthMap.Load(int3(pixelCoord, 0)).r;

    if (depth >= 0.9999f)
        return;
    
    float linear_depth = LinearizeDepth(depth, nearPlane, farPlane);

    uint clusterIndex = GetClusterIndex(float2(input.dispatchID.xy), linear_depth);

    activeClusters[clusterIndex] = true;
}

uint GetClusterIndex(float2 pixelPosition, float linearDepth)
{
    uint depth_slice = uint(clusterCount.z * log2(linearDepth / nearPlane) / log2(farPlane / nearPlane));
    depth_slice = clamp(depth_slice, 0, clusterCount.z - 1);

    float3 cluster = float3(pixelPosition / tileSize, depth_slice);

    uint clusterIndex = cluster.x +
                         cluster.y * clusterCount.x +
                         cluster.z * (clusterCount.x * clusterCount.y);

    return clusterIndex;
}
