#include "common.hlsl"

RWTexture2DArray<float4> outputCubeMap : register(u0, space0);
Texture2D<float4> inputEquirectMap : register(t1, space0);
SamplerState linearSampler : register(s2, space0);

[numthreads(8, 8, 1)]
void CSmain(ComputeInput input)
{
    uint width, height, elements;
    outputCubeMap.GetDimensions(width, height, elements);
    
    if (input.dispatchID.x >= width || input.dispatchID.y >= height)
        return;
    
    uint face = input.dispatchID.z;
    float2 uv = (float2(input.dispatchID.xy) + 0.5) / float(width);
    uv = uv * 2.0 - 1.0;
    
    
    //Change to some smart math trick later.
    float3 dir;
    if (face == 0)
        dir = float3(1.0, -uv.y, -uv.x);
    else if (face == 1)
        dir = float3(-1.0, -uv.y, uv.x);
    else if (face == 2)
        dir = float3(uv.x, 1.0, uv.y);
    else if (face == 3)
        dir = float3(uv.x, -1.0, -uv.y);
    else if (face == 4)
        dir = float3(uv.x, -uv.y, 1.0);
    else
        dir = float3(-uv.x, -uv.y, -1.0);
    
    dir = normalize(dir);
    
    float phi = atan2(dir.z, dir.x);
    float theta = asin(dir.y);
    
    float2 equiUV;
    equiUV.x = phi / (2.0 * 3.14159265) + 0.5;
    equiUV.y = theta / 3.14159265 + 0.5;
    
    float4 color = inputEquirectMap.SampleLevel(linearSampler, equiUV, 0);
    outputCubeMap[input.dispatchID] = color;
}