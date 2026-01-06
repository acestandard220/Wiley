#include "ResourceCache.h"
#include "Geometry.h"

namespace Wiley {

	//Default Assets/Resources
	Resource::Ref defaultAoMap = nullptr;
	Resource::Ref defaultAlbedoMap = nullptr;
	Resource::Ref defaultNormalMap = nullptr;
	Resource::Ref defaultMetallicMap = nullptr;
	Resource::Ref defaultRoughnessMap = nullptr;

	Material::Ref defaultMaterial = nullptr;

	Resource::Ref defaultEnvironmentMap = nullptr;


	ResourceCache::ResourceCache(RHI::RenderContext::Ref rctx)
		: rctx(rctx)
	{
		vertexUploadBuffer = rctx->CreateUploadBuffer<Vertex>(MAX_VERTEX_COUNT * WILEY_SIZEOF(Vertex), WILEY_SIZEOF(Wiley::Vertex), "VertexUploadBuffer");
		indexUploadBuffer = rctx->CreateUploadBuffer<UINT>(MAX_INDEX_COUNT * WILEY_SIZEOF(UINT), WILEY_SIZEOF(UINT), "IndexUploadBuffer");

		materialDataPool = std::make_shared<LinearAllocator<MaterialData>>(MAX_MATERIAL_COUNT);
		materialDataPool->Initialize();

		meshLoader = new MeshLoader(this);
		materialLoader = new MaterialLoader(this);
		imageTextureLoader = new ImageTextureLoader(this);
		environmentMapLoader = new EnvironmentMapLoader(this);

		albedoManager.descriptors  = rctx->AllocateCBV_SRV_UAV(MAX_IMAGETEXTURE_COUNT);
		albedoManager.descriptorPtr = 0;

		normalManager.descriptors = rctx->AllocateCBV_SRV_UAV(MAX_IMAGETEXTURE_COUNT);
		normalManager.descriptorPtr = 0;

		armManager.descriptors = rctx->AllocateCBV_SRV_UAV(MAX_IMAGETEXTURE_COUNT * 2);
		armManager.descriptorPtr = 0;

		LoadDefaultResources();
	}

	ResourceCache::~ResourceCache()
	{
		delete meshLoader;
		delete materialLoader;
		delete imageTextureLoader;
		delete environmentMapLoader;
	}

	void ResourceCache::Cache(Resource::Ref resource, const ResourceDesc& resourceDesc, const UUID& id)
	{
		resource->id = (id == WILEY_INVALID_UUID) ? WILEY_GEN_UUID : id;
		resource->name = resourceDesc.path.filename().string();
		resource->path = resourceDesc.path;

		resource->refCount = 0;
		
		resource->state = resourceDesc.state;
		resource->type = resourceDesc.type;

		resources[resource->id] = resource;
		pathMap[resourceDesc.path] = resource->id;
	}


