#pragma once
#include "Delegate.h"
#include "Tracy/tracy/Tracy.hpp"

#include <stdint.h>
#include <unordered_map>
#include <array>
#include <memory>
#include <thread>

#include <Windows.h>

namespace Wiley
{
    enum class Key : uint32_t {
        A, B, C, D, E, F,
        G, H, I, J, K, L,
        M, N, O, P, Q, R,
        S, T, U, V, W, X,
        Y, Z,

        Alpha0, Alpha1, Alpha2, Alpha3,
        Alpha4, Alpha5, Alpha6, Alpha7,
        Alpha8, Alpha9,

        Num0, Num1, Num2, Num3, Num4,
        Num5, Num6, Num7, Num8, Num9,

        F1, F2, F3, F4, F5, F6,
        F7, F8, F9, F10, F11, F12,

        Space,
        Enter,
        Tab,

        Left,
        Up,
        Right,
        Down,

        Esc,
        Shift,
        Control,
        LAlt,
        RAlt,

        MouseLeft, MouseRight, MouseMiddle,

        Gamepad1,
        Gamepad2,
        Gamepad3,
        Gamepad4,

        GamepadLT,
        GamepadLB,
        GamepadRT,
        GamepadRB,


        None 
    };

    class MiddleMouseScrollEvent : public MultiCastDelegate<int> 
    {
        private: 
            friend class Input; 
            using MultiCastDelegate<int>::Broadcast; 
            using MultiCastDelegate<int>::RemoveAll; 
            using MultiCastDelegate<int>::Remove;
    };

    DECLARE_EVENT(RightMouseClickedEvent, Input, uint32_t, uint32_t);
    DECLARE_EVENT(WindowResizeEvent, Input, uint32_t, uint32_t);
    DECLARE_EVENT(F9ClickedEvent, Input, bool);

    struct InputEvent {
        MiddleMouseScrollEvent middleMouseEvent;
        RightMouseClickedEvent rightMouseEvent;
        WindowResizeEvent windowResizeEvent;

        F9ClickedEvent f9ClickedEvent;
    };
    
    class Window;
    struct WindowEventInfo;
    class Input
    {
        public:
            Input()
                :initialize(false), _window(nullptr), prevMousePosX(0.0f), prevMousePosY(0.0f), mouseWheelDelta(0.0f)
            {
                keys.fill(false);
                prevKeys.fill(false);

                POINT mouseScreenPos{};
                if (GetCursorPos(&mouseScreenPos)) {
                    mousePosX = static_cast<float>(mouseScreenPos.x);
                    mousePosY = static_cast<float>(mouseScreenPos.y);
                }
            }

            void Initialize(Window* window);

            InputEvent& GetInputEvent();
            void OnWindowEvent(const WindowEventInfo& info);

            void DeferResize(uint32_t width, uint32_t height);

            void Tick();
            bool IsPressed(int key)const noexcept;

            bool GetKey(Key key)const;
            bool IsKeyDown(Key key)const;
            bool IsKeyUp(Key key)const;
            bool IsKeyHeld(Key key)const;

            void SetMousePosition(float x, float y);
            void SetMouseVisiblity(bool isVisible);

            float GetMousePosX()const;
            float GetMousePosY()const;

            float GetMouseDeltaX()const;
            float GetMouseDeltaY()const;

            float GetMouseWheelData()const;

            std::mutex& GetMutex() { return mutex; }
            static Input& Get();

        private:
            Input(const Input&) = delete;
            Input& operator=(const Input&) = delete;
        private:
            InputEvent inputEvent;

            std::array<bool, static_cast<uint32_t>(Key::None)> keys;
            std::array<bool, static_cast<uint32_t>(Key::None)> prevKeys;

            float mousePosX;
            float mousePosY;

            float prevMousePosX;
            float prevMousePosY;

            float mouseWheelDelta;

            Window* _window;
            bool initialize;

            std::mutex mutex;
    };

#define gInput Input::Get()
}
