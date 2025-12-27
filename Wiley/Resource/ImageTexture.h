#pragma once
#include "Resource.h"
#include "DirectXMath.h"
#include "../RHI/Texture.h"

namespace Wiley {

	enum class MapType {
		Albedo, Normal, Roughness,
		Metalloic, AO
	};

	struct ImageTexture final : public Resource 
	{
		/// <summary>
		/// This is an index into the bindless SRV pool.
		/// </summary>
		UINT srvIndex;

		UINT width;
		UINT height;
		UINT nChannels;
		UINT bitPerChannel;

		MapType mapType;

		RHI::Texture::Ref textureResource;
		ImageTexture()
			:width(0), height(0), nChannels(0), bitPerChannel(0), mapType(MapType::Albedo)
		{
		}
	};

}