	void ResourceCache::LoadDefaultResources()
	{
		ResourceLoadDesc albedoLoadDesc{};
		albedoLoadDesc.desc.imageTextureDesc.type = MapType::Albedo;
		ResourceLoadDesc normalDesc{};
		normalDesc.desc.imageTextureDesc.type = MapType::Normal;
		ResourceLoadDesc roughnessLoadDesc{};
		roughnessLoadDesc.desc.imageTextureDesc.type = MapType::Roughness;
		ResourceLoadDesc metallicLoadDesc{};
		metallicLoadDesc.desc.imageTextureDesc.type = MapType::Metalloic;
		ResourceLoadDesc aoLoadDesc{};
		aoLoadDesc.desc.imageTextureDesc.type = MapType::AO;

		//Default Maps
		{
			defaultAlbedoMap = LoadResource<ImageTexture>("P:/Projects/VS/Wiley/Wiley/Assets/Default Textures/default_diffuse.png", albedoLoadDesc);
			rctx->GetDevice()->GetRemoveReason();
			defaultNormalMap = LoadResource<ImageTexture>("P:/Projects/VS/Wiley/Wiley/Assets/Default Textures/default_normal.png", normalDesc);
			rctx->GetDevice()->GetRemoveReason();
			defaultAoMap = LoadResource<ImageTexture>("P:/Projects/VS/Wiley/Wiley/Assets/Default Textures/default_ambient_occlusion.png", aoLoadDesc);
			rctx->GetDevice()->GetRemoveReason();
			defaultMetallicMap = LoadResource<ImageTexture>("P:/Projects/VS/Wiley/Wiley/Assets/Default Textures/default_metallic.png", metallicLoadDesc);
			rctx->GetDevice()->GetRemoveReason();
			defaultRoughnessMap = LoadResource<ImageTexture>("P:/Projects/VS/Wiley/Wiley/Assets/Default Textures/default_roughness.png", roughnessLoadDesc);
			rctx->GetDevice()->GetRemoveReason();
		}
		
		//Default Material
		{
			defaultMaterial = materialLoader->CreateNew("Material.toml");
			Material* defaultMtl = static_cast<Material*>(defaultMaterial.get());

			defaultMtl->albedoMap = defaultAlbedoMap->GetUUID();
			defaultMtl->normalMap = defaultNormalMap->GetUUID();
			defaultMtl->ambientOcclusionMap = defaultAoMap->GetUUID();
			defaultMtl->metaillicMap = defaultMetallicMap->GetUUID();
			defaultMtl->roughnessMap = defaultRoughnessMap->GetUUID();

			MaterialData* defaultMtlData = defaultMtl->dataPtr;
			defaultMtlData->albedo.mapIndex = static_cast<ImageTexture*>(defaultAlbedoMap.get())->srvIndex;
			defaultMtlData->albedo.value = { 0.75f,0.75f,0.75f,1.0f };

			defaultMtlData->ambientOcclusion.mapIndex = static_cast<ImageTexture*>(defaultAoMap.get())->srvIndex;
			defaultMtlData->ambientOcclusion.value = 1.0f;
			defaultMtlData->ambientOcclusion.valueChannel = TextureChannel::R;

			defaultMtlData->metallic.mapIndex = static_cast<ImageTexture*>(defaultMetallicMap.get())->srvIndex;
			defaultMtlData->metallic.value = 0.0f;
			defaultMtlData->metallic.valueChannel = TextureChannel::R;

			defaultMtlData->normal.mapIndex = static_cast<ImageTexture*>(defaultNormalMap.get())->srvIndex;
			defaultMtlData->normal.strength = 1.0f;

			defaultMtlData->roughness.mapIndex = static_cast<ImageTexture*>(defaultRoughnessMap.get())->srvIndex;
			defaultMtlData->roughness.value = 1.0f;
			defaultMtlData->roughness.valueChannel = TextureChannel::R;

			Cache(defaultMaterial, { .type = ResourceType::Material, .path = "Material.toml", .state = ResourceState::NotOnDisk }, WILEY_INVALID_UUID);
		}

		{
			ResourceLoadDesc loadDesc{};
			loadDesc.extension = FileExtension::HDR;
			loadDesc.desc.emDesc.convSize = Convolute_64x64;
			loadDesc.flipUV = true;
			defaultEnvironmentMap = LoadResource<EnvironmentMap>("P:/Projects/VS/Wiley/Wiley/Assets/Environment Maps/studio_small_09_4k.hdr", loadDesc);
		}

	}

	UINT ResourceCache::GetFreeImageDescriptorIndex(ResourceCache::ImageTextureDescriptorManager* manager)
	{
		if (!manager->freeDescriptors.empty()) {
			UINT index = manager->freeDescriptors.front();
			manager->freeDescriptors.pop();
			return index;
		}

		if (manager->descriptorPtr + 1 >= MAX_IMAGETEXTURE_COUNT) {
			std::cout << "The maximum image texture count has been reached. Cannot allocate SRV descriptor space." 
				<< std::endl;
			return UINT_MAX;
		}

		return manager->descriptorPtr++;
	}

