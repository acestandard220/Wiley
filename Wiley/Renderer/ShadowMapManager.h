#pragma once
#include "../RHI/RenderContext.h"
#include "../Resource/ResourceCache.h"
#include "../Scene/Component.h"

#include <entt.hpp>

#include <string>

namespace Renderer3D
{
	struct ShadowMapData {
		uint32_t textureIndex;
		uint32_t srvOffset;
		uint32_t vp;
	};

	enum ShadowMapSize : uint32_t{
		ShadowMapSize_512  = 512,  //Probably using a dell latitude 5490 
		ShadowMapSize_1024 = 1024, //Okay
		ShadowMapSize_2048 = 2048, //Hight Quality Use
		ShadowMapSize_4096 = 4096, //I know you are rich

		ShadowMapSize_8192 = 8192  //What are you doing.

	};

	class ShadowMapManager
	{
		public:
			using Ref = std::shared_ptr<ShadowMapManager>;
			ShadowMapManager(RHI::RenderContext::Ref rctx);
			~ShadowMapManager();

			WILEY_NODISCARD ShadowMapData AllocateTexture(Wiley::LightType type, ShadowMapSize mapSize = ShadowMapSize::ShadowMapSize_1024, const std::string& name = "ShadowMapTexture");
			void DeallocateTexture(ShadowMapData index,Wiley::LightType type);

			void MakeLightEntityDirty(entt::entity entity);
			void MakeAllLightEntityDirty();
			void MakeAllLightClean();

			WILEY_NODISCARD RHI::Texture::Ref GetDepthMap(int index)const;
			WILEY_NODISCARD RHI::DescriptorHeap::Descriptor GetCubeSRVHead()const;
			WILEY_NODISCARD RHI::DescriptorHeap::Descriptor GetArraySRVHead()const;

			DirectX::XMFLOAT4X4* GetLightProjection(uint32_t index)const;

			Queue<entt::entity>& GetDirtyEntities();
			bool IsAllLightEntitiesDirty()const;

		private:
			uint32_t AllocateSRV(Wiley::LightType type);
			void BuildSRV(Wiley::LightType type, int index, RHI::Texture::Ref& depthMap);

			uint32_t AllocateMatrixSpace(Wiley::LightType type);

		private:
			//Texture Pools
			std::array<RHI::Texture::Ref, MAX_LIGHTS> depthMaps;	
			Queue<int> mapfreelist;
			int mapPtr;

			//SRV pools
			std::vector<RHI::DescriptorHeap::Descriptor> cubeSrv;
			Queue<int> cubefreelist;
			int cubePtr;

			std::vector<RHI::DescriptorHeap::Descriptor> arraySrv;
			Queue<int> arrayfreelist;
			int arrayPtr;

			std::unique_ptr<Wiley::LinearAllocator<DirectX::XMFLOAT4X4>> lightViewProjections;

			Queue<entt::entity> dirtyLightEntities;
			bool isAllLightEntitiesDirty;

			RHI::RenderContext::Ref rctx;
	};

}
