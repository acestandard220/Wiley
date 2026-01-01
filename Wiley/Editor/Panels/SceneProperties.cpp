#include "SceneProperties.h"
#include "../Editor.h"

namespace Wiley
{

	SceneProperties::SceneProperties(Editor* editor)
		:Panel(editor)
	{
		name = "SceneProperties";

		nearFarParamSpeed = 0.025;
		fovParamSpeed = 5.0f;
		decimalFormat = "%.3f";
	}

	void SceneProperties::Render()
	{
		if (!enabled)
			return;

		auto scene = editor->GetCurrentScene();

		ImGui::Begin(name.c_str());

		auto& environment = scene->GetEnvironment();
		{
			ImGui::ColorEdit3("Background Color", &environment.backgroundColor.x);
			ImGui::Checkbox("IBL", &environment.doIBL);

			ImGui::DragFloat("Gamma", &environment.gamma);

			ImGui::DragFloat("Exposure", &environment.exposure);
			ImGui::ColorEdit3("White Point", &environment.whitePoint.x);
		}

		auto& camera = scene->GetCamera();
		{
			auto oldNear = camera->GetNear();
			if (ImGui::DragFloat("Near Plane", &oldNear, nearFarParamSpeed, 0.1f, 0.8f,decimalFormat, ImGuiSliderFlags_ClampOnInput))
			{
				camera->SetNear(oldNear);
				scene->MakeCameraDirty();
			}
			
			auto oldFar = camera->GetFar();
			if (ImGui::DragFloat("Far Plane", &oldFar, nearFarParamSpeed, 1.0f, 10000.0f, decimalFormat, ImGuiSliderFlags_ClampOnInput)) {
				camera->SetFar(oldFar);
				scene->MakeCameraDirty();
			}

			auto oldAspectRatio = camera->GetAspectRatio();
			if (ImGui::DragFloat("Field Of View", &oldAspectRatio, fovParamSpeed, 40.0f, 90.0f, decimalFormat, ImGuiSliderFlags_ClampOnInput)) {
				camera->SetAspectRatio(oldAspectRatio);
				scene->MakeCameraDirty();
			}
		}

		ImGui::End();
	}
}