	ResourceCache::ImageTextureDescriptorManager* ResourceCache::GetImageTextureDescriptorManager(MapType type)
	{
		switch (type) {
			case MapType::Albedo: {
				return &albedoManager;
			}
			case MapType::Normal: {
				return &normalManager;
			}
			case MapType::Metalloic: [[fallthrough]];
			case MapType::AO: [[fallthrough]];
			case MapType::Roughness: {
				return &armManager;
			}
			default: return nullptr;
		}
	}

	void ResourceCache::SetMaterialMap(UUID materialID, UUID imageTextureID, MapType mapType, TextureChannel channel) {
		auto imageTexture = GetResource<ImageTexture>(imageTextureID);
		if (!imageTexture || !IsResourceOfType(imageTextureID, ResourceType::ImageTexture)) {
			std::cout << "Cannot set material map. Invalid Texture ID." << std::endl;
			return;
		}
		auto material = GetResource<Material>(materialID);
		if (!material || !IsResourceOfType(materialID, ResourceType::Material)) {
			std::cout << "Cannot set material map. Invalid Material ID." << std::endl;
			return;
		}

		switch (mapType) {
			case MapType::Albedo:
			{
				material->albedoMap = imageTextureID;
				material->dataPtr->albedo.mapIndex = imageTexture->srvIndex;
				return;
			}
			case MapType::Normal: {
				material->normalMap = imageTextureID;
				material->dataPtr->normal.mapIndex = imageTexture->srvIndex;
				return;
			}
			case MapType::AO: {
				material->ambientOcclusionMap = imageTextureID;
				material->dataPtr->ambientOcclusion.mapIndex = imageTexture->srvIndex;
				material->dataPtr->ambientOcclusion.valueChannel = channel;
				if (imageTexture->srvIndex == 0 || imageTexture->srvIndex == 3)
					//WILEY_DEBUGBREAK;
				return;
			}
			case MapType::Metalloic: {
				material->metaillicMap = imageTextureID;
				material->dataPtr->metallic.mapIndex = imageTexture->srvIndex;
				material->dataPtr->metallic.valueChannel = channel;
				if (imageTexture->srvIndex == 1 || imageTexture->srvIndex == 2)
					//WILEY_DEBUGBREAK;
				return;
			}
			case MapType::Roughness: {
				material->roughnessMap = imageTextureID;
				material->dataPtr->roughness.mapIndex = imageTexture->srvIndex;
				material->dataPtr->roughness.valueChannel = channel;
				if (imageTexture->srvIndex == 1 || imageTexture->srvIndex == 4)
					//WILEY_DEBUGBREAK;
				return;
			}
		}
		return;
	}

	Resource::Ref ResourceCache::GetDefaultImageTexture(MapType type) const
	{
		switch (type) {
			case MapType::Albedo:return defaultAlbedoMap;
			case MapType::Normal:return defaultNormalMap;
			case MapType::Roughness:return defaultRoughnessMap;
			case MapType::Metalloic:return defaultMetallicMap;
			case MapType::AO:return defaultAoMap;
			default: return defaultAlbedoMap;
		}
	}

	
	Material::Ref ResourceCache::GetDefaultMaterial() const
	{
		return defaultMaterial;
	}

	Resource::Ref ResourceCache::GetDefaultEnvironmentMap() const
	{
		return defaultEnvironmentMap;
	}

	WILEY_NODISCARD const RHI::UploadBuffer<Vertex>::Ref& ResourceCache::GetVertexUploadBuffer() const {
		return vertexUploadBuffer;
	}

	WILEY_NODISCARD const RHI::UploadBuffer<UINT>::Ref& ResourceCache::GetIndexUploadBuffer() const {
		return indexUploadBuffer;
	}


}