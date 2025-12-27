#pragma once
#include "entt.hpp"
#include "Scene.h"
#include "Component.h"

#include "../Core/Utils.h"

namespace Wiley
{
	class Scene;
	class Entity
	{
		public:
			Entity() = default;
			Entity(entt::entity entt, Scene* scene)
				:entity(entt), parentScene(scene), active(true)
			{
							
			}

			~Entity() {

			}

			//Utilities
			WILEY_NODISCARD UUID GetUUID() {
				if (!HasComponent<TagComponent>())
				{
					std::cout << "Entity does not have a tag component. That alone is a huge problem." << std::endl;
					return UUIDFactory::InvalidUUID();
				}

				return GetComponent<TagComponent>().uuid;
			}

			WILEY_NODISCARD std::string_view GetTag() {
				if (!HasComponent<TagComponent>())
				{
					std::cout << "Entity does not have a tag component. That alone is a huge problem." << std::endl;
					return "Invalid Tag.";
				}

				return GetComponent<TagComponent>().tag;
			}

			WILEY_NODISCARD DirectX::XMFLOAT4X4& GetModelMatrix() {
				return GetComponent<TransformComponent>().modelMatrix;
			}



			WILEY_NODISCARD bool IsActive()const {
				return active;
			}

			template<typename Component>
			WILEY_NODISCARD bool HasComponent(){
				return parentScene->registery.any_of<Component>(entity);
			}

			template<typename Component,typename ...CtorArgs>
			Component& AddComponent(CtorArgs&& ...args) {
				return parentScene->registery.emplace<Component>(entity, std::forward<CtorArgs>(args)...);
			}

			template<typename Component>
			void RemoveComponent() {
				parentScene->registery.remove<Component>(entity);
			}

			template<typename Component>
			WILEY_NODISCARD Component& GetComponent(){
				//if (!HasComponent<Component>()) {
				//	std::cout << "Entity does not have specified component." << std::endl;
				//	//return {};
				//}
				if (entity == entt::null) {
					std::cout << "Cannot get null entity component data." << std::endl;
				}
				return parentScene->registery.get<Component>(entity);
			}


			bool operator==(Entity const& other) const{
				return other.entity == entity;
			}
	
			explicit operator entt::entity()const {
				return entity;
			}

		private:
			bool active;

			entt::entity entity = entt::null;
			Scene* parentScene;
	};

}
