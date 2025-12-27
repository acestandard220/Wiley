#pragma once
#include "Resource.h"
#include "DirectXMath.h"
#include "../RHI/CubeMap.h"

namespace Wiley {

	enum ConvoluteSize : uint32_t {
		Convolute_16x16 = 16, //Okay
		Convolute_32x32  = 32, //Enough
		Convolute_64x64  = 64 //You are probably not the developer and have 128gigabytes of RAM. 
	};

	struct EnvironmentMap final : public Resource{
		uint32_t srvIndex;

		uint32_t width;
		uint32_t height;

		RHI::CubeMap::Ref cubeMap;
		RHI::CubeMap::Ref irradianceMap; //32x32 or 64x64
	};
}