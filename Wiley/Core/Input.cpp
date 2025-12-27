#include "Input.h"
#include "Window.h"

namespace Wiley
{
    Input input;

    uint32_t deferredWidth;
    uint32_t deferredHeight;
    bool deferred = false;


    float Input::GetMouseDeltaY() const
    {
        return mousePosY - prevMousePosY;
    }

    float Input::GetMousePosX() const
    {
        return mousePosX;
    }

    float Input::GetMousePosY() const
    {
        return mousePosY;
    }

    float Input::GetMouseDeltaX() const
    {
        return mousePosX - prevMousePosX;
    }

    float Input::GetMouseWheelData() const
    {
        return mouseWheelDelta;
    }

    Input& Input::Get()
    {
        return input;
    }

    void Input::Initialize(Window* window)
    {
        if(!initialize)
        {
            _window = window;
            initialize = true;
        }
        else {
            std::cout << "This singleton class has already been initialized." << std::endl;
            return;
        }
    }

    InputEvent& Input::GetInputEvent()
    {
        return inputEvent;
    }

    void Input::OnWindowEvent(const WindowEventInfo& info)
    {
        switch (info.msg) {
            case WM_MOUSEWHEEL:
            {
                inputEvent.middleMouseEvent.Broadcast((uint32_t)GET_WHEEL_DELTA_WPARAM(info.wparam) / WHEEL_DELTA);
                break;
            }
            case WM_RBUTTONDOWN: {
                uint32_t mx = GET_X_LPARAM(info.lparam);
                uint32_t my = GET_Y_LPARAM(info.lparam);
                inputEvent.rightMouseEvent.Broadcast(mx, my);
                break;
            }
            case WM_SIZE:
            {
                inputEvent.windowResizeEvent.Broadcast((uint32_t)info.width, (uint32_t)info.height);
                break;
            }
        }
        mouseWheelDelta = (float)GET_WHEEL_DELTA_WPARAM(info.wparam) / (float)WHEEL_DELTA;
    }

    void Input::DeferResize(uint32_t width, uint32_t height)
    {
        deferred = true;
        deferredWidth = width;
        deferredHeight = height;
    }

    bool Input::IsPressed(int key)const noexcept
    {
        return (::GetKeyState(key) & 0x8000) != 0;
    }

    bool Input::GetKey(Key key) const 
    { 
        return keys[static_cast<uint32_t>(key)];
    }

    bool Input::IsKeyDown(Key key) const
    { 
        return (GetKey(key) && !prevKeys[static_cast<uint32_t>(key)]);
    }

    bool Input::IsKeyUp(Key key) const 
    { 
        return (!GetKey(key) && prevKeys[static_cast<uint32_t>(key)]);
    }

    bool Input::IsKeyHeld(Key key) const
    {
        return (GetKey(key) && prevKeys[static_cast<uint32_t>(key)]);
    }

    void Input::SetMousePosition(float x, float y)
    {
        HWND handle = static_cast<HWND>(_window->GetHWND());
        if (handle == ::GetActiveWindow())
        {
            POINT mouse_screen_pos = POINT{ static_cast<LONG>(x), static_cast<LONG>(x) };
            if (::ClientToScreen(handle, &mouse_screen_pos))
            {
                ::SetCursorPos(mouse_screen_pos.x, mouse_screen_pos.y);
            }
        }
    }

    void Input::SetMouseVisiblity(bool isVisible)
    {
        ::ShowCursor(isVisible);
    }

