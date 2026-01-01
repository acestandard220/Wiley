#pragma once
//Created on 11/25/2025 at 18:30.>

#include "../Core/defines.h"
#include "../Core/Utils.h"
#include "../Core/UUID.h"
#include "../Core/Allocator.h"

#include "../RHI/RenderContext.h"

#include "Resource.h"
#include "ResourceLoader.h"
#include "ImageTexture.h"
#include "Material.h"
#include "EnvironmentMap.h"

#include <unordered_map>
#include <queue>
#include <ranges>
#include <variant>

#define MAX_VERTEX_COUNT 10'000'000
#define MAX_INDEX_COUNT  10'000'000

#define MAX_MESH_COUNT 10'000
#define MAX_SUBMESH_COUNT 100'000

#define MAX_MATERIAL_COUNT 512
#define MAX_IMAGETEXTURE_COUNT 512

#define MAX_LIGHTS 1000

template<typename T>
using Queue = std::queue<T>;

namespace Wiley
{
	/// <summary>
	///		This structure gives extra information to the loaders on how to load the resources.
	///		An invalid ID means the resource will be given a new UUID else it will use what ever value it is given.
	/// </summary>


	struct ResourceLoadDesc {

		struct MeshLoadDesc {
			LODDecayType decayType = LODDecayType::Exponential;
			UINT lodCount = 4;
			NormalType normalType = NormalType::Smooth;
		};

		struct ImageTextureLoadDesc {
			MapType type = MapType::Albedo;
		};

		struct EnvironmentMapLoadDesc {
			ConvoluteSize convSize;
		};

		union {
			MeshLoadDesc meshDesc;
			ImageTextureLoadDesc imageTextureDesc;
			EnvironmentMapLoadDesc emDesc;
		} desc;

		bool flipUV = false;
		FileExtension extension;
		UUID id = WILEY_INVALID_UUID;
	};

	class ResourceCache
	{
		struct ResourceDesc {
			ResourceType type;
			filespace::filepath path;
			ResourceState state;
		};

		struct ImageTextureDescriptorManager {
			UINT descriptorPtr = 0;
			std::vector<RHI::DescriptorHeap::Descriptor> descriptors;
			Queue<UINT> freeDescriptors;
		};

		public:
			ResourceCache(RHI::RenderContext::Ref rctx);
			~ResourceCache();

			struct ResourceCacheMeta {

				UINT vertexCount = 0;;
				UINT indexCount = 0;

				UINT subMeshCount = 0;
				UINT meshCount = 0;

				struct MeshMeta
				{
					std::unordered_map<UUID, UINT> vertexStartLocation;
					std::unordered_map<UUID, UINT> indexStartLocation;
				}meshMeta;
			};

			/// <summary>
			///		Loads a resource of the specified ResourceClass from the given filesystem path.
			/// </summary>
			/// <typeparam name="ResourceClass">The concrete resource type to load.</typeparam>
			/// <param name="path">The filesystem path identifying the resource to load.</param>
			/// <returns>
			///		A Resource::Ref that refers to the loaded resource (may be empty or a nullptr if loading fails).
			///		It will return a reference to a resource if it has already been loaded.
			/// </returns>
			template<typename ResourceClass>
				requires IsResourceType<ResourceClass>
			Resource::Ref LoadResource(filespace::filepath path, ResourceLoadDesc& loadDesc);

			void Cache(Resource::Ref resource, const ResourceDesc& resourceDesc, const UUID& id);

			template<typename ResourceClass>
				requires IsResourceType<ResourceClass>
			std::shared_ptr<ResourceClass> GetResource(UUID id) {
				if (resources.find(id) == resources.end())
				{
					std::cout << "Failed to find resource in cache." << std::endl;
					return nullptr;
				}
				return std::static_pointer_cast<ResourceClass>(resources[id]);
			}

			template<typename ResourceClass>
				requires IsResourceType<ResourceClass>
			filespace::filepath GetResourcePath(UUID id) {
				auto resource = GetResource<ResourceClass>(id);
				if (!resource) {
					std::cout << "Cannot get path." << std::endl;
					return "";
				}
				return resource->path;
			}

			template<typename ResourceClass>
				requires IsResourceType<ResourceClass>
			std::string GetResourceName(UUID id) {
				auto resource = GetResource<ResourceClass>(id);
				if (!resource) {
					std::cout << "Cannot get name." << std::endl;
					return "";
				}
				return resource->name;
			}

