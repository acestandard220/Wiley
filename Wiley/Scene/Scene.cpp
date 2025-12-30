#include "Scene.h"
#include <optick.h>
#include "Entity.h"
#include "Component.h"
#include "../Resource/EnvironmentMap.h"

#include "Systems/TransformSystem.h"
#include "Systems/MeshFilterSystem.h"
#include "Systems/LightComponentSystem.h"

#include <algorithm>
#include <execution>
#include <span>
#include <ranges>
#include <print>

namespace Wiley
{
	Resource::Ref gResource = nullptr;

	Scene::Scene(RHI::RenderContext::Ref rctx, Renderer3D::ShadowMapManager::Ref shadowMapManager)
		:shadowMapManager(shadowMapManager)
	{
		ZoneScopedN("Scene::Scene");

		camera = std::make_shared<Camera>();
		resourceCache = std::make_shared<ResourceCache>(rctx);
		
		{
			environment.currentEnvirontmentMap = std::static_pointer_cast<EnvironmentMap>(resourceCache->GetDefaultEnvironmentMap());
			environment.backgroundColor = { 1,1,1 };
			environment.doIBL = false;
		}

		subMeshData = std::make_shared<LinearAllocator<SubMeshData>>(MAX_SUBMESH_COUNT);
		subMeshData->Initialize();

		{
			systems.emplace_back(std::make_unique<TransformSystem>(this));
			systems.emplace_back(std::make_unique<MeshFilterSystem>(this));
			systems.emplace_back(std::make_unique<LightComponentSystem>(this));
		}


		auto defaultMaterial = static_cast<Material*>(resourceCache->GetDefaultMaterial().get());
		auto grayRockMaterial = static_cast<Material*>(resourceCache->GetDefaultRockMaterial().get());
		auto subMeshBase = (SubMeshData*)subMeshData->GetBasePtr();

		{
			ResourceLoadDesc loadDesc{};
			loadDesc.extension = FileExtension::TOML;
			gResource = resourceCache->LoadResource<EnvironmentMap>("EM.toml", loadDesc);
		}

		/*auto testP = AddLight("TestPointLight", LightType::Point);
		testP.GetComponent<LightComponent>().position = {
			0.0, 100.0f, 0.0
		};*/

		/*auto testD = AddLight("TestDirLight", LightType::Directional);
		testD.GetComponent<LightComponent>().position = {
			0.0, 0.0, 1.0
		};*/

		auto testS = AddLight("TestSpotLight", LightType::Spot);
		testS.GetComponent<LightComponent>().position = {
			0.0f, 100.0f, 0.0f
		};
		testS.GetComponent<LightComponent>().spotDirection = {
			0.0f,0.0f,1.0f
		};

		/*ResourceLoadDesc loadDesc{};
		loadDesc.extension = FileExtension::OBJ;
		loadDesc.desc.mesh.lodCount = 4;
		Entity sphere = AddModel("P:/Projects/VS/Wiley/Wiley/Assets/Models/Cylinder.obj", loadDesc);
		auto& transform = sphere.GetComponent<TransformComponent>();
		transform.position = {
			0.0, 10.0f, 1.0
		};*/


		if(0)
		{
			ZoneScopedN("LoadStressTest");

			
			ResourceLoadDesc loadDesc{};
			loadDesc.extension = FileExtension::OBJ;
			loadDesc.desc.meshDesc.lodCount = 4;

			int gridSizeX = 8;
			int gridSizeY = 5;
			int gridSizeZ = 7;
			float spacing = 5.0f;

			float offsetX = (gridSizeX - 1) * spacing / 2.0f;
			float offsetY = (gridSizeY - 1) * spacing / 2.0f;
			float offsetZ = (gridSizeZ - 1) * spacing / 2.0f;

			int _i_ = 0;
			for (int x = 0; x < gridSizeX; ++x)
			{

				for (int y = 0; y < gridSizeY; ++y)
				{
					
					for (int z = 0; z < gridSizeZ; ++z)
					{
						Entity sphere = AddModel("P:/Projects/VS/Wiley/Wiley/Assets/Models/Cylinder.obj", loadDesc);
						auto& transform = sphere.GetComponent<TransformComponent>();
						transform.position = {
							x * spacing - offsetX,
							y * spacing - offsetY,
							z * spacing - offsetZ
						};

						if (_i_ % 2 == 0)
							AssignGlobalDefaultMaterial(sphere);
						else
							AssignGlobalMaterial(sphere, grayRockMaterial->GetUUID());
						
						_i_++;
					}
				}
			}
		}
	}

