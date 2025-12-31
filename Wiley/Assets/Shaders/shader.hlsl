#include "common.hlsl"

cbuffer Constants : register(b1)
{
    float4x4 viewProjection;
    float pad[48];
};

cbuffer ExecuteID : register(b0)
{
    uint drawID;
}

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
    float4 worldPos : POSITION;
    float2 uv : TEXCOORD;
    float3x3 tbn : TBN;
    uint materialID : MATERIALID;
};

StructuredBuffer<MeshFilterComponent> meshFilters : register(t2);
StructuredBuffer<SubMeshData> subMeshData : register(t3);

StructuredBuffer<MeshInstanceBase> meshInstanceBase : register(t0,space4);
StructuredBuffer<uint> meshFilterIndex : register(t1,space4);

VertexOutput VSmain(VertexInput input, uint instanceID : SV_InstanceID)
{
    VertexOutput output;
    
    MeshInstanceBase mib = meshInstanceBase[drawID];
    uint instanceMeshFilterIndex = meshFilterIndex[mib.offset + instanceID];

    SubMeshData vertexSubMeshData = subMeshData[meshFilters[instanceMeshFilterIndex].subMeshDataOffset + input.subMeshID];
    float4x4 modelMatrix = vertexSubMeshData.modelMatrix;
    
    output.worldPos = mul(modelMatrix, float4(input.position, 1.0f));
    output.position = mul(viewProjection, output.worldPos);
    
    output.uv = input.uv;
    output.materialID = vertexSubMeshData.materialID;
    
    //Gram-Schmidt orthogonalization
    float3x3 model3x3 = float3x3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz);
    float3 T = normalize(mul(model3x3, input.tangent.xyz)); 
    float3 N = normalize(mul(model3x3, input.normal));
    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T) * input.tangent.w; 
    
    output.tbn = float3x3(T,B,N);
        
    return output;
}

struct PixelOutput
{
    float4 position : SV_Target;
    float4 normal : SV_Target1;
    float4 albedo : SV_Target2;
    float4 arm : SV_Target3; //ao.roughness.metallic
};


Texture2D<float4> albedoMap[] : register(t0, space1);
Texture2D<float4> normalMap[] : register(t0, space2);
Texture2D<float4> armMap[] : register(t0, space3);

StructuredBuffer<MaterialData> materialData : register(t4);

SamplerState textureSampler : register(s5);

PixelOutput PSmain(VertexOutput input) 
{
    PixelOutput output;

    MaterialData mtlData = materialData[input.materialID];
    
    output.position = input.worldPos;
        
    float3 normalValue = normalMap[mtlData.normal.mapIndex].Sample(textureSampler, input.uv).rgb;
    normalValue = normalValue * 2.0f - 1.0f; 
    normalValue.y = -normalValue.y;
    normalValue.xy *= mtlData.normal.strength; 
    normalValue = normalize(normalValue);
    normalValue = normalize(mul(normalValue, input.tbn));
    output.normal = float4(normalValue, 1.0f);

    output.albedo = albedoMap[mtlData.albedo.mapIndex].Sample(textureSampler, input.uv) * mtlData.albedo.value;

    uint aoChannel = mtlData.ambientOcclusion.valueChannel;
    uint roughnessChannel = mtlData.roughness.valueChannel;
    uint metallicChannel = mtlData.metallic.valueChannel;
    
    float aoValue = armMap[mtlData.ambientOcclusion.mapIndex].Sample(textureSampler, input.uv)[aoChannel] * mtlData.ambientOcclusion.value;
    float roughnessValue = armMap[mtlData.roughness.mapIndex].Sample(textureSampler, input.uv)[roughnessChannel] * mtlData.roughness.value;
    float metallicValue = armMap[mtlData.metallic.mapIndex].Sample(textureSampler, input.uv)[metallicChannel] * mtlData.metallic.value;
    
    output.arm = float4(aoValue, roughnessValue, metallicValue, 1.0f);
    
    return output;
}
