#pragma once
#include "../RHI/RenderContext.h"
#include "../Resource/ResourceCache.h"
#include "../Scene/Component.h"

namespace Renderer3D
{
	struct ShadowMapData {
		uint32_t textureIndex;
		uint32_t srvOffset;
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

			WILEY_NODISCARD ShadowMapData AllocateTexture(Wiley::LightType type, ShadowMapSize mapSize = ShadowMapSize::ShadowMapSize_1024);
			void DeallocateTexture(ShadowMapData index,Wiley::LightType type);

			WILEY_NODISCARD RHI::Texture::Ref GetDepthMap(int index)const;
			WILEY_NODISCARD RHI::DescriptorHeap::Descriptor GetCubeSRVHead()const;
			WILEY_NODISCARD RHI::DescriptorHeap::Descriptor GetArraySRVHead()const;

		private:
			uint32_t AllocateSRV(Wiley::LightType type);
			void BuildSRV(Wiley::LightType type, int index, RHI::Texture::Ref& depthMap);

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

			RHI::RenderContext::Ref rctx;
	};

}
