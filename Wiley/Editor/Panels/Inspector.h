#pragma once
#include "Panel.h"
#include "../Core/UUID.h"


namespace Wiley
{
	class Inspector final : public Panel
	{
	public:
		Inspector(Editor* editor);

		virtual void Render()override;

	private:

		//Selection Data.
		int selectedIndex;
		UUID selectedUUID;
	};

}