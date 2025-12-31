#include "common.hlsl"

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