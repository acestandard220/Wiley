#include "common.hlsl"


cbuffer DispatchConstants
{
    uint3 clusterCount;    
};

RWStructuredBuffer<bool> activeClusters : register(u0, space1);
RWStructuredBuffer<uint> activeClusterIndex : register(u1, space1);
RWByteAddressBuffer activeClusterCount : register(u2, space1);

[numthreads(1, 1, 1)]
void CompactClusters(ComputeInput input)
{
    uint index = input.dispatchID.x +
                        input.dispatchID.y * (clusterCount.x) +
                        input.dispatchID.z * (clusterCount.x * clusterCount.y);
    
    if (activeClusters[index])
    {
        uint oldIndex;
        activeClusterCount.InterlockedAdd(0, 1, oldIndex);
        activeClusterIndex[oldIndex] = index;
    }
}