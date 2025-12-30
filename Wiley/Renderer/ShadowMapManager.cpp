#include "ShadowMapManager.h"
#include "../Scene/Entity.h"


namespace Renderer3D {

	using namespace Wiley;

	ShadowMapManager::ShadowMapManager(RHI::RenderContext::Ref rctx)
		:arrayPtr(0), cubePtr(0), mapPtr(0), rctx(rctx), isAllLightEntitiesDirty(false)
	{
		cubeSrv	 = rctx->AllocateCBV_SRV_UAV(MAX_LIGHTS);
		arraySrv = rctx->AllocateCBV_SRV_UAV(MAX_LIGHTS);

		//Tears in my eyes... 400KB...
		lightViewProjections = std::make_unique<LinearAllocator<DirectX::XMFLOAT4X4>>(MAX_LIGHTS * 6);
		lightViewProjections->Initialize();

	}

	ShadowMapManager::~ShadowMapManager()
	{
		rctx.reset();
		cubeSrv.clear();
		arraySrv.clear();
		depthMaps.fill(nullptr);
	}

	ShadowMapData ShadowMapManager::AllocateTexture(Wiley::LightType type, ShadowMapSize mapSize, const std::string& name)
	{
		uint32_t index;
		const std::string mapName = name + "_ShadowMap";

		if (mapfreelist.size()) {
			index = mapfreelist.front();
			mapfreelist.pop();

			RHI::Texture::Ref depthMap = depthMaps[index];
			
			uint32_t oldWidth = depthMap->GetWidth();
			uint32_t oldHeight = depthMap->GetHeight();
			
			if (oldWidth != mapSize || oldHeight != mapSize) {
				depthMap->Resize(mapSize, mapSize);
			}

			
			depthMap->SetName(mapName);

			uint32_t srvIndex = AllocateSRV(type);
			BuildSRV(type, srvIndex, depthMap);	

			uint32_t matrixIndex = AllocateMatrixSpace(type);
			return {
				.textureIndex = index,
				.srvOffset = srvIndex,
				.vp = matrixIndex
			};
		}

		if (mapPtr + 1 >= MAX_LIGHTS) {
			WILEY_DEBUGBREAK;
			std::cout << "Max light count exceeded" << std::endl;
		}

		index = mapPtr++;
		RHI::Texture::Ref &depthMap = depthMaps[index];
		depthMap = std::make_shared<RHI::Texture>(rctx->GetDevice(), mapSize, mapSize, rctx->GetDescriptorHeaps(), mapName);
		
		uint32_t srvIndex = AllocateSRV(type);
		BuildSRV(type, srvIndex, depthMap);
		uint32_t matrixIndex = AllocateMatrixSpace(type);

		return {
			.textureIndex = index,
			.srvOffset = srvIndex,
			.vp = matrixIndex
		};
	}

	void ShadowMapManager::DeallocateTexture(ShadowMapData data, Wiley::LightType type)
	{
		mapfreelist.push(data.textureIndex);
		uint32_t matrixAllocCount = 0;
		switch (type)
		{
			case Wiley::LightType::Directional: 
			{
				arrayfreelist.push(data.srvOffset);
				matrixAllocCount = 4;
				break;
			}
			case Wiley::LightType::Spot:
			{
				arrayfreelist.push(data.srvOffset);
				matrixAllocCount = 1;
				break;
			}
			case LightType::Point:
			{
				cubefreelist.push(data.srvOffset);
				matrixAllocCount = 6;
				break;
			}
		}

		lightViewProjections->Deallocate(data.vp, matrixAllocCount);

		return;
	}

	void ShadowMapManager::MakeLightEntityDirty(entt::entity entity)
	{
		dirtyLightEntities.push(entity);
	}

	void ShadowMapManager::MakeAllLightEntityDirty()
	{
		isAllLightEntitiesDirty = true;
	}

	void ShadowMapManager::MakeAllLightClean()
	{
		isAllLightEntitiesDirty = false;
	}

	RHI::Texture::Ref ShadowMapManager::GetDepthMap(int index) const
	{
		if (index >= mapPtr)
		{
			std::cout << "Invalid Depth Map ID." << std::endl;
			return nullptr;
		}
		return depthMaps[index];
	}

	RHI::DescriptorHeap::Descriptor ShadowMapManager::GetCubeSRVHead() const
	{
		return *cubeSrv.data();
	}

	RHI::DescriptorHeap::Descriptor ShadowMapManager::GetArraySRVHead() const
	{
		return *arraySrv.data();
	}

	DirectX::XMFLOAT4X4* ShadowMapManager::GetLightProjection(uint32_t index) const
	{
		return lightViewProjections->GetPointerByIndex(index);
	}

	Queue<entt::entity>& ShadowMapManager::GetDirtyEntities()
	{
		return dirtyLightEntities;
	}

	bool ShadowMapManager::IsAllLightEntitiesDirty() const
	{
		return isAllLightEntitiesDirty;
	}

	/// <summary>
	///		This function only returns the index and does not indicate what srv pool to build view into.
	///		Returns an index into the type of srv array based on the type of light.
	///		Directional Lights & Spot lights share one array since they are simple texture arrays.
	///		Point lights use the cube SRV array.
	/// </summary>
	uint32_t ShadowMapManager::AllocateSRV(Wiley::LightType type)
	{
		uint32_t index;
		switch (type)
		{
			case Wiley::LightType::Directional: [[fallthrough]];
			case Wiley::LightType::Spot:
			{
				if (arrayfreelist.size())
				{
					index = arrayfreelist.front();
					arrayfreelist.pop();
				}
				else {
					index = arrayPtr++;
				}
				break;
			}
			case Wiley::LightType::Point:
			{
				if (cubefreelist.size())
				{
					index = cubefreelist.front();
					cubefreelist.pop();
				}
				else {
					index = cubePtr++;
				}
				break;
			}
		}
		return index;
	}

	void ShadowMapManager::BuildSRV(LightType type, int index, RHI::Texture::Ref& depthMap)
	{
		RHI::DescriptorHeap::Descriptor descriptor;
		switch (type) {
			case LightType::Directional: [[fallthrough]];
			case LightType::Spot:
			{
				descriptor = arraySrv[index];
				break;
			}
			case LightType::Point:
			{
				descriptor = cubeSrv[index];
				break;
			}			
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		if (type == Wiley::LightType::Point) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = 1;
		}
		else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.MipLevels = 1;
			srvDesc.Texture2DArray.ArraySize = 6;
			srvDesc.Texture2DArray.FirstArraySlice = 0;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
		}
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		rctx->GetDevice()->GetNative()->CreateShaderResourceView(depthMap->GetResource(), &srvDesc, descriptor.cpuHandle);
	}

	uint32_t ShadowMapManager::AllocateMatrixSpace(Wiley::LightType type)
	{
		int allocCount = 0;
		switch (type) {
			case LightType::Directional:
			{
				allocCount = 4;
				break;
			}
			case LightType::Point:
			{
				allocCount = 6;
				break;
			}
			case LightType::Spot:
			{
				allocCount = 1;
				break;
			}
		}

		auto memBlk = lightViewProjections->Allocate(allocCount);
		return lightViewProjections->GetIndex(memBlk);
	}

}


//Todo: Change all pools to use linear allocator as free list will be handled for you.