#pragma once
#include "../Core/UUID.h"

#include "../Resource/Geometry.h"

#include <iostream>
#include <string>

#include <Windows.h>
#include <MathHelper.h>

namespace Wiley {

	struct TagComponent {
		std::string tag;
		UUID uuid;

		TagComponent(std::string name)
			:tag(name)
		{	
			uuid = WILEY_GEN_UUID;
		}
	};

	struct TransformComponent {
		DirectX::XMFLOAT3 localPosition = { 0.0f,0.0f,0.0f };

		DirectX::XMFLOAT3 position = { 0.0f,0.0f,0.0f };
		DirectX::XMFLOAT3 rotation = { 0.0f,0.0f,0.0f };
		DirectX::XMFLOAT3 scale = { 1.0f,1.0f,1.0f };

		DirectX::XMFLOAT4X4 modelMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	};

	/// <summary>
	/// This structure contains the specific draw data of every submesh in the scene.
	/// It is indexed by a subMeshDataOffset + subMeshCount of it's parent mesh.
	/// </summary>

	struct SubMeshData {
		DirectX::XMFLOAT4X4 modelMatrix;
		/// <summary>
		/// The materialDataIndex is an index into the materialData pool which only contains the needed material data.(-Resource UUID, states etc. whcih the GPU does not need)
		/// </summary>
		UINT materialDataIndex;
	};

	struct AABB;
	struct MeshFilterComponent {

		UUID mesh = WILEY_INVALID_UUID;

		AABB aabb{};

		//Offset + Count = Index into Submesh Data Buffer/Pool
		UINT subMeshCount = 0;
		UINT subMeshDataOffset = 0;

		UINT padding = 0;
	};

	enum class LightType {
		Directional,
		Point,
		Spot
	};


	//Rasticate
	struct SortedLight
	{
		UINT index;
		float projected_z;
		float projected_min_z;
		float projected_max_z;
	};

	struct LightComponent {
		
		LightType type = LightType::Directional;

		//Base Parameters
		DirectX::XMFLOAT3 position = { 0.0f,0.0f,0.0f };
		DirectX::XMFLOAT3 color = { 1.0f,1.0f,1.0f };
		FLOAT intensity = 1.0f;

		//Spot Parameters
		FLOAT innerRadius = 5.0f;
		FLOAT outerRadius = 30.0f;
		DirectX::XMFLOAT3 spotDirection = { 0.0f,0.0f,-1.0f };

		uint32_t depthMapIndex;
		uint32_t depthMapSrvIndex;
		uint32_t matrixIndex;
	};

}