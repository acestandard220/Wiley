#include "common.hlsl"

#define MAX_MESH_INSTANCE 512
#define THREAD_PER_GROUP 64

struct AABB
{
    float3 min;
    float3 max;
    float3 pos;
};

struct MeshFilterComponent
{
    uint4 mesh;

    AABB aabb;
    
    uint subMeshCount;
    uint subMeshDataOffset;
    
    uint __padding;
};

struct MeshInstanceBase
{
    uint offset;
    uint size;
};

cbuffer DispatchDetails : register(b0)
{
    uint threadGroupCountX;
    uint threadGroupCountY;
    uint threadGroupCountZ;
};

RWByteAddressBuffer meshFilterIndexPtr : register(u0,space1);
RWStructuredBuffer<MeshFilterComponent> meshFilters : register(u1, space1);

RWStructuredBuffer<MeshInstanceBase> meshInstanceBase : register(u2, space1);
RWStructuredBuffer<uint> meshFilterIndex : register(u3, space1);
RWStructuredBuffer<uint> meshFilterIndexPostOcc : register(u4, space1);

groupshared uint instanceCount;
groupshared uint instanceOffset;
groupshared uint meshFilterIndexes[MAX_MESH_INSTANCE];

groupshared MeshInstanceBase meshBase;

bool IsOccluded(AABB aabb);

//I dispatched a flat thread group(in only the x direction)
//No need for multi-dim groups
[numthreads(THREAD_PER_GROUP, 1, 1)]
void CSmain(ComputeInput input)
{
    uint groupID = input.groupID.x;
    uint groupThreadID = input.groupThreadID.x;
    
    if (groupThreadID == 0)
    {
        meshBase = meshInstanceBase[groupID];

        instanceCount = 0;
        instanceOffset = 0;
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    for (int i = groupThreadID; i < meshBase.size; i += THREAD_PER_GROUP)
    {
        uint index = meshFilterIndex[meshBase.offset + i];

        MeshFilterComponent meshFilter = meshFilters[index];
        if (!IsOccluded(meshFilter.aabb))
        {
            uint writeIndex;
            InterlockedAdd(instanceCount, 1, writeIndex);
            meshFilterIndexes[writeIndex] = index;
        }
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    if (groupThreadID == 0)
    {
        meshFilterIndexPtr.InterlockedAdd(0, instanceCount, instanceOffset);
        MeshInstanceBase mib;
        mib.offset = instanceOffset;
        mib.size = instanceCount;
        meshInstanceBase[groupID] = mib;
    }

    GroupMemoryBarrierWithGroupSync();
    
    for (int j = groupThreadID; j < instanceCount; j += THREAD_PER_GROUP)
    {
        meshFilterIndexPostOcc[instanceOffset + j] = meshFilterIndexes[j];
    }
}

//Implementation will come later.
bool IsOccluded(AABB aabb)
{
    return false;
}