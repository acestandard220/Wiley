#include "Hierarchy.h"
#include "../Editor.h"
#include "../Scene/Entity.h"

namespace Wiley
{
	Hierarchy::Hierarchy(Editor* editor)
		:Panel(editor), selectedIndex(0)
	{
		name = "Hierarchy";
	}

	void Hierarchy::Render()
	{
		if (!enabled)
			return;
		int flags = ImGuiWindowFlags_NoCollapse;

		ImGui::Begin(name.c_str(), nullptr, flags);

		int i = 0;
		ImGui::Indent(10.0f);
		auto& entts = editor->GetCurrentScene()->GetEntities();
		for (auto& entt : entts) {
			std::string enttName = entt.GetTag().data() + std::string("##") + std::to_string(i);

			bool selected = (i == selectedIndex) ? true : false;
			if (ImGui::Selectable(enttName.c_str(), &selected) || selected)
			{
				editor->MakeEditorSelection(i, entt.GetUUID(), &entt);
				selectedIndex = i;
			}
			i++;
		}

		ImGui::End();

	}
}
