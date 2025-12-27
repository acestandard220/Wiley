#include "Themes.h"


void SetBlenderTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Modern dark palette - inspired by VS Code, CLion, and Blender
    const ImVec4 almostBlack = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);         // #171719 - Deepest background
    const ImVec4 darkBg = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);              // #1E1E21 - Main background
    const ImVec4 mediumBg = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);            // #26262B - Elevated panels
    const ImVec4 lightBg = ImVec4(0.19f, 0.19f, 0.21f, 1.00f);             // #303035 - Headers/hover
    const ImVec4 inputBg = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);             // #1A1A1C - Input fields

    // Modern accent colors
    const ImVec4 accentBlue = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);          // #4296FA - Primary accent
    const ImVec4 accentBlueBright = ImVec4(0.36f, 0.69f, 1.00f, 1.00f);    // #5CB0FF - Hover
    const ImVec4 accentBlueActive = ImVec4(0.16f, 0.49f, 0.88f, 1.00f);    // #297DE0 - Active/pressed

    const ImVec4 accentOrange = ImVec4(0.98f, 0.65f, 0.26f, 1.00f);        // #FAA643 - Warning/secondary
    const ImVec4 accentCyan = ImVec4(0.26f, 0.82f, 0.96f, 1.00f);          // #43D1F5 - Info

    // Text colors - modern hierarchy
    const ImVec4 textBright = ImVec4(0.95f, 0.95f, 0.96f, 1.00f);          // #F2F2F5 - Primary text
    const ImVec4 textNormal = ImVec4(0.82f, 0.82f, 0.84f, 1.00f);          // #D1D1D6 - Secondary text
    const ImVec4 textDim = ImVec4(0.58f, 0.58f, 0.60f, 1.00f);             // #949499 - Tertiary text
    const ImVec4 textDisabled = ImVec4(0.40f, 0.40f, 0.42f, 1.00f);        // #66666B - Disabled

    // Borders and separators
    const ImVec4 borderStrong = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);        // #404045 - Strong borders
    const ImVec4 borderSubtle = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);        // #2E2E33 - Subtle dividers
    const ImVec4 borderNone = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);          // Transparent

    // Window backgrounds
    colors[ImGuiCol_WindowBg] = darkBg;
    colors[ImGuiCol_ChildBg] = mediumBg;
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.16f, 0.98f);
    colors[ImGuiCol_Border] = borderSubtle;
    colors[ImGuiCol_BorderShadow] = borderNone;

    // Title bars
    colors[ImGuiCol_TitleBg] = almostBlack;
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = almostBlack;
    colors[ImGuiCol_MenuBarBg] = almostBlack;

    // Scrollbars - thin and modern
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.09f, 0.80f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.35f, 0.35f, 0.37f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.45f, 0.47f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = accentBlue;

    // Interactive elements
    colors[ImGuiCol_CheckMark] = accentBlue;
    colors[ImGuiCol_SliderGrab] = accentBlue;
    colors[ImGuiCol_SliderGrabActive] = accentBlueActive;

    // Buttons - modern flat design
    colors[ImGuiCol_Button] = mediumBg;
    colors[ImGuiCol_ButtonHovered] = lightBg;
    colors[ImGuiCol_ButtonActive] = accentBlueActive;

    // Headers - subtle modern style
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);    // Translucent blue

    // Separators
    colors[ImGuiCol_Separator] = borderSubtle;
    colors[ImGuiCol_SeparatorHovered] = accentBlue;
    colors[ImGuiCol_SeparatorActive] = accentBlueBright;

    // Resize grips
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ResizeGripActive] = accentBlue;

    // Tabs - modern clean tabs
    colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.11f, 0.12f, 1.00f);
    colors[ImGuiCol_TabHovered] = lightBg;
    colors[ImGuiCol_TabActive] = darkBg;
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);

    // Docking
    colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = almostBlack;

    // Plots
    colors[ImGuiCol_PlotLines] = accentBlue;
    colors[ImGuiCol_PlotLinesHovered] = accentBlueBright;
    colors[ImGuiCol_PlotHistogram] = accentCyan;
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.36f, 0.92f, 1.00f, 1.00f);

    // Text
    colors[ImGuiCol_Text] = textNormal;
    colors[ImGuiCol_TextDisabled] = textDisabled;
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);

    // Input fields - modern dark inputs
    colors[ImGuiCol_FrameBg] = inputBg;
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);

    // Modal backgrounds
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.65f);

    // Tables
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = borderStrong;
    colors[ImGuiCol_TableBorderLight] = borderSubtle;
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.04f);

    // Drag and drop
    colors[ImGuiCol_DragDropTarget] = accentBlue;

    // Navigation
    colors[ImGuiCol_NavHighlight] = accentBlue;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

    // Style settings - modern spacing and rounding
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 6.0f;                                           // Smoother corners
    style.WindowBorderSize = 1.0f;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);                          // Left-aligned modern
    style.WindowMenuButtonPosition = ImGuiDir_Left;

    style.ChildRounding = 4.0f;
    style.ChildBorderSize = 1.0f;

    style.PopupRounding = 6.0f;
    style.PopupBorderSize = 1.0f;

    // Frame styling
    style.FramePadding = ImVec2(8.0f, 4.0f);                              // Comfortable padding
    style.FrameRounding = 4.0f;                                           // Modern rounded
    style.FrameBorderSize = 0.0f;

    // Item spacing - modern comfortable
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.CellPadding = ImVec2(6.0f, 3.0f);

    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);

    // Indent
    style.IndentSpacing = 16.0f;
    style.ColumnsMinSpacing = 6.0f;

    // Scrollbar
    style.ScrollbarSize = 14.0f;
    style.ScrollbarRounding = 8.0f;                                       // Smooth rounded

    // Grab elements
    style.GrabMinSize = 10.0f;
    style.GrabRounding = 4.0f;

    style.LogSliderDeadzone = 4.0f;

    // Tabs
    style.TabRounding = 4.0f;
    style.TabBorderSize = 0.0f;

    // Tables
    style.TableAngledHeadersAngle = 35.0f * 3.142f / 180.0f;

    // Color button
    style.ColorButtonPosition = ImGuiDir_Right;

    // Button text alignment
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.5f);

    // Safe area padding
    style.DisplaySafeAreaPadding = ImVec2(3.0f, 3.0f);

    // Font configuration
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->Clear();

    ImFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 2;
    config.RasterizerMultiply = 1.25f;
    io.FontDefault = io.Fonts->AddFontFromFileTTF("P:/Projects/VS/Wiley/Wiley/Assets/Fonts/OpenSans/OpenSans-Regular.ttf", 15.0f, &config);

    io.Fonts->Build();
}