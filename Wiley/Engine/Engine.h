#pragma once
#include "../Core/ThreadPool.h"
#include "../Core/Window.h"

#include "../RHI/RenderContext.h"

#include "../Renderer/Renderer.h"

#include "../Scene/Scene.h"
#include "../Scene/Entity.h"
#include "../Scene/Component.h"
#include "../Editor/Editor.h"

#include "Tracy/tracy/Tracy.hpp"


namespace Wiley {

	class Engine {
		public:
			Engine(Window::Ref window);
			~Engine();

			void OnUpdate();
				
			void OnWindowEvent(const WindowEventInfo& info);

			WILEY_NODISCARD Scene::Ref GetScene();
		private:
			Window::Ref window;
			Renderer3D::Renderer::Ref renderer;
			Scene::Ref scene;
			RHI::RenderContext::Ref rctx;

			Editor::Ref editor;
			bool isEditorVisible;
			
	};

}