			ResourceType GetResourceType(UUID id) {
				if (resources.find(id) == resources.end())
				{
					std::cout << "Failed to find resource in cache." << std::endl;
					return ResourceType::Unknown;
				}
				return resources[id]->GetType();
			}

			WILEY_NODISCARD bool IsResourceOfType(UUID resourceID, ResourceType type) {
				return GetResourceType(resourceID) == type;
			}

			template<typename ResourceClass>
				requires IsResourceType<ResourceClass>
			std::vector<std::shared_ptr<ResourceClass>> GetResourceOfType(ResourceType type)
			{
				auto filtered = resources
					| std::views::values
					| std::views::filter([&](auto& r) { return (r->GetType() == type); })
					| std::views::transform([&](auto& r) {
						return std::static_pointer_cast<ResourceClass>(r); 
					});

				return { filtered.begin(), filtered.end() };
			}

			template<typename ResourceClass>
				requires IsResourceType<ResourceClass>
			void UseResource(UUID uuid, int n = 1) {
				auto resource = GetResource<ResourceClass>(uuid);
				if (!resource)return;
				resource->refCount += n;
			}

			template<typename ResourceClass>
				requires IsResourceType<ResourceClass>
			void UnuseResource(UUID uuid, int n = 1) {
				auto resource = GetResource<ResourceClass>(uuid);
				if (!resource)return;
				if (resource->refCount - n >= 0)
					resource->refCount -= n;
			}

			void SetMaterialMap(UUID materialID, UUID imageTextureID, MapType mapType, TextureChannel chanel = TextureChannel::R);

			Resource::Ref GetDefaultImageTexture(MapType type)const;

			Material::Ref GetDefaultMaterial()const;
			Material::Ref GetDefaultRockMaterial()const;

			Resource::Ref GetDefaultCube()const;
			Resource::Ref GetDefaultCylinder()const;
			Resource::Ref GetDefaultSphere()const;

			Resource::Ref GetDefaultEnvironmentMap()const;

			/// <summary>
			///		Returns a const reference to the object's ResourceCacheMeta.
			/// </summary>
			/// <returns>
			///		A const reference to the internal ResourceCacheMeta (the member resourceCacheMeta). 
			///		The reference is read-only and remains valid while the object that owns it exists; the function does not modify the object.
			/// </returns>
			const ResourceCacheMeta& GetCacheMeta()const { return resourceCacheMeta; }
			
			WILEY_NODISCARD const Vertex* GetVertexPoolBasePtr()const {
				return (Vertex*)vertexPool->GetBasePtr();
			}

			WILEY_NODISCARD const UINT* GetIndexPoolBasePtr()const {
				return (UINT*)indexPool->GetBasePtr();
			}

			std::shared_ptr<LinearAllocator<Vertex>> GetVertexPool()const {
				return vertexPool;
			}

			std::shared_ptr<LinearAllocator<UINT>> GetIndexPool()const {
				return indexPool;
			}

			WILEY_NODISCARD std::shared_ptr<LinearAllocator<MaterialData>> GetMaterialDataPool()const {
				return materialDataPool;
			}

			WILEY_NODISCARD RHI::DescriptorHeap::Descriptor GetImageTextureDescriptorStart(MapType type) {
				return GetImageTextureDescriptorManager(type)->descriptors[0];
			}

			WILEY_NODISCARD bool IsVertexIndexDataDiry()const { return isVertexIndexDataDirty; }
			void MakeVertexIndexDataDirty() { isVertexIndexDataDirty = true; }
			void MakeVertexIndexDataClean() { isVertexIndexDataDirty = false; }


		private:
			void LoadDefaultResources();
			UINT GetFreeImageDescriptorIndex(ResourceCache::ImageTextureDescriptorManager* manager);
			ImageTextureDescriptorManager* GetImageTextureDescriptorManager(MapType type);
		private:
			friend class MeshLoader;
			friend class MaterialLoader;
			friend class ImageTextureLoader;
			friend class EnvironmentMapLoader;

			ResourceCacheMeta resourceCacheMeta;

			std::unordered_map<UUID, Resource::Ref> resources;
			std::unordered_map<filespace::filepath, UUID> pathMap;

			//Resource Data Pools
			std::shared_ptr<LinearAllocator<Vertex>> vertexPool;
			std::shared_ptr<LinearAllocator<UINT>> indexPool;

			std::shared_ptr<LinearAllocator<MaterialData>> materialDataPool;

