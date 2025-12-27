#include "common.hlsl"

cbuffer DispatchParams
{
    uint4 clusterCount;   

    uint2 tileSize;
    uint2 screenDimension;
    
    float nearPlane;
    float farPlane;
    
    float3 cameraPosition;
    
    float4x4 inverseProjection;
};

RWStructuredBuffer<Cluster> clusters : register(u0, space1);

[numthreads(1, 1, 1)]
void GenerateClusters(ComputeInput input)
{
    uint clusterIndex = input.dispatchID.x +
                        input.dispatchID.y * (clusterCount.x) +
                        input.dispatchID.z * (clusterCount.x * clusterCount.y);
    
    if (clusterIndex >= clusterCount.x * clusterCount.y * clusterCount.z)
        return;
    
    float4 tile_min_ss = float4(input.dispatchID.xy * tileSize, 1.0f, 1.0f);
    float4 tile_max_ss = float4(float2(input.dispatchID.xy + 1) * tileSize, 1.0f, 1.0f);
    
    float3 tile_min_vs = ScreenToViewSpace(inverseProjection, tile_min_ss, screenDimension).xyz;
    float3 tile_max_vs = ScreenToViewSpace(inverseProjection, tile_max_ss, screenDimension).xyz;
    
    float sliceNearPlane = nearPlane * pow(farPlane / nearPlane, float(input.dispatchID.z) / clusterCount.z);
    float sliceFarPlane = nearPlane * pow(farPlane / nearPlane, float(input.dispatchID.z + 1.0f) / clusterCount.z);

    float3 min_point_near = IntersectLineToZPlane(float3(0, 0, 0), tile_min_vs, sliceNearPlane);
    float3 min_point_far = IntersectLineToZPlane(float3(0, 0, 0), tile_min_vs, sliceFarPlane);
    float3 max_point_near = IntersectLineToZPlane(float3(0, 0, 0), tile_max_vs, sliceNearPlane);
    float3 max_point_far = IntersectLineToZPlane(float3(0, 0, 0), tile_max_vs, sliceFarPlane);
    
    clusters[clusterIndex].min = min(min(min_point_near, min_point_far), min(max_point_near, max_point_far));
    clusters[clusterIndex].max = max(max(min_point_near, min_point_far), max(max_point_near, max_point_far));
}
