#include "common.hlsl"

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

struct MaterialData
{
    struct AlbedoProperty
    {
        uint mapIndex;
        float4 value;
    } albedo;

    struct NormalProperty
    {
        uint mapIndex;
        float strength;
    } normal;

    struct AmbientOcclusionProperty
    {
        uint mapIndex;
        float value;
        uint valueChannel;
    } ambientOcclusion;

    struct MetallicProperty
    {
        uint mapIndex;
        float value;
        uint valueChannel;
    } metallic;

    struct RoughnessProperty
    {
        uint mapIndex;
        float value;
        uint valueChannel;
    } roughness;

    float2 mapScale;
};

struct MeshInstanceBase
{
    uint offset;
    uint size;
};

struct SubMeshData
{
    float4x4 modelMatrix;
    uint materialID;
};


struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 tangent : TANGENT;
    uint subMeshID : SUBMESHID;
};

struct VertexOutput
{
    float4 position : SV_Position;
};

uint drawID : register(b0);
cbuffer Constants : register(b1)
{
    float4x4 viewProjection;
    float pad[48];
};

StructuredBuffer<MeshFilterComponent> meshFilters : register(t2);
StructuredBuffer<SubMeshData> subMeshData : register(t3);
StructuredBuffer<MeshInstanceBase> meshInstanceBase : register(t4);
StructuredBuffer<uint> meshFilterIndex : register(t5);

VertexOutput VSmain(VertexInput input, uint instanceID : SV_InstanceID)
{
    VertexOutput output;
    
    MeshInstanceBase mib = meshInstanceBase[drawID];
    uint instanceMeshFilterIndex = meshFilterIndex[mib.offset + instanceID];
    
    SubMeshData vertexSubMeshData = subMeshData[meshFilters[instanceMeshFilterIndex].subMeshDataOffset + input.subMeshID];
    float4x4 modelMatrix = vertexSubMeshData.modelMatrix;
    
    float4 worldPos = mul(modelMatrix, float4(input.position, 1.0f));
    output.position = mul(viewProjection, worldPos);

    return output;
}

void EmptyPixelShader()
{
    //Depth Write
}