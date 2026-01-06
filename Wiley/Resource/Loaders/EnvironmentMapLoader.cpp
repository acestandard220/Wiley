#include "../ResourceLoader.h"
#include "../ResourceCache.h"

#include "toml.hpp"
#include "stb_image.h"

#include <fstream>

namespace Wiley {

	Wiley::EnvironmentMapLoader::EnvironmentMapLoader(ResourceCache* resourceCache)
		:resourceCache(resourceCache)
	{
	}

	Resource::Ref EnvironmentMapLoader::LoadFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc)
	{
		switch (loadDesc.extension) {
		case FileExtension::EXR:
			WILEY_DEBUGBREAK;
			return nullptr;
		case FileExtension::HDR:
			return LoadTOMLFromFile(path, loadDesc);
		}
	} 

	Resource::Ref EnvironmentMapLoader::LoadTOMLFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc)
	{
		auto emResource = std::make_shared<EnvironmentMap>();
		const std::string fileName = path.stem().string();

		int width, height, nChannel;
		if (!stbi_is_hdr(path.string().c_str())) {
			std::cout << "ERROR :: provided file is not HDR." << std::endl;
			return nullptr;
		}

		stbi_set_flip_vertically_on_load(loadDesc.flipUV);

		float* data = stbi_loadf(path.string().c_str(), &width, &height, &nChannel, 4);
		if (!data) {
			std::cout << "STB failed to load image. :)\n";
			return nullptr;
		}

		emResource->width = width;
		emResource->height = height;
		
		nChannel = 4;
		
		emResource->cubeMap = resourceCache->rctx->CreateShaderResourceCubeMap(data, width, height, nChannel, 32, fileName);
		emResource->irradianceMap = resourceCache->rctx->ConvoluteCubeMap(emResource->cubeMap, loadDesc.desc.emDesc.convSize, fileName + "_Irrandiance");
		emResource->prefilteredMap = resourceCache->rctx->CreatePrefilteredMap(emResource->cubeMap, PrefilterSize_128x128, "PreFilterMap");
		emResource->brdfLUT = resourceCache->rctx->CreateBRDFLUT(BRDFLUTSize_512x512, "BRDFLUT");

		stbi_image_free(data);
		return emResource;
	}	
} 