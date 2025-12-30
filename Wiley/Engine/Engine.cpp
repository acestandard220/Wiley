#include "Engine.h"

namespace Wiley {

	Engine::Engine(Window::Ref window)
		:window(window), isEditorVisible(true)
	{
		ZoneScopedN("Engine::Engine");

		gThreadPool.Initialize();
		rctx = std::make_shared<RHI::RenderContext>(window);
		renderer = std::make_shared<Renderer3D::Renderer>(window, rctx);

		shadowMapManager = std::make_shared<Renderer3D::ShadowMapManager>(rctx);

		//Default Scene Creation
		scene = Scene::CreateScene(rctx, shadowMapManager);

		editor = std::make_shared<Editor>(window, rctx, scene,renderer);

        ResourceLoadDesc loadDesc{};
        loadDesc.extension = FileExtension::OBJ;
		loadDesc.flipUV = false;

		const auto model = scene->AddModel("P:/Projects/VS/Wiley/Wiley/Assets/Models/SponzaOld/sponza.obj", loadDesc);
		const auto cylinder = scene->AddModel("P:/Projects/VS/Wiley/Wiley/Assets/Models/Cylinder.obj",loadDesc);
		scene->AssignGlobalDefaultMaterial(cylinder);


		gInput.GetInputEvent().windowResizeEvent.AddMember(&Renderer3D::Renderer::OnResize, *renderer.get());
		gInput.GetInputEvent().windowResizeEvent.AddMember(&Scene::OnResize, *scene.get());

		gInput.GetInputEvent().middleMouseEvent.AddMember(&Camera::Zoom, *scene->GetCamera().get());
	}

	Engine::~Engine()
	{
        gThreadPool.~ThreadPool();
        scene.reset();
        renderer.reset();
        rctx.reset();
	}

	void Engine::OnUpdate()
	{
		ZoneScopedN("Engine::OnUpdate");

        {
            std::lock_guard<std::mutex> lock(gInput.GetMutex());
            gInput.Tick();
        }	
        
		scene->OnUpdate();

		renderer->NewFrame(scene);
		renderer->RenderFrame();

		//Crashes RenderDoc and PIX on launch
		//editor->Run();

		renderer->EndFrame();
		renderer->RenderToWindowDirect();

	}


	void Engine::OnWindowEvent(const WindowEventInfo& info)
	{
		gInput.OnWindowEvent(info);
	}

	Scene::Ref Engine::GetScene()
	{
		return scene;
	}
}