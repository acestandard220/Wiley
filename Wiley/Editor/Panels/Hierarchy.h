#pragma once
#include "Panel.h"
#include "../Core/UUID.h"


namespace Wiley
{
	class Hierarchy final : public Panel
	{
	public:
		Hierarchy(Editor* editor);

		virtual void Render()override;

	private:
		int selectedIndex;
	};

}
