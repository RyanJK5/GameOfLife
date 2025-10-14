#include "Logging.h"
#include "GameWindow.h"
#include "GLException.h"

#include "vendor/imgui.h"
#include "vendor/imgui_internal.h"
#include "vendor/imgui_impl_glfw.h"
#include "vendor/imgui_impl_opengl3.h"

gol::GameWindow::GameWindow(Size2 size)
    : GameWindow(size.Width, size.Height)
{ }

gol::GameWindow::GameWindow(int32_t width, int32_t height)
    : m_WindowBounds(0, 0, width, height)
{
    if (!glfwInit())
        throw GLException("Failed to initialize glfw");

    m_Window = std::unique_ptr<GLFWwindow>(
        glfwCreateWindow(width, height, "Conway's Game of Life", NULL, NULL)
    );

    if (!m_Window)
        throw GLException("Failed to create window");

    glfwMakeContextCurrent(m_Window.get());
    GL_DEBUG(glLineWidth(4));
    GL_DEBUG(glfwSwapInterval(1));

    if (glewInit() != GLEW_OK)
        throw GLException("Failed to initialize glew");

    InitImGUI();
}

gol::GameWindow::~GameWindow()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void gol::GameWindow::InitImGUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

     ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= IOFlags;
    io.ConfigDebugHighlightIdConflicts = true;
    m_Font = io.Fonts->AddFontFromFileTTF("resources/font/arial.ttf", 30.0f);

    ImVec4 transparent = ImVec4(0, 0, 0, 0);
    ImVec4 background = ImVec4(0.1, 0.1, 0.1, 1);
    ImVec4 contrast = ImVec4(0.16, 0.16, 0.16, 1);
    ImVec4 hover = ImVec4(1, 1, 1, 0.2);
    ImVec4 text = ImVec4(0.8, 0.8, 0.8, 1);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowMenuButtonPosition = ImGuiDir_None;

    style.Colors[ImGuiCol_WindowBg] = background;
    style.Colors[ImGuiCol_Border] = contrast;

    style.Colors[ImGuiCol_Text] = text;

    style.Colors[ImGuiCol_Button] = contrast;
    style.Colors[ImGuiCol_ButtonHovered] = hover;
    
    style.Colors[ImGuiCol_Header] = contrast;
    style.Colors[ImGuiCol_HeaderActive] = contrast;

    style.Colors[ImGuiCol_TitleBg] = contrast;
    style.Colors[ImGuiCol_TitleBgActive] = contrast;

    style.Colors[ImGuiCol_Tab] = transparent;
    style.Colors[ImGuiCol_TabSelectedOverline] = transparent;
    style.Colors[ImGuiCol_TabDimmedSelected] = transparent;
    style.Colors[ImGuiCol_TabHovered] = transparent;
    style.Colors[ImGuiCol_TabUnfocused] = transparent;
    style.Colors[ImGuiCol_TabDimmed] = transparent;
    style.Colors[ImGuiCol_TabSelected] = transparent;

    ImGui_ImplGlfw_InitForOpenGL(m_Window.get(), true);
    ImGui_ImplOpenGL3_Init();
}

gol::Rect gol::GameWindow::WindowBounds() const
{
    return Rect(
        static_cast<int32_t>(m_WindowBounds.X), 
        static_cast<int32_t>(m_WindowBounds.Y), 
        static_cast<int32_t>(m_WindowBounds.Width),
        static_cast<int32_t>(m_WindowBounds.Height)
    );
}

gol::Rect gol::GameWindow::ViewportBounds(Size2 gridSize) const
{
    Rect window = WindowBounds();
    const float widthRatio = static_cast<float>(window.Width) / gridSize.Width;
    const float heightRatio = static_cast<float>(window.Height) / gridSize.Height;
    if (widthRatio > heightRatio)
    {
        const int32_t newWidth = static_cast<int32_t>(heightRatio * gridSize.Width);
        const int32_t newX = (window.Width - newWidth) / 2;
        return Rect { window.X + newX, window.Y, newWidth, window.Height};
    }
    const int32_t newHeight = static_cast<int32_t>(widthRatio * gridSize.Height);
    const int32_t newY = (window.Height - newHeight) / 2;
    return Rect { window.X, window.Y + newY, window.Width, newHeight };
}

gol::Vec2F gol::GameWindow::CursorPos() const
{
    return ImGui::GetMousePos();
}

