#include "Inspector.h"
#include "../Editor.h"
#include "../Scene/Entity.h"

namespace Wiley
{
	Inspector::Inspector(Editor* editor)
		:Panel(editor)
	{
		name = "Inspector";
	}


	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_Framed;

		if (!entity.HasComponent<T>())
			return;

		auto& component = entity.GetComponent<T>();

		if (ImGui::TreeNodeEx(name.c_str(), flags))
		{

			uiFunction(component);

			ImGui::TreePop();
		}

	}


	void Inspector::Render()
	{
		if (!enabled)
			return;

		auto& selection = editor->GetEditorSelection();

		if (!selection.entt)
			return;

		auto entt = *selection.entt;

		ImGui::Begin(name.c_str(),nullptr);

		ImGui::Text("UUID :: %s", WILEY_UUID_STRING(selection.selectedUUID).c_str());

		const auto smm = editor->GetCurrentScene()->GetShadowMapManager();

		DrawComponent<TagComponent>("Entity", entt, [&](auto& component) {
			auto& tag = component;
			
			ImGui::Text("Name :: %s", tag.tag.c_str());
			ImGui::Text("UUID :: %s","67");

		});

		DrawComponent<TransformComponent>("Transform",entt,[&](auto& component){
			auto& transform = component;

			if (ImGui::DragFloat3("Position", &transform.position.x, 0.5f, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_ColorMarkers)){
				smm->MakeAllLightEntityDirty();
			}
			if (ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.5f, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_ColorMarkers)) {
				smm->MakeAllLightEntityDirty();
			}
			if (ImGui::DragFloat3("Scale", &transform.scale.x, 0.5f, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_ColorMarkers)) {
				smm->MakeAllLightEntityDirty();
			}
		});

		DrawComponent<MeshFilterComponent>("Mesh Filter", entt, [&](auto& component) {
			auto& meshFilter = component;



		});

		DrawComponent<LightComponent>("Light", entt, [&](auto& component) {
			auto& light = component;

			if (
				ImGui::Combo("Type", (int*)&light.type, "Directional\0Point\0Spot") |
				ImGui::DragFloat3(((bool)light.type) ? "Position" : "Direction", &light.position.x, 0.5f, 0.0f, 0.0f, "%.3f") |
				ImGui::DragFloat3("Color", &light.color.x, 0.5f, 0.0f, 0.0f, "%.3f") |
				ImGui::DragFloat("Intensity", &light.intensity, 0.5, 0.0f, 0.0f))
			{

				if (light.type == LightType::Point)
					smm->MakePointLightDirty(static_cast<entt::entity>(entt));
				else
					smm->MakeLightEntityDirty(static_cast<entt::entity>(entt));
			}
			

			if (light.type == LightType::Spot) {
				ImGui::Separator(); 
				const static float _bias = 0.05f;
				
				const float orMin = light.innerRadius + _bias;
				const float orMax = 0.0f;
				const float irMax = orMin - _bias;
				const float irMin = 1.0f;

				ImGui::DragFloat("Inner Radius", &light.innerRadius, 0.5f, irMin, irMax);
				ImGui::DragFloat("Outer Radius", &light.outerRadius, 0.5f, orMin, 0.0);

				if (ImGui::DragFloat3("Spot Direction", &light.spotDirection.x)) {
					if (light.type == LightType::Point)
						smm->MakePointLightDirty(static_cast<entt::entity>(entt));
					else
						smm->MakeLightEntityDirty(static_cast<entt::entity>(entt));
				}
			}

			ImGui::Separator(); 
			
		});


		{
			ImGui::Button("Add Component");
		}

		ImGui::End();

	}

}
