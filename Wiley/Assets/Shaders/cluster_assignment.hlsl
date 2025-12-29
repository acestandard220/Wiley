#include "common.hlsl"
#define THREAD_PER_GROUP 64

#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

#define MAX_LIGHT_PER_CLUSTER 64

struct Light
{
    float3 position;
    float radius;
};

cbuffer DispatchParams : register(b0)
{
    uint lightCount;
    float4x4 view;
};

RWStructuredBuffer<Cluster> clusters : register(u0, space1);
RWStructuredBuffer<uint> activeClusterIndex : register(u1, space1);
RWStructuredBuffer<Light> lights : register(u2, space1);
RWByteAddressBuffer lightOffsetPtr : register(u3, space1);

RWStructuredBuffer<ClusterData> clusterData : register(u4, space1);
RWStructuredBuffer<uint> lightGrid : register(u5, space1);

groupshared uint gLightCount;
groupshared uint gLightOffset;

groupshared uint clusterIndex;
groupshared float3 clusterMin;
groupshared float3 clusterMax;
groupshared uint clusterLightIndex[MAX_LIGHT_PER_CLUSTER];

[numthreads(THREAD_PER_GROUP, 1, 1)]
void ClusterAssignment(ComputeInput input)
{
    uint groupThreadID = input.groupThreadID.x;
    uint groupID = input.groupID.x;
    
    if (groupThreadID == 0)
    {
        gLightCount = 0;
        gLightOffset = 0;
        
        clusterIndex = activeClusterIndex[groupID];
        Cluster cl = clusters[clusterIndex];
        
        clusterMin = cl.min;
        clusterMax = cl.max;        
    }
    
    GroupMemoryBarrierWithGroupSync();

    
    for (int i = groupThreadID; i < lightCount; i += THREAD_PER_GROUP)
    {
        Light light = lights[i];
        
        float3 lightPositionVS = mul(view, float4(light.position, 1.0f)).xyz;

        Sphere lightInfluence;
        lightInfluence.center = lightPositionVS;
        lightInfluence.radius = light.radius;
        
        if (IntersectSphereAABB(lightInfluence, clusterMin, clusterMax))
        {
            uint index;
            InterlockedAdd(gLightCount, 1, index);
            
            if (index < MAX_LIGHT_PER_CLUSTER)
            {
                clusterLightIndex[index] = i;
            }
        }
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    if (groupThreadID == 0)
    {
        gLightCount = min(gLightCount, MAX_LIGHT_PER_CLUSTER);
        lightOffsetPtr.InterlockedAdd(0, gLightCount, gLightOffset);
        
        clusterData[clusterIndex].offset = gLightOffset;
        clusterData[clusterIndex].size = gLightCount;
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    { 
        for (int i = groupThreadID; i < gLightCount; i += THREAD_PER_GROUP)
        {
            lightGrid[gLightOffset + i] = clusterLightIndex[i];
        }
    }

}