void gol::GameWindow::BeginFrame()
{
    GL_DEBUG(glfwPollEvents());
    GL_DEBUG(glClear(GL_COLOR_BUFFER_BIT));

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void gol::GameWindow::DisplaySimulation(uint32_t textureID)
{
    ImGui::Begin("Simulation");
    {
        ImGui::BeginChild("GameRender");

        m_WindowBounds = { Vec2F(ImGui::GetWindowPos()), Size2F(ImGui::GetContentRegionAvail()) };

        ImGui::Image(
            (ImTextureID)textureID,
            ImGui::GetContentRegionAvail(),
            ImVec2(0, 1),
            ImVec2(1, 0)
        );
    }
    ImGui::EndChild();
    ImGui::End();
}

gol::GameAction gol::GameWindow::DisplaySimulationControl(const DrawInfo& info)
{
    ImGui::Begin("Simulation Control");

    auto disableBegin = [](bool enabled)
    {
        if (enabled)
            return;
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    };
    auto disableEnd = [](bool enabled)
    {
        if (enabled)
            return;
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    };

    GameAction result = GameAction::None;

    bool enterShortcut = m_EnterShortcut.Update();

    bool startEnabled = !info.GridDead && (info.State == GameState::Paint || info.State == GameState::Paused);
    disableBegin(startEnabled);
    if (ImGui::Button("Start", { 100, 50 }) || (startEnabled && enterShortcut))
        result = GameAction::Start;
    disableEnd(startEnabled);

    bool clearEnabled = !info.GridDead;
    disableBegin(clearEnabled);
    ImGui::SameLine();
    ImGui::Button("Clear", { 100, 50 });
    if (ImGui::IsItemActivated())
        result = GameAction::Clear;
    disableEnd(clearEnabled);

    bool resetEnabled = info.State == GameState::Simulation;
    disableBegin(resetEnabled);
    ImGui::SameLine();
    ImGui::Button("Reset", { 100, 50 });
    if (ImGui::IsItemActivated() || (resetEnabled && enterShortcut))
        result = GameAction::Reset;
    disableEnd(resetEnabled);

    ImGui::End();

    return result;
}

gol::UpdateInfo gol::GameWindow::CreateGUI(const DrawInfo& info)
{
    UpdateInfo result = {};

    ImGui::PushFont(m_Font, 30.0f);
    CreateDockspace();

    result.Action = DisplaySimulationControl(info);
    DisplaySimulation(info.SimulationTextureID);

    ImGui::Begin("Presets");
    ImGui::Text("Hello, down!");
    ImGui::End();

    ImGui::PopFont();

    return result;
}

void gol::GameWindow::CreateDockspace()
{
    ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Once);

    int32_t windowWidth{}, windowHeight{};
    glfwGetFramebufferSize(m_Window.get(), &windowWidth, &windowHeight);
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

    ImGui::Begin("DockSpace", nullptr, DockspaceFlags | ImGuiWindowFlags_NoTitleBar);
    ImGuiID dockspaceID = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    if (m_Startup)
    {
        InitDockspace(dockspaceID);
        m_Startup = false;
    }

    ImGui::End();
}

void gol::GameWindow::InitDockspace(uint32_t dockspaceID)
{
    ImGui::DockBuilderRemoveNode(dockspaceID);
    ImGui::DockBuilderAddNode(
        dockspaceID,
        ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace
    );
    ImGui::DockBuilderSetNodeSize(dockspaceID, { m_WindowBounds.Width, m_WindowBounds.Height });

    ImGuiID leftID{}, rightID{};
    ImGui::DockBuilderSplitNode(
        dockspaceID,
        ImGuiDir_Left,
        0.25f,
        &leftID,
        &rightID
    );
    auto downID = ImGui::DockBuilderSplitNode(
        rightID,
        ImGuiDir_Down,
        0.25f,
        nullptr,
        &rightID
    );

    // we now dock our windows into the docking node we made above
    ImGui::DockBuilderDockWindow("Presets", downID);
    ImGui::DockBuilderDockWindow("Simulation", rightID);
    ImGui::DockBuilderDockWindow("Simulation Control", leftID);
    ImGui::DockBuilderFinish(dockspaceID);
}

void gol::GameWindow::EndFrame() const
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);

    GL_DEBUG(glfwSwapBuffers(m_Window.get()));
}

void gol::GameWindow::UpdateViewport(Size2 gridSize) const
{
    Rect bounds = ViewportBounds(gridSize);
    glViewport(bounds.X - m_WindowBounds.X, bounds.Y - m_WindowBounds.Y, bounds.Width, bounds.Height);
}