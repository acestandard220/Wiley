#include "Editor.h"

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_dx12.h"
#include "../ImGui/imgui_impl_win32.h"

#include "ImGuiExtension/Themes.h"

namespace Wiley {

    static void AllocDescriptor(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu) {
        RHI::DescriptorHeap* heap = static_cast<RHI::DescriptorHeap*>(info->UserData);
        RHI::DescriptorHeap::Descriptor desc = heap->Allocate();

        *out_cpu = desc.cpuHandle;  
        *out_gpu = desc.gpuHandle;  
    }

    static void FreeDescriptor(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu) {
        RHI::DescriptorHeap* heap = static_cast<RHI::DescriptorHeap*>(info->UserData);
        // Calculate index from handle if needed, or store it differently
        // heap->Deallocate(index);
    }

    Editor::Editor(Window::Ref window, RHI::RenderContext::Ref rctx, Scene::Ref scene, Renderer3D::Renderer::Ref renderer)
        : window(window), rctx(rctx), scene(scene), renderer(renderer)
    {
        {
            // Setup ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

            ImGuiStyle& style = ImGui::GetStyle();
            
            // Init Win32
            ImGui_ImplWin32_Init(window->GetHWND());

            // Init DX12
            ImGui_ImplDX12_InitInfo init_info = {};
            init_info.Device = rctx->GetDevice()->GetNative();
            init_info.CommandQueue = rctx->GetCommandQueue()->GetQueue();
            init_info.NumFramesInFlight = FRAMES_IN_FLIGHT;
            init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
            init_info.SrvDescriptorHeap = rctx->GetDescriptorHeaps().cbv_srv_uav->GetHeap();
            init_info.SrvDescriptorAllocFn = AllocDescriptor;
            init_info.SrvDescriptorFreeFn = FreeDescriptor;
            init_info.UserData = rctx->GetDescriptorHeaps().cbv_srv_uav.get();

            ImGui_ImplDX12_Init(&init_info);

            io.FontGlobalScale = 1.0f;

            SetBlenderTheme();

            //SetD5RendererStyle();

            ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImGui::GetStyle().Colors[ImGuiCol_TitleBg];

        }


        {
            panels.emplace_back(std::make_unique<Viewport>(this));

            panels.emplace_back(std::make_unique<Inspector>(this));
            panels.emplace_back(std::make_unique<Hierarchy>(this));
            panels.emplace_back(std::make_unique<SceneProperties>(this));
        }
    }

    // Add cleanup in destructor:
    Editor::~Editor()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void Editor::BeginDockspace()
    {
        ImGuiIO& io = ImGui::GetIO();

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_MenuBar |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("MainDockSpace", nullptr, flags);
        ImGui::PopStyleVar(2);

        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
    }

    void Editor::EndDockspace()
    {
        ImGui::End();
    }


    float color[3];
    void Editor::Run()
    {
        BeginDockspace();
        auto& io = ImGui::GetIO();

        for (auto& panel : panels) {
            panel->Render();
        }

        ImGui::Begin("WindowIMGuiDemo");
        ImGui::ColorEdit3("ColorV", color);
        ImGui::Text("FPS %.3f", io.Framerate);  
        ImGui::End();

        EndDockspace();
    }

    const Editor::EditorSelction& Editor::GetEditorSelection() const
    {
        return selection;
    }

    void Editor::MakeEditorSelection(int index, UUID id, Entity* entt)
    {
        selection.selectedIndex = index;
        selection.selectedUUID = id;
        selection.entt = entt;
    }

    Scene::Ref Editor::GetCurrentScene() const
    {
        return scene;
    }

    Renderer3D::Renderer::Ref Editor::GetRenderer() const
    {
        return renderer;
    }

    Window::Ref Editor::GetWindow() const
    {
        return window;
    }

}