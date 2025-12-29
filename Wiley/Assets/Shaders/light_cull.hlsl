#define DirectionalLight 0
#define PointLight 1
#define SpotLight 2

struct SortedLight
{
    uint index;
    float projected_z;
    float projected_min_z;
    float projected_max_z;
};


struct Light
{
    uint type;

    //Base Parameters
    float3 position;
    float3 color;
    float intensity;
    float radius;

	//Spot Parameters
    float innerRadius;
    float outerRadius;
    float3 spotDirection;
    
    uint textureIndex;
    uint srvIndex;
};

cbuffer Constants : register(b0, space0)
{
    uint lightCount;
    float near;
    float far;
    float3 cameraPosition;
    float4x4 viewProjection;
    float __padding[42];
}

RWStructuredBuffer<Light> lights : register(u0, space1);
RWStructuredBuffer<uint> dirLightIndex : register(u1, space1);
RWByteAddressBuffer dirLightCount : register(u2, space1);
RWStructuredBuffer<SortedLight> sortedLight : register(u3, space1);

[numthreads(64, 1, 1)]
void LightDataUpload(uint3 globalThreadID : SV_DispatchThreadID)
{
    int index = globalThreadID.x;
    if (globalThreadID.x >= lightCount)
        return;
    
    Light light = lights[index];
    
    //Record Directional Light Locations
    if (light.type == DirectionalLight)
    {
        uint dstIndex;
        dirLightCount.InterlockedAdd(0, 1, dstIndex);
        dirLightIndex[dstIndex] = index;
        return;
    }
    
    float3 lightCameraDir = normalize(light.position - cameraPosition);
    
    float4 p = float4(light.position.x, light.position.y, light.position.z, 1.0f);
    float3 p_min = light.position + (lightCameraDir * -light.radius);
    float3 p_max = light.position + (lightCameraDir * light.radius);
    
    float4 proj_p = mul(viewProjection, p);
    float4 proj_p_min = mul(viewProjection, float4(p_min, 1.0));
    float4 proj_p_max = mul(viewProjection, float4(p_max, 1.0));
    
    SortedLight sLight;
    sLight.index = index;
    sLight.projected_z = (-proj_p.z - near) / (far - near);
    sLight.projected_min_z = (-proj_p_min.z - near) / (far - near);
    sLight.projected_max_z = (-proj_p_max.z - near) / (far - near);
    
    sortedLight[index] = sLight;
}


/// =============== Light Sorting ============== \\\


cbuffer BitonicSortConstants : register(b0, space0)
{
    uint totalLightCount;
    uint blockSize;
    uint compareDistance;
    uint sortAscending;
};

RWStructuredBuffer<SortedLight> sortedLightBuffer : register(u0, space1);

[numthreads(256, 1, 1)]
void BitonicSortLightsCS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint threadIndex = dispatchThreadID.x;
    
    if (threadIndex >= totalLightCount / 2)
        return;
    
    uint blockIndex = threadIndex / (blockSize / 2);
    
    uint indexWithinBlock = threadIndex % (blockSize / 2);
    
    uint firstIndex = blockIndex * blockSize + indexWithinBlock;
    uint secondIndex = firstIndex + compareDistance;
    
    if (secondIndex >= totalLightCount)
        return;
    
    SortedLight firstLight = sortedLightBuffer[firstIndex];
    SortedLight secondLight = sortedLightBuffer[secondIndex];
    
    bool isAscendingBlock = ((blockIndex & 1) == 0);
    
    if (blockSize == totalLightCount)
    {
        isAscendingBlock = (sortAscending == 1);
    }
    
    bool firstIsCloser = (firstLight.projected_z < secondLight.projected_z);

    bool shouldSwap = (isAscendingBlock && !firstIsCloser) ||
                      (!isAscendingBlock && firstIsCloser);
    
    if (shouldSwap)
    {
        sortedLightBuffer[firstIndex] = secondLight;
        sortedLightBuffer[secondIndex] = firstLight;
    }
}


/// =============== Light Tile Assignment (TA) ============== \\\

struct Tile
{
        
};

struct AABB
{
    float3 min;
    float3 max;
};

cbuffer TileAssignmentConstant : register(b0, space0)
{
    uint2 dispatchXY;
    uint skipInvisibleLight;
    uint tileSize;

    uint wordCount;
    float nearTA;
    float farTA;
    float screenWidth;

    float screenHeight;
    float4 invisibleBias;

    float4x4 viewMatrixTA;
    float4x4 projectionMatrixTA;

    float _paddingTA[20];
}

