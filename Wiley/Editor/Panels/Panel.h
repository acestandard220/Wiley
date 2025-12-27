#pragma once
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_dx12.h"
#include "../ImGui/imgui_impl_win32.h"

#include <string>
#include <memory>

namespace Wiley
{
	class Editor;

	class Panel
	{
	public:
		using Ptr = std::unique_ptr<Panel>;
		Panel(Editor* editor);

		virtual void Render() = 0;

	protected:
		std::string name;
		bool enabled;

		Editor* editor;
	};

}