			ImageTextureDescriptorManager albedoManager;
			ImageTextureDescriptorManager normalManager;
			ImageTextureDescriptorManager armManager;

			//Resource Loaders
			MeshLoader* meshLoader;
			MaterialLoader* materialLoader;
			ImageTextureLoader* imageTextureLoader;
			EnvironmentMapLoader* environmentMapLoader;


			RHI::RenderContext::Ref rctx;

			bool isVertexIndexDataDirty = true;
	};


	template<>
	inline Resource::Ref ResourceCache::LoadResource<Mesh>(filespace::filepath path, ResourceLoadDesc& loadDesc)
	{
		if (pathMap.find(path) != pathMap.end())
		{
			std::cout << "Resource has already been loaded." << std::endl;
			return resources[pathMap[path]];
		}

		Resource::Ref resource = meshLoader->LoadFromFile(path, loadDesc);
		if (!resource)
		{
			std::cout << "Failed to load mesh resource." << std::endl;
			return nullptr;
		}
		resource->type = ResourceType::Mesh;

		ResourceDesc resourceDesc = {
			.type = ResourceType::Mesh,
			.path = path,
			.state = ResourceState::SavedOnDisk
		};

		Cache(resource, resourceDesc, loadDesc.id);

		resourceCacheMeta.meshCount++;

		MakeVertexIndexDataDirty();
		return resource;
	}

	template<>
	inline Resource::Ref ResourceCache::LoadResource<Material>(filespace::filepath path, ResourceLoadDesc& loadDesc)
	{
		if (pathMap.find(path) != pathMap.end())
		{
			std::cout << "Material Resource has already been loaded." << std::endl;
			return resources[pathMap[path]];
		}

		Resource::Ref resource = materialLoader->LoadFromFile(path, loadDesc);
		if (!resource)
		{
			std::cout << "Failed to load material resource." << std::endl;
			return nullptr;
		}
		resource->type = ResourceType::Material;

		ResourceDesc resourceDesc = {
			.type = ResourceType::Material,
			.path = path,
			.state = ResourceState::NotOnDisk
		};

		Cache(resource, resourceDesc, loadDesc.id);

		return resource;
	}

	/// <summary>
	/// Guaranteed to return a valid ImageTexture Resource.
	/// If load fails for any reason it will return the default map type for the texture.
	/// The default texture maps for all map types contain values that can be overrided by the values parameter of any material data;
	/// </summary>
	template<>
	inline Resource::Ref ResourceCache::LoadResource<ImageTexture>(filespace::filepath path, ResourceLoadDesc& loadDesc)
	{
		if (pathMap.find(path) != pathMap.end())
		{
			std::cout << "Image Texture Resource has already been loaded." << std::endl;
			return resources[pathMap[path]];
		}

		if (!filespace::Exists(path)) {
			std::cout << "Resource path specified does not exist." << std::endl;
			return nullptr;
		}

		Resource::Ref resource = imageTextureLoader->LoadFromFile(path, loadDesc);
		if (!resource)
		{
			std::cout << "Failed to load Image Texture resource. Using default image texture for the specified type." << std::endl;
			return GetDefaultImageTexture(loadDesc.desc.imageTextureDesc.type);
		}
		resource->type = ResourceType::ImageTexture;

		ResourceDesc resourceDesc = {
			.type = ResourceType::ImageTexture,
			.path = path,
			.state = ResourceState::SavedOnDisk
		};

		Cache(resource, resourceDesc, loadDesc.id);

		return resource;
	}

	template<>
	inline Resource::Ref ResourceCache::LoadResource<EnvironmentMap>(filespace::filepath path, ResourceLoadDesc& loadDesc)
	{
		if (pathMap.find(path) != pathMap.end())
		{
			std::cout << "Environment Map Resource has already been loaded." << std::endl;
			return resources[pathMap[path]];
		}

		if (!filespace::Exists(path)) {
			std::cout << "Resource path specified does not exist." << std::endl;
			return nullptr;
		}

		Resource::Ref resource = environmentMapLoader->LoadFromFile(path, loadDesc);
		if (!resource)
		{
			std::cout << "Failed to load Environment Map resource. Using default Environment Map." << std::endl;
			return nullptr;
		}
		resource->type = ResourceType::EnvironmentMap;

		ResourceDesc resourceDesc = {
		.type = ResourceType::EnvironmentMap,
		.path = path,
		.state = ResourceState::SavedOnDisk
		};

		Cache(resource, resourceDesc, loadDesc.id);

		return resource;

	}

}
