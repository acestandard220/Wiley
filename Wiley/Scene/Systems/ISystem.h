#pragma once
#include <memory>

namespace Wiley {

	class Scene;
	class ISystem {
	public:
		using Ptr = std::unique_ptr<ISystem>;
		ISystem(Scene* scene)
			:scene(scene)
		{

		}
		virtual void OnUpdate(float dt) = 0;
	protected:
		Scene* scene;
	};

}