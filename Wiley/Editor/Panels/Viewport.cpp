#include "Viewport.h"
#include "../Editor.h"

namespace Wiley {



	Viewport::Viewport(Editor* editor)
		:Panel(editor), lastViewportSize(0, 0)
	{
		name = "Viewport";
	}

	void Viewport::Render()
	{
		if (!enabled)
			return;

		ImGui::Begin(name.c_str());

		const auto texture = editor->GetRenderer()->GetOutputTexture();
		const auto srv = texture->GetSRV();

		ImVec2 avail = ImGui::GetContentRegionAvail();

		uint32_t pixelWidth = (uint32_t)(avail.x);
		uint32_t pixelHeight = (uint32_t)(avail.y);

		if (avail.x != lastViewportSize.x || avail.y != lastViewportSize.y)
		{
			if (avail.x < 0 || avail.y < 0) {
				ImGui::End();
				return;
			}

			lastViewportSize = avail;
			gInput.DeferResize(pixelWidth, pixelHeight);
		}

		ImGui::Image((ImTextureID)srv.gpuHandle.ptr, lastViewportSize);

		ImGui::End();
	}

}