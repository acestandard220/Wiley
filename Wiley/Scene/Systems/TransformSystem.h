#pragma once
#include "ISystem.h"

#include <memory>

namespace Wiley {

	class Scene;
	class TransformSystem : public ISystem {
	public:
		TransformSystem(Scene* scene)
			:ISystem(scene)
		{

		}
		virtual void OnUpdate(float dt)override;
	private:
	};

}