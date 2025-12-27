#pragma once
#include "ISystem.h"

namespace Wiley {

	class Scene;
	class MeshFilterSystem :public ISystem {
	public:
		MeshFilterSystem(Scene* scene)
			:ISystem(scene)
		{

		}
		virtual void OnUpdate(float dt)override;
	private:
	};
	

}