    void Input::Tick()
    {
        ZoneScopedN("RenderContext::RenderContext");

        if (deferred) {
            inputEvent.windowResizeEvent.Broadcast(deferredWidth, deferredHeight);
            deferred = false;
        }

        prevKeys = std::move(keys);

        prevMousePosX = mousePosX;
        prevMousePosY = mousePosY;

        if (_window->IsActive())
        {
            POINT mouseScreenPos{};
            if (GetCursorPos(&mouseScreenPos)) {
                mousePosX = static_cast<float>(mouseScreenPos.x);
                mousePosY = static_cast<float>(mouseScreenPos.y);
            }

            using enum Key;

            keys[(uint32_t)MouseLeft] = (GetKeyState(VK_LBUTTON) & 0x8000) != 0;
            keys[(uint32_t)MouseMiddle] = (GetKeyState(VK_MBUTTON) & 0x8000) != 0;
            keys[(uint32_t)MouseRight] = (GetKeyState(VK_RBUTTON) & 0x8000) != 0;

            keys[static_cast<uint32_t>(A)] = IsPressed('A');
            keys[static_cast<uint32_t>(B)] = IsPressed('B');
            keys[static_cast<uint32_t>(C)] = IsPressed('C');
            keys[static_cast<uint32_t>(D)] = IsPressed('D');
            keys[static_cast<uint32_t>(E)] = IsPressed('E');
            keys[static_cast<uint32_t>(F)] = IsPressed('F');
            keys[static_cast<uint32_t>(G)] = IsPressed('G');
            keys[static_cast<uint32_t>(H)] = IsPressed('H');
            keys[static_cast<uint32_t>(I)] = IsPressed('I');
            keys[static_cast<uint32_t>(J)] = IsPressed('J');
            keys[static_cast<uint32_t>(K)] = IsPressed('K');
            keys[static_cast<uint32_t>(L)] = IsPressed('L');
            keys[static_cast<uint32_t>(M)] = IsPressed('M');
            keys[static_cast<uint32_t>(N)] = IsPressed('N');
            keys[static_cast<uint32_t>(O)] = IsPressed('O');
            keys[static_cast<uint32_t>(P)] = IsPressed('P');
            keys[static_cast<uint32_t>(Q)] = IsPressed('Q');
            keys[static_cast<uint32_t>(R)] = IsPressed('R');
            keys[static_cast<uint32_t>(S)] = IsPressed('S');
            keys[static_cast<uint32_t>(T)] = IsPressed('T');
            keys[static_cast<uint32_t>(U)] = IsPressed('U');
            keys[static_cast<uint32_t>(V)] = IsPressed('V');
            keys[static_cast<uint32_t>(W)] = IsPressed('W');
            keys[static_cast<uint32_t>(X)] = IsPressed('X');
            keys[static_cast<uint32_t>(Y)] = IsPressed('Y');
            keys[static_cast<uint32_t>(Z)] = IsPressed('Z');

            keys[static_cast<uint32_t>(Alpha0)] = IsPressed('0');
            keys[static_cast<uint32_t>(Alpha1)] = IsPressed('1');
            keys[static_cast<uint32_t>(Alpha2)] = IsPressed('2');
            keys[static_cast<uint32_t>(Alpha3)] = IsPressed('3');
            keys[static_cast<uint32_t>(Alpha4)] = IsPressed('4');
            keys[static_cast<uint32_t>(Alpha5)] = IsPressed('5');
            keys[static_cast<uint32_t>(Alpha6)] = IsPressed('6');
            keys[static_cast<uint32_t>(Alpha7)] = IsPressed('7');
            keys[static_cast<uint32_t>(Alpha8)] = IsPressed('8');
            keys[static_cast<uint32_t>(Alpha9)] = IsPressed('9');

            keys[static_cast<uint32_t>(F1)] = IsPressed(VK_F1);
            keys[static_cast<uint32_t>(F2)] = IsPressed(VK_F2);
            keys[static_cast<uint32_t>(F3)] = IsPressed(VK_F3);
            keys[static_cast<uint32_t>(F4)] = IsPressed(VK_F4);
            keys[static_cast<uint32_t>(F5)] = IsPressed(VK_F5);
            keys[static_cast<uint32_t>(F6)] = IsPressed(VK_F6);
            keys[static_cast<uint32_t>(F7)] = IsPressed(VK_F7);
            keys[static_cast<uint32_t>(F8)] = IsPressed(VK_F8);
            keys[static_cast<uint32_t>(F9)] = IsPressed(VK_F9);
            keys[static_cast<uint32_t>(F10)] = IsPressed(VK_F10);
            keys[static_cast<uint32_t>(F11)] = IsPressed(VK_F11);
            keys[static_cast<uint32_t>(F12)] = IsPressed(VK_F12);

            keys[static_cast<uint32_t>(Num0)] = IsPressed(VK_NUMPAD0);
            keys[static_cast<uint32_t>(Num1)] = IsPressed(VK_NUMPAD1);
            keys[static_cast<uint32_t>(Num2)] = IsPressed(VK_NUMPAD2);
            keys[static_cast<uint32_t>(Num3)] = IsPressed(VK_NUMPAD3);
            keys[static_cast<uint32_t>(Num4)] = IsPressed(VK_NUMPAD4);
            keys[static_cast<uint32_t>(Num5)] = IsPressed(VK_NUMPAD5);
            keys[static_cast<uint32_t>(Num6)] = IsPressed(VK_NUMPAD6);
            keys[static_cast<uint32_t>(Num7)] = IsPressed(VK_NUMPAD7);
            keys[static_cast<uint32_t>(Num8)] = IsPressed(VK_NUMPAD8);
            keys[static_cast<uint32_t>(Num9)] = IsPressed(VK_NUMPAD9);

            keys[static_cast<uint32_t>(Space)] = IsPressed(VK_SPACE);
            keys[static_cast<uint32_t>(Enter)] = IsPressed(VK_RETURN);
            keys[static_cast<uint32_t>(Tab)] = IsPressed(VK_TAB);

            keys[static_cast<uint32_t>(Left)] = IsPressed(VK_LEFT);
            keys[static_cast<uint32_t>(Right)] = IsPressed(VK_RIGHT);
            keys[static_cast<uint32_t>(Up)] = IsPressed(VK_UP);
            keys[static_cast<uint32_t>(Down)] = IsPressed(VK_DOWN);

            keys[static_cast<uint32_t>(Shift)] = IsPressed(VK_SHIFT);
            keys[static_cast<uint32_t>(Control)] = IsPressed(VK_CONTROL);
            keys[static_cast<uint32_t>(LAlt)] = IsPressed(VK_LMENU);
            keys[static_cast<uint32_t>(RAlt)] = IsPressed(VK_RMENU);
            keys[static_cast<uint32_t>(Esc)] = IsPressed(VK_ESCAPE);

            keys[static_cast<uint32_t>(Gamepad1)] = IsPressed(VK_GAMEPAD_Y);
            keys[static_cast<uint32_t>(Gamepad2)] = IsPressed(VK_GAMEPAD_B);
            keys[static_cast<uint32_t>(Gamepad3)] = IsPressed(VK_GAMEPAD_X);
            keys[static_cast<uint32_t>(Gamepad4)] = IsPressed(VK_GAMEPAD_A);

            keys[static_cast<uint32_t>(GamepadLB)] = IsPressed(VK_GAMEPAD_LEFT_SHOULDER);
            keys[static_cast<uint32_t>(GamepadLT)] = IsPressed(VK_GAMEPAD_LEFT_TRIGGER);

            keys[static_cast<uint32_t>(GamepadRT)] = IsPressed(VK_GAMEPAD_RIGHT_SHOULDER);
            keys[static_cast<uint32_t>(GamepadRB)] = IsPressed(VK_GAMEPAD_RIGHT_TRIGGER);

        }
    }

}