RWStructuredBuffer<Light> lightsTA : register(u0, space1);
RWStructuredBuffer<SortedLight> sortedLightBufferTA : register(u1, space1);
RWStructuredBuffer<uint> tile_bits : register(u2, space1);

[numthreads(256, 1, 1)]
void TileAssignment(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    
    if (index >= dispatchXY.x)
        return;
    
    SortedLight sortLight = sortedLightBufferTA[index];
    Light light = lightsTA[sortLight.index];
    
    //Check Camera Visibility
    float4 position = float4(light.position, 1.0f);
    float radius = light.radius;
    
    float3 viewSpacePosition = mul(viewMatrixTA, position).xyz;
    bool isCameraVisible = (viewSpacePosition.z - radius < nearTA);
    
    if (skipInvisibleLight && !isCameraVisible)
        return;
    
    
    AABB aabb;
    aabb.min = float3(1e30, 1e30, 1e30);
    aabb.max = float3(-1e30, -1e30, -1e30);
    
    for (int c = 0; c < 8; c++)
    {
        float3 corner = float3((c % 2) ? 1.f : -1.f, (c & 2) ? 1.f : -1.f, (c & 4) ? 1.f : -1.f);
        corner = corner * radius;
        corner = corner + float3(position.xyz);
        
        float4 corner_vs = mul(viewMatrixTA, float4(corner.xyz, 1.f));
        corner_vs.z = -max(nearTA, -corner_vs.z);

        float4 corner_ndc = mul(projectionMatrixTA, corner_vs);
        corner_ndc = corner_ndc / corner_ndc.w;

        aabb.min.x = min(aabb.min.x, corner_ndc.x);
        aabb.min.y = min(aabb.min.y, corner_ndc.y);
        aabb.max.x = max(aabb.max.x, corner_ndc.x);
        aabb.max.y = max(aabb.max.y, corner_ndc.y);
    }
    
    float4 aabb_final;
    aabb_final.x = aabb.min.x;
    aabb_final.z = aabb.max.x;
    aabb_final.w = -1.0f * aabb.min.y;
    aabb_final.y = -1.0f * aabb.max.y;

    float4 aabb_screen = float4(
        (aabb_final.x * 0.5f + 0.5f) * (screenWidth - 1),
        (aabb_final.y * 0.5f + 0.5f) * (screenHeight - 1),
        (aabb_final.z * 0.5f + 0.5f) * (screenWidth - 1),
        (aabb_final.w * 0.5f + 0.5f) * (screenHeight - 1)
    );
    
    float width = aabb_screen.z - aabb_screen.x;
    float height = aabb_screen.w - aabb_screen.y;
    
    if (width < 0.0001f || height < 0.0001f)
        return;
    
    float min_x = aabb_screen.x;
    float min_y = aabb_screen.y;
    
    float max_x = min_x + width;
    float max_y = min_y + height;
    
    if (min_x > screenWidth || min_y > screenHeight)
        return;
    if (max_x < 0.0f || max_y < 0.0f)
        return;
    
    
    min_x = max(min_x, 0.0f);
    min_y = max(min_y, 0.0f);
    max_x = min(max_x, (float) screenWidth);
    max_y = min(max_y, (float) screenHeight);
    
    float tileSizeInv = 1.0f / (float) tileSize;
    uint tileCountX = screenWidth / tileSize;
    uint tileCountY = screenHeight / tileSize;
    uint tileStride = tileCountX * wordCount;
    
    uint first_tile_x = (uint) (min_x * tileSizeInv);
    uint last_tile_x = min(tileCountX - 1, (uint) (max_x * tileSizeInv));
    
    uint first_tile_y = (uint) (min_y * tileSizeInv);
    uint last_tile_y = min(tileCountY - 1, (uint) (max_y * tileSizeInv));
    
    for (uint y = first_tile_y; y <= last_tile_y; ++y)
    {
        for (uint x = first_tile_x; x <= last_tile_x; ++x)
        {
            uint array_index = y * tileStride + x;
            uint word_index = index / 32;
            uint bit_index = index % 32;
            InterlockedOr(tile_bits[array_index + word_index], (1u << bit_index));
        }
    }
}