	Scene::~Scene()
	{
		ZoneScopedN("Scene::~Scene");
#ifdef _DEBUG
		std::cout << "Scene Owned Object RefCount => OnDTOR\n";
		std::cout << "Camera::Ref " << camera.use_count() << std::endl;
		std::cout << "ResourceCache::Ref " << resourceCache.use_count() << std::endl;
#endif // _DEBUG

		camera.reset();
		resourceCache.reset();

		subMeshData.reset();
		subMeshMaterialMap.clear();
		registery.clear();
	}

	void Scene::OnUpdate()
	{
		ZoneScopedN("Scene::OnUpdate");

		camera->Update(0.1f);

		std::for_each(systems.begin(), systems.end(), [&](const ISystem::Ptr& system) {
			system->OnUpdate(0.1f);
			});

		//Reset Dirty Flags
		{
			sceneFlags.isCameraDirty = false;
			sceneFlags.isWindowResize = false;
		}
	}

	void Scene::OnResize(uint32_t width, uint32_t height)
	{
		ZoneScopedN("Scene::OnResize");

		if (width == 0 || height == 0) {
			return;
		}

		camera->OnResize(width, height);

		sceneFlags.isWindowResize = true;
	}

	Scene::Ref Scene::CreateScene(RHI::RenderContext::Ref rctx, Renderer3D::ShadowMapManager::Ref shadowMapManager)
	{
		return std::make_shared<Scene>(rctx, shadowMapManager);
	}

	/// <summary>
	///		Base entity creation function. This function will add the default/needed component by every component(Tag & Transform).
	/// </summary>
	Entity& Wiley::Scene::AddEntity(const std::string name)
	{
		entities.push_back({ registery.create(), this });
		Entity& entity = entities.back();

		TransformComponent transform = entity.AddComponent<TransformComponent>();
		TagComponent tag = entity.AddComponent<TagComponent>(name);

		return entity;
	}

	Entity& Scene::AddModel(std::filesystem::path path, ResourceLoadDesc& loadDesc)
	{
		Resource::Ref resource = resourceCache->LoadResource<Mesh>(path, loadDesc);
		Mesh& mesh = *(static_cast<Mesh*>(resource.get()));
		Entity& entity = AddEntity(path.filename().string());

		MeshFilterComponent* meshFilterBase = GetComponentStorage<MeshFilterComponent>();

		MeshFilterComponent& meshFilter = entity.AddComponent<MeshFilterComponent>();
		meshFilter.mesh = resource->GetUUID();
		meshFilter.subMeshCount = mesh.subMeshes.size();
		meshFilter.aabb = mesh.aabb;

		MeshFilterComponent* mFilterPtr = &meshFilter;
		UINT meshFilterIndex = (meshFilterBase) ? (mFilterPtr - meshFilterBase) : 0;
		mesh.instanceMeshFilterIndex.push_back(meshFilterIndex);

		Material::Ref defaultMaterialResource = resourceCache->GetDefaultMaterial();
		Material* defaultMaterial = static_cast<Material*>(defaultMaterialResource.get());
		MaterialData* mtlDataPoolBasePtr = static_cast<MaterialData*>(resourceCache->GetMaterialDataPool()->GetBasePtr());

		MemoryBlock<SubMeshData> memoryBlk = subMeshData->Allocate(meshFilter.subMeshCount);
		meshFilter.subMeshDataOffset = memoryBlk.data() - (SubMeshData*)subMeshData->GetBasePtr();

		std::span<SubMeshData> subMeshDataSpan((SubMeshData*)memoryBlk.data(), meshFilter.subMeshCount);

		std::vector<UUID> materialList(meshFilter.subMeshCount);
		subMeshMaterialMap[entity.GetUUID()] = std::move(materialList);

		DirectX::XMFLOAT4X4 modelMatrix = entity.GetComponent<TransformComponent>().modelMatrix;
		for (int i = 0; i < meshFilter.subMeshCount; i++) {
			SubMeshData* subMeshData = memoryBlk.data() + i;
			subMeshData->modelMatrix = entity.GetComponent<TransformComponent>().modelMatrix;
			const auto& loadSubMeshMtlUUID = mesh.loadMaterials[i];
			AssignMaterial(entity, loadSubMeshMtlUUID, i);
		}

		return entity;
	}

	Entity& Scene::AddLight(const std::string name, LightType type)
	{
		Entity& entity = AddEntity(name);

		LightComponent& lightComponent = entity.AddComponent<LightComponent>();
		lightComponent.color = { 1.0f,1.0f,1.0f };
		lightComponent.innerRadius = 5.0f;
		lightComponent.outerRadius = 30.0f;
		lightComponent.position = { 0.0f,3.0f,1.0f };
		lightComponent.spotDirection = { 0.0f,-1.0f,0.0f };
		lightComponent.type = type;
		lightComponent.intensity = 10000.0f;

		//Allocate Resource for shadow map
		const auto shadowMapData = shadowMapManager->AllocateTexture(type, Renderer3D::ShadowMapSize_1024, name);
		lightComponent.depthMapIndex = shadowMapData.textureIndex;
		lightComponent.depthMapSrvIndex = shadowMapData.srvOffset;
		lightComponent.matrixIndex = shadowMapData.vp;

		shadowMapManager->MakeLightEntityDirty(static_cast<entt::entity>(entity));

		return entities.back();
	}

	void Scene::AssignGlobalMaterial(Entity entity, UUID materialUUID)
	{
		auto materialResource = resourceCache->GetResource<Material>(materialUUID);
		if (!materialResource) {
			std::cout << "Invalid material id." << std::endl;
			return;
		}

		const auto materialDataBase = (MaterialData*)resourceCache->GetMaterialDataPool()->GetBasePtr();
		const auto materialDataPtr = materialResource->dataPtr;

		const UINT materialDataIndex = materialDataPtr - materialDataBase;

		auto subMeshDataBase = (SubMeshData*)subMeshData->GetBasePtr();

		MeshFilterComponent meshFilter = entity.GetComponent<MeshFilterComponent>();
		for (int i = 0; i < meshFilter.subMeshCount; i++)
		{
			auto& subMeshData = subMeshDataBase[meshFilter.subMeshDataOffset + i];
			subMeshData.materialDataIndex = materialDataPtr - materialDataBase;
		}

		{
			auto& entityMaterialList = subMeshMaterialMap[entity.GetUUID()];
			for (auto& sm : entityMaterialList)
			{
				resourceCache->UnuseResource<Material>(sm);
				sm = materialUUID;
			}

			resourceCache->UseResource<Material>(materialUUID, meshFilter.subMeshCount);
		}

	}

	void Scene::AssignMaterial(Entity entity, UUID materialUUID, int subMeshIndex)
	{
		auto materialResource = resourceCache->GetResource<Material>(materialUUID);
		if (!materialResource) {
			return;
		}

		const auto materialDataBase = (MaterialData*)resourceCache->GetMaterialDataPool()->GetBasePtr();
		const auto materialDataPtr = materialResource->dataPtr;

		const UINT materialDataIndex = materialDataPtr - materialDataBase;

		auto subMeshDataBase = (SubMeshData*)subMeshData->GetBasePtr();

		MeshFilterComponent meshFilter = entity.GetComponent<MeshFilterComponent>();
		if (subMeshIndex >= meshFilter.subMeshCount)
			return;

		auto& subMeshData = subMeshDataBase[meshFilter.subMeshDataOffset + subMeshIndex];
		subMeshData.materialDataIndex = materialDataPtr - materialDataBase;

		{
			auto& entityMaterialList = subMeshMaterialMap[entity.GetUUID()];
			auto& sm = entityMaterialList[subMeshIndex];
			resourceCache->UnuseResource<Material>(sm);
			sm = materialUUID;
			resourceCache->UseResource<Material>(materialUUID);
		}
	}

	void Scene::AssignGlobalDefaultMaterial(Entity entity)
	{
		AssignGlobalMaterial(entity, resourceCache->GetDefaultMaterial()->GetUUID());
	}

	void Scene::AssignDefaultMaterial(Entity entity, int subMeshIndex)
	{
		AssignMaterial(entity, resourceCache->GetDefaultMaterial()->GetUUID(), subMeshIndex);
	}

	const Scene::Environment& Scene::GetEnvironment() const
	{
		return environment;
	}

}