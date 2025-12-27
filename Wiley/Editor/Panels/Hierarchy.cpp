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

		ImGui::Begin(name.c_str());

		int i = 0;
		auto& entts = editor->GetCurrentScene()->GetEntities();
		for (auto& entt : entts) {
			std::string enttName = entt.GetTag().data() + std::string("##") + std::to_string(i);

			bool selected = (i == selectedIndex) ? true : false;
			if (ImGui::Selectable(enttName.c_str(), &selected))
			{
				editor->MakeEditorSelection(i, entt.GetUUID(), &entt);
				selectedIndex = i;
			}
			i++;
		}

		ImGui::End();

	}
}
