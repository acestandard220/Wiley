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
    float4 worldPosition : POSITION;
    float3 lightPos : POSITION1;
    float farPlane  : FAR_PLANE;
};

cbuffer ExecuteID : register(b0, space0)
{
    uint drawID;
    uint vpIndex;
    float farPlane;
    uint _pad;
    
    float3 lightPos;
    uint _pad2;
}

StructuredBuffer<MeshFilterComponent> meshFilters : register(t0, space1);
StructuredBuffer<SubMeshData> subMeshData : register(t1, space1);
StructuredBuffer<MeshInstanceBase> meshInstanceBase : register(t2, space1);
StructuredBuffer<uint> meshFilterIndex : register(t3, space1);

StructuredBuffer<float4x4> viewProjections : register(t0, space2);
VertexOutput VSmain(VertexInput input, uint instanceID : SV_InstanceID)
{    
    MeshInstanceBase mib = meshInstanceBase[drawID];
    uint instanceMeshFilterIndex = meshFilterIndex[mib.offset + instanceID];

    SubMeshData vertexSubMeshData = subMeshData[meshFilters[instanceMeshFilterIndex].subMeshDataOffset + input.subMeshID];
    float4x4 modelMatrix = vertexSubMeshData.modelMatrix;
    
    VertexOutput output;
   
    output.worldPosition = mul(modelMatrix, float4(input.position, 1.0f));
    output.position = mul(viewProjections[vpIndex], output.worldPosition);
    output.lightPos = float3(lightPos.x, lightPos.y, lightPos.z);
    output.farPlane = farPlane;    
    return output;
}

float PSmain(VertexOutput input) : SV_Target
{
    float3 fragToLight = float3(input.worldPosition.xyz - input.lightPos);
    float output = length(fragToLight);
    output /= input.farPlane;
    return output;
}