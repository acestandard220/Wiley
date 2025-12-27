#include "Engine/Engine.h"
#include "Tracy/tracy/Tracy.hpp"

#include <crtdbg.h>
#include "../Core/MemoryLeakDetector.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "dxil.dll")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "WindowsApp.lib")
#pragma comment(lib, "runtimeobject.lib")

///====================
/// Testing of default texture desciptors #
/// Texture Descritpor API # 
/// 
/// Testing Frame graph#
/// 
/// Root Constants #
/// Samplers #
/// Loading Textures #
/// 
/// =====FEATURES=====
/// Fog
/// Edge Detection
///====================
constexpr int width = 800;
constexpr int height = 800;

bool done = false;

using namespace Wiley;
int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

    //MemoryLeakDetector::SetMemoryAllocBreak(2371);

    tracy::SetThreadName("MainThread");

    ShowWindow(GetConsoleWindow(), SW_HIDE);

    WindowCreateParams windowParams = {
        .title = L"Wiley",
        .width = width,
        .height = height,
        .maximize = false
    };

    Window::Ref window = std::make_shared<Window>(windowParams);
    Engine engine(window);

    Scene::Ref scene = engine.GetScene();
    Camera::Ref camera = engine.GetScene()->GetCamera();

    window->GetWindowEvent().AddLambda([&](const WindowEventInfo& info) {
        engine.OnWindowEvent(info);
    });

    size_t frameCount = 0;
    while (window->Tick([]{}))
    {
        FrameMarkNamed("MainFrame");

        engine.OnUpdate();
        std::cout << frameCount++ << std::endl;
    }

    //MemoryLeakDetector::DumpMemoryLeaks();
}