#pragma once
#include "Input.h"

#include <iostream>
#include <string>
#include <cstdint>
#include <functional>
#include <array>

#include <Windows.h>
#include <windowsx.h>


namespace Wiley
{
	struct WindowEventInfo {
		HWND hwnd;
		UINT msg;
		WPARAM wparam;
		LPARAM lparam;
		float width;
		float height;
	};

	struct WindowCreateParams {
		std::wstring title;

		UINT width;
		UINT height;

		bool maximize = false;
	};

	class Window;
	DECLARE_EVENT(WindowEvent, Window, const WindowEventInfo&);

	class Window
	{
	public:
		using Ref = std::shared_ptr<Window>;
		Window(WindowCreateParams const& windowParams);
		~Window();

		bool Tick(std::function<void()> callback);

		void OnResize(std::function<void(uint32_t width, uint32_t height)> callback) { resizeCallback = callback; }

		bool IsActive()const noexcept;
		bool IsDone()const noexcept;

		void GetDimensions(uint32_t& width, uint32_t& height) const;
		void GetClientDimensions(uint32_t& width, uint32_t& height);

		HWND GetHWND() { return hwnd; }

		WindowEvent& GetWindowEvent();

		void BroadCast(const WindowEventInfo& eventInfo);
		static void SetDimension(UINT width, UINT height);

		inline static Window* sWindow;
	private:
		HWND hwnd;

		UINT width;
		UINT height;
		bool isDone;

		WindowEvent windowEvent;

		std::function<void(uint32_t, uint32_t)> resizeCallback;
	};
}

