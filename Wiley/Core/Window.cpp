#include "Window.h"

#include "imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx12.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);         

namespace Wiley
{

	LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

	Window::Window(WindowCreateParams const& windowParams)
		:width(windowParams.width), height(windowParams.height)
	{
		sWindow = this;

		WNDCLASS windowClass = {};
		windowClass.lpszClassName = L"WindowClassWiley";
		windowClass.lpfnWndProc = WindowProc;
		windowClass.hInstance = GetModuleHandle(nullptr);

		RegisterClass(&windowClass);
		
		hwnd = CreateWindowEx(0, L"WindowClassWiley", windowParams.title.c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
			width ,height, 0, 0, GetModuleHandleA(0), 0);

		if (!hwnd)
		{
			MessageBox(nullptr, L"Window creation failed!", L"Fatal Error!", MB_ICONEXCLAMATION | MB_OK);
			return;
		}

		ShowWindow(hwnd, (windowParams.maximize) ? SW_MAXIMIZE : SW_SHOW);

		SetForegroundWindow(hwnd);
		SetFocus(hwnd);

		gInput.Initialize(this);

		// Force WM_SIZE to update cached dimensions with actual client area
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);
		PostMessage(hwnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top));
	}

	Window::~Window()
	{

	}

	bool Window::Tick(std::function<void()> callback)
	{
		MSG msg{};		
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		

		if (msg.message == WM_QUIT)
		{
			return false;
		}
		
		return true;
	}

	bool Window::IsActive() const noexcept
	{
		return GetForegroundWindow() == hwnd;
	}

	void Window::GetDimensions(uint32_t& width, uint32_t& height) const
	{
		width = this->width;
		height = this->height;
	}

	void Window::GetClientDimensions(uint32_t& width, uint32_t& height)
	{
		RECT windowRect{};
		if (GetClientRect(hwnd, &windowRect))
		{
			width = windowRect.right - windowRect.left;
			height = windowRect.bottom - windowRect.top;
		}
	}

	WindowEvent& Window::GetWindowEvent()
	{
		return windowEvent;
	}

	void Window::BroadCast(const WindowEventInfo& eventInfo)
	{
		windowEvent.Broadcast(eventInfo);
	}

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
			return true;

		WindowEventInfo eventInfo{};
		eventInfo.hwnd = hwnd;
		eventInfo.msg = msg;
		eventInfo.wparam = wparam;
		eventInfo.lparam = lparam;

		if (msg == WM_CLOSE || msg == WM_DESTROY)
		{
			PostQuitMessage(0);
			return 0;
		}
		else if (msg == WM_DISPLAYCHANGE || msg == WM_SIZE)
		{
			int width = LOWORD(lparam);
			int height = HIWORD(lparam);
			
			eventInfo.width =  static_cast<float>(width);
			eventInfo.height = static_cast<float>(height);
		}
		Window::sWindow->BroadCast(eventInfo);

		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}