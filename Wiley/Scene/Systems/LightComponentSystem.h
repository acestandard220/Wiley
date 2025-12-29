#pragma once
#include "ISystem.h"

namespace Wiley {

	class Scene;
	class LightComponentSystem :public ISystem {
	public:
		LightComponentSystem(Scene* scene)
			:ISystem(scene)
		{

		}
		virtual void OnUpdate(float dt)override;
	private:
		void ComputeDirectionalLightViewProjections(void* lightComponent);
		void ComputePointLightViewProjections(void* lightComponent);
		void ComputeSpotLightViewProjection(void* lightComponent);
	};


}
