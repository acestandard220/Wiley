#pragma once
#define NOMINMAX
#include "Resource.h"
#include "../Core/Allocator.h"

#include <iostream>
#include <filesystem>
#include <vector>
#include <limits>
#undef max

#include "DirectXMath.h"

namespace Wiley
{
   
    struct
    {
        DirectX::XMFLOAT4X4 vp;
        float padding[48];
    } typedef cBuffer, cBufferLight;

    struct AABB
    {
        DirectX::XMFLOAT3 min = { FLT_MAX,FLT_MAX,FLT_MAX };
        DirectX::XMFLOAT3 max = { -FLT_MAX,-FLT_MAX,-FLT_MAX };
        DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };

        [[nodiscard]] bool Intersects(const AABB& other) const
        {
            return (min.x <= other.max.x && max.x >= other.min.x) &&
                (min.y <= other.max.y && max.y >= other.min.y) &&
                (min.z <= other.max.z && max.z >= other.min.z);
        }

        [[nodiscard]] bool Intersects(const DirectX::XMFLOAT3 point)
        {
            return (point.x >= min.x && point.x <= max.x) &&
                (point.y >= min.y && point.y <= max.y) &&
                (point.z >= min.z && point.z <= max.z);
        }
    };

    enum class LODDecayType {
        Exponential,
        Linear,
        HalfLife,
    };

    enum NormalType {
        Flat,
        Smooth
    };

    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 uv;
        DirectX::XMFLOAT4 tangent;
        UINT subMeshIndex; //This index indicates the submesh a vertex is from in respect to a parent mesh.
    };

    struct SubMesh
    {
        SIZE_T vertexOffset;
        SIZE_T indexOffset;
        SIZE_T vertexCount;
        SIZE_T indexCount;

        UINT index;
        UINT nameCharCount;
        UINT nameOffset;
    };


    struct MeshInstanceBase {
        uint32_t offset;
        uint32_t size;
    };

    struct Mesh final : public Resource
    {
        MemoryBlock<Vertex> vertexBlock;
        MemoryBlock<UINT> indexBlock;

        std::vector<MemoryBlock<UINT>> lodIndexBlocks;

        UINT vertexCount = 0;
        UINT indexCount = 0;

        std::vector<SubMesh> subMeshes;
        std::vector<AABB> boxes;
        std::string names;
        AABB aabb;

        std::vector<UINT> instanceMeshFilterIndex;
        std::vector<UUID> loadMaterials;
    };

}
