#pragma once
#include "../Core/Window.h"
#include "../RHI/RenderContext.h"
#include "../Scene/Scene.h"
#include "../Renderer/Renderer.h"

#include "Panels/Viewport.h"
#include "Panels/Hierarchy.h"
#include "Panels/Inspector.h"
#include "Panels/Viewport.h"
#include "Panels/SceneProperties.h"


namespace Wiley {

	class Editor {
		
		enum class EditorTheme {
			EditorTheme_Default,
			EditorTheme_Blender,
			EditorTheme_Adobe
		};

		struct EditorSelction {
			int selectedIndex = 0;
			UUID selectedUUID = WILEY_INVALID_UUID;
			Entity* entt = nullptr;
		};

	public:
		using Ref = std::shared_ptr<Editor>;
		Editor(Window::Ref window,RHI::RenderContext::Ref rctx,Scene::Ref scene, Renderer3D::Renderer::Ref renderer);
		~Editor();

		void BeginDockspace();
		void EndDockspace();

		void Run();

		void MakeEditorSelection(int index, UUID id, Entity* entt);

		const EditorSelction& GetEditorSelection()const;

		Scene::Ref GetCurrentScene()const;
		Renderer3D::Renderer::Ref GetRenderer()const;
		Window::Ref GetWindow()const;
	private:
		EditorSelction selection;
		std::vector<Panel::Ptr> panels;
		
		std::unique_ptr<Hierarchy> hierarchy;
		std::unique_ptr<Inspector> inspector;

		//Contexts...
		Window::Ref window;
		RHI::RenderContext::Ref rctx;
		Scene::Ref scene;
		Renderer3D::Renderer::Ref renderer;

	};

}