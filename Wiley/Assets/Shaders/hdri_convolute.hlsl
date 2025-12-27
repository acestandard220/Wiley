#include "common.hlsl"

RWTexture2DArray<float4> irradianceMap : register(u0, space0);
TextureCube<float4> inputEM : register(t1, space0);
SamplerState linearSampler : register(s2, space0);

static const float pi = 3.14159265359;

[numthreads(8, 8, 1)]
void Convolute(ComputeInput input)
{
    uint3 threadID = input.dispatchID;
    uint width, height, element;
    irradianceMap.GetDimensions(width, height, element);
    
    if (threadID.x >= width || threadID.y >= height)
        return;
    
    uint face = threadID.z;
    float2 uv = (float2(threadID.xy) + 0.5) / float(width);
    uv = uv * 2.0 - 1.0;
    
    float3 normal;
    if (face == 0)
        normal = normalize(float3(1.0, -uv.y, -uv.x));
    else if (face == 1)
        normal = normalize(float3(-1.0, -uv.y, uv.x));
    else if (face == 2)
        normal = normalize(float3(uv.x, 1.0, uv.y));
    else if (face == 3)
        normal = normalize(float3(uv.x, -1.0, -uv.y));
    else if (face == 4)
        normal = normalize(float3(uv.x, -uv.y, 1.0));
    else
        normal = normalize(float3(-uv.x, -uv.y, -1.0));
    
    float3 up = abs(normal.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangent = normalize(cross(up, normal));
    float3 bitangent = cross(normal, tangent);
    

    float3 irradiance = float3(0, 0, 0);
    float sampleCount = 0.0;
    
    float sampleDelta = 0.025; 
    
    for (float phi = 0.0; phi < 2.0 * pi; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * pi; theta += sampleDelta)
        {
            float3 tangentSample = float3(
                sin(theta) * cos(phi),
                sin(theta) * sin(phi),
                cos(theta)
            );
            
            float3 sampleVec = tangent * tangentSample.x +
                              bitangent * tangentSample.y +
                              normal * tangentSample.z;
            
            irradiance += inputEM.SampleLevel(linearSampler, sampleVec, 0).rgb 
                         * cos(theta) * sin(theta);
            sampleCount++;
        }
    }
    
    irradiance = pi * irradiance / sampleCount;
    irradianceMap[threadID] = normalize(float4(irradiance, 1.0));
}