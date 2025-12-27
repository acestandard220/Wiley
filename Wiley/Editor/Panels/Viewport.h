#pragma once
#include "Panel.h"

namespace Wiley {

	class Viewport final : public Panel{
	public:
		Viewport(Editor* editor);

		virtual void Render()override;
	private:
		ImVec2 lastViewportSize;
	};

}