#pragma once
#include "Panel.h"
#include "../Core/UUID.h"


namespace Wiley
{
	class SceneProperties final : public Panel
	{
	public:
		SceneProperties(Editor* editor);

		virtual void Render()override;

	private:
		float nearFarParamSpeed;
		float fovParamSpeed;

		const char* decimalFormat;
	};

}