#include "Themes.h"


void SetBlenderTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

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

    colors[ImGuiCol_TabSelectedOverline] = ImVec4(1, 0, 0, 1);

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
    style.ScrollbarSize = 8.0f;
    style.ScrollbarRounding = 8.0f;                                       // Smooth rounded

    // Grab elements
    style.GrabMinSize = 10.0f;
    style.GrabRounding = 4.0f;

    style.LogSliderDeadzone = 4.0f;

    // Tabs
    style.TabRounding = 0.0f;
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
    io.FontDefault = io.Fonts->AddFontFromFileTTF("P:/Projects/VS/Wiley/Wiley/Assets/Fonts/OpenSans/OpenSans-SemiBold.ttf", 16.0f, &config);

    io.Fonts->Build();
}

void ThemeTest()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.04f, 0.04f, 0.04f, 0.99f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.51f, 0.51f, 0.80f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.03f, 0.03f, 0.04f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.17f, 0.17f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.30f, 0.60f, 0.10f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.60f, 0.10f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.43f, 0.90f, 0.11f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.21f, 0.22f, 0.23f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.51f, 0.51f, 0.80f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.54f, 0.55f, 0.55f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.38f, 0.51f, 0.51f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.16f, 0.16f, 0.16f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.23f, 0.23f, 0.24f, 0.80f);
    colors[ImGuiCol_Tab] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.13f, 0.78f, 0.07f, 1.00f);
    colors[ImGuiCol_TabDimmed] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.10f, 0.60f, 0.12f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.14f, 0.87f, 0.05f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.30f, 0.60f, 0.10f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.23f, 0.78f, 0.02f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.46f, 0.47f, 0.46f, 0.06f);
    colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavCursor] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.78f, 0.69f, 0.69f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    style.WindowRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 3.0f;
    style.PopupRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.WindowMenuButtonPosition = ImGuiDir_Right;
    style.ScrollbarSize = 10.0f;
    style.GrabMinSize = 10.0f;
    style.DockingSeparatorSize = 1.0f;
    style.SeparatorTextBorderSize = 2.0f;

    // Font configuration
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->Clear();

    ImFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 2;
    config.RasterizerMultiply = 1.25f;
    io.FontDefault = io.Fonts->AddFontFromFileTTF("P:/Projects/VS/Wiley/Wiley/Assets/Fonts/OpenSans/OpenSans-SemiBold.ttf", 16.0f, &config);

    io.Fonts->Build();
}

void SetD5RendererStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Main styling parameters
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.TouchExtraPadding = ImVec2(0, 0);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;

    // Borders
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;

    // Rounding
    style.WindowRounding = 6.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;

    // Alignment
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    // Color scheme - D5 Renderer inspired dark theme

    // Background colors
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.26f, 0.27f, 0.80f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frame colors
    colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.23f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.27f, 0.28f, 1.00f);

    // Title colors
    colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.11f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.11f, 0.12f, 0.75f);

    // Menu colors
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.13f, 0.14f, 1.00f);

    // Scrollbar colors
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.12f, 0.13f, 0.14f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.35f, 0.36f, 0.37f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.46f, 0.47f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.55f, 0.56f, 0.57f, 0.80f);

    // Check mark colors (D5 blue accent)
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

    // Slider colors
    colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.36f, 0.69f, 1.00f, 1.00f);

    // Button colors (D5 blue accent)
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.48f, 0.82f, 0.80f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.16f, 0.39f, 0.72f, 1.00f);

    // Header colors (for collapsing headers, tree nodes)
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.48f, 0.82f, 0.60f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

    // Separator colors
    colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.26f, 0.27f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

    // Resize grip colors
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

    // Tab colors
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.48f, 0.82f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.13f, 0.14f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.19f, 0.20f, 1.00f);

    // Docking colors
    colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

    // Plot colors
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

    // Table colors
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);

    // Text selection colors
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);

    // Drag and drop colors
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);

    // Navigation colors
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

    // Modal window dimming
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.75f);
}

// Alternative variant with cyan accents instead of blue
void SetD5RendererStyleCyan()
{
    SetD5RendererStyle();

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Override with cyan accents
    colors[ImGuiCol_CheckMark] = ImVec4(0.20f, 0.80f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.20f, 0.80f, 0.90f, 0.80f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.90f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.15f, 0.65f, 0.75f, 0.80f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.80f, 0.90f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.12f, 0.55f, 0.65f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.15f, 0.65f, 0.75f, 0.60f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.80f, 0.90f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.80f, 0.90f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.20f, 0.80f, 0.90f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.20f, 0.80f, 0.90f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.20f, 0.80f, 0.90f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.20f, 0.80f, 0.90f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.20f, 0.80f, 0.90f, 0.95f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.80f, 0.90f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.15f, 0.65f, 0.75f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.20f, 0.80f, 0.90f, 0.70f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.80f, 0.90f, 0.35f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.20f, 0.80f, 0.90f, 1.00f);
}