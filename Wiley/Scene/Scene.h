#pragma once
#include "Systems/TransformSystem.h"
#include "Systems/MeshFilterSystem.h"

#include "../Resource/Geometry.h"
#include "../Resource/ResourceCache.h"

#include "../RHI/RenderContext.h"


#include "../Renderer/ShadowMapManager.h"


#include "Camera.h"
#include "Component.h"

#include "entt.hpp"

#include <memory>
#include <vector>

#include <Windows.h>

namespace Wiley
{
	class Entity;
	class Scene {
		struct Environment {
			std::shared_ptr<EnvironmentMap> currentEnvirontmentMap;
			DirectX::XMFLOAT3 backgroundColor;

			/// <summary>
			///		Controls the brightness threshold where colors saturate to white
			///		The higher value, the high the threshold for saturating bright colors to white.
			///		Use white point to control how highlights behave (blown out vs detailed)
			/// </summary>
			DirectX::XMFLOAT3 whitePoint = { 1.0,1.0,1.0 };

			/// <summary>
			///		Controls the overrall brightness of the image.
			///		Use exposure to fix if your scene is too dark/bright overall
			/// </summary>
			float exposure = 1.0f;

			float gamma = 2.2f;

			bool doIBL;
		};
	public:
		using Ref = std::shared_ptr<Scene>;
		Scene(RHI::RenderContext::Ref rctx, Renderer3D::ShadowMapManager::Ref shadowMapManager);
		~Scene();

		void OnUpdate();
		void OnResize(uint32_t width, uint32_t height);

		static Scene::Ref CreateScene(RHI::RenderContext::Ref rctx, Renderer3D::ShadowMapManager::Ref shadowMapManager);

		Entity& AddEntity(const std::string name);
		Entity& AddModel(std::filesystem::path path, ResourceLoadDesc& loadDesc);

		Entity& AddLight(const std::string name, LightType type);
		template<typename ...Components>
		std::vector<Entity> GetEntitiesWith()
		{
			std::vector<Entity> ents;
			auto view = registery.view<Components...>();

			for (auto entity : view) {
				ents.emplace_back(entity, this);
			}
			return ents;
		}

		void AssignGlobalMaterial(Entity entity, UUID materialUUID);
		void AssignGlobalDefaultMaterial(Entity entity);

		void AssignMaterial(Entity entity, UUID materialUUID, int subMeshIndex = 0);
		void AssignDefaultMaterial(Entity entity, int subMeshIndex = 0);

		template<typename ...Components>
		auto GetComponentView() {
			return registery.view<Components...>();
		}

		template<typename Component>
		Component* GetComponentStorage() {
			auto& sto = registery.storage<Component>();
			if (sto.raw() != nullptr) {
				Component* basePtr = *sto.raw();
				return basePtr;
			}
			std::cout << "Empty Component Storage." << std::endl;
			return nullptr;
		}

		/// <summary>
		/// Get the number of Components of type T in use.
		/// This is not the byte size.
		/// </summary>
		/// <typeparam name="Component"></typeparam>
		/// <returns></returns>
		template<typename Component>
		UINT GetComponentReach() {
			return static_cast<UINT>(registery.storage<Component>().size());;
		}

		Camera::Ref& GetCamera() { return camera; }
		Environment& GetEnvironment();

		std::shared_ptr<ResourceCache> GetResourceCache()const { return resourceCache; }

		RHI::UploadBuffer<SubMeshData>::Ref& GetSubMeshDataUploadBuffer();

		bool IsCameraDirty()const { return sceneFlags.isCameraDirty; }
		bool IsWindowResize()const { return sceneFlags.isWindowResize; }

		bool IsVertexIndexDataDirty()const { return resourceCache->IsVertexIndexDataDiry(); }
		
		void MakeVertexIndexDataClean() { resourceCache->MakeVertexIndexDataClean(); }
		void MakeCameraDirty() { sceneFlags.isCameraDirty = true; }

		std::vector<Entity>& GetEntities() { return entities; }

		Renderer3D::ShadowMapManager::Ref GetShadowMapManager()const { return shadowMapManager; }
	private:
		friend class Entity;
		entt::registry registery;

		std::vector<Entity> entities;
		std::vector<ISystem::Ptr> systems;

		Camera::Ref camera;
		Environment environment;

		std::shared_ptr<ResourceCache> resourceCache;
		RHI::UploadBuffer<SubMeshData>::Ref subMeshDataBuffer;

		std::unordered_map<UUID, std::vector<UUID>> subMeshMaterialMap;

		struct SceneFlags {
			bool isCameraDirty = true; //Has any camera parameter been changed?
			bool isWindowResize = false;
		}sceneFlags;

		Renderer3D::ShadowMapManager::Ref shadowMapManager;

	};
}
