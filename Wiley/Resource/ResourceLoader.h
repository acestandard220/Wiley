#pragma once
#define NOMINMAX
#include "Resource.h"
#include "Geometry.h"
#include "ImageTexture.h"
#include "Material.h"

#define SHOULD_INCLUDE_ASSIMP
#ifdef SHOULD_INCLUDE_ASSIMP
	#include <Assimp/Importer.hpp>
	#include <Assimp/scene.h>
	#include <Assimp/postprocess.h>
#undef SHOULD_INCLUDE_ASSIMP
#endif

namespace Wiley {
	
	struct ResourceLoadDesc;

	class MeshLoader 
	{
		public:
			MeshLoader(ResourceCache* resourceCache);

			Resource::Ref LoadFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc);
			Resource::Ref LoadObjFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc);
			Resource::Ref LoadGLTFFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc);
			Resource::Ref LoadWithAssimp(filespace::filepath path, ResourceLoadDesc& loadDesc);

			void SaveToFile(filespace::filepath path, Mesh* meshResource);
			void ProcessNode(aiNode* node, const aiScene* scene, std::vector<Vertex>& vertices, std::vector<UINT>& indices, Mesh& meshData, std::filesystem::path modelDirectory);
		private:
			void GenerateLevelOfDetail(LODDecayType type, UINT lodCount, std::vector<Vertex>& vertices, std::vector<UINT>& indices, Mesh& meshData);
			ResourceCache* resourceCache;
	};

	class MaterialLoader
	{
		public:
			MaterialLoader(ResourceCache* resourceCache);

			Resource::Ref CreateNew(filespace::filepath path);
			Resource::Ref LoadFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc);
			Resource::Ref LoadMTLFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc);
			Resource::Ref LoadTOMLFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc);

			void SaveToFile(filespace::filepath path, Material* material);

		private:
			ResourceCache* resourceCache;
	};

	class ImageTextureLoader {
		public:
			ImageTextureLoader(ResourceCache* resourceCache);

			Resource::Ref LoadFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc);

			Resource::Ref LoadFromDDSFile(filespace::filepath path, ResourceLoadDesc& loadDesc);

			void SaveToFile(filespace::filepath path, ImageTexture* imageTexture);
		private:
			ResourceCache* resourceCache;
	};

	class EnvironmentMapLoader {
	public:
		EnvironmentMapLoader(ResourceCache* resourceCache);

		Resource::Ref LoadFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc);
		Resource::Ref LoadTOMLFromFile(filespace::filepath path, ResourceLoadDesc& loadDesc);

	private:
		ResourceCache* resourceCache;

	};

}