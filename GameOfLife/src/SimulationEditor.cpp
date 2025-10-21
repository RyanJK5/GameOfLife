#include <exception>

#include "Logging.h"
#include "SimulationEditor.h"

gol::SimulationEditor::SimulationEditor(Size2 windowSize, Size2 gridSize)
    : m_Grid(gridSize)
    , m_Graphics(std::filesystem::path("resources") / "shader" / "default.shader", windowSize.Width, windowSize.Height)
{ }   

gol::GameState gol::SimulationEditor::Update(const SimulationEditorArgs& args)
{
    GraphicsHandlerArgs graphicsArgs = { ViewportBounds(), m_Grid.Size(), 1.f };

    UpdateViewport();
    m_Graphics.RescaleFrameBuffer(WindowBounds().Size());
    m_Graphics.ClearBackground(WindowBounds(), graphicsArgs.ViewportBounds);

    GameState state = args.Action == GameAction::None 
        ? args.State 
        : UpdateState(args.Action);

    state = [&]()
    {
        switch (state)
        {
        case GameState::Simulation:
            return SimulationUpdate(graphicsArgs);
        case GameState::Paint:
            return PaintUpdate(graphicsArgs);
        case GameState::Paused:
            return PauseUpdate(graphicsArgs);
        case GameState::Empty:
            return PaintUpdate(graphicsArgs);
        };
    }();

    DisplaySimulation();
    return state;
}


gol::GameState gol::SimulationEditor::SimulationUpdate(const GraphicsHandlerArgs& args)
{
    const bool success = glfwGetTime() * 1000 >= m_TickDelayMs;
    if (success)
    {
        glfwSetTime(0);
        m_Grid.Update();
        if (m_Grid.Dead())
            return GameState::Empty;
    }
    m_Graphics.DrawGrid(m_Grid.Data(), args);
    return GameState::Simulation;
}

gol::GameState gol::SimulationEditor::PaintUpdate(const GraphicsHandlerArgs& args)
{
    m_Graphics.DrawGrid(m_Grid.Data(), args);

    auto gridPos = CursorGridPos();
    if (gridPos)
    {
        m_Graphics.DrawSelection(*gridPos, args);
        if (gridPos->X >= 0 && gridPos->Y >= 0 && gridPos->X < m_Grid.Width() && gridPos->Y < m_Grid.Height())
            UpdateMouseState(*gridPos);
    }

    return m_Grid.Dead() 
        ? GameState::Empty 
        : GameState::Paint;
}

gol::GameState gol::SimulationEditor::PauseUpdate(const GraphicsHandlerArgs& args)
{
    m_Graphics.DrawGrid(m_Grid.Data(), args);
    return GameState::Paused;
}

void gol::SimulationEditor::DisplaySimulation()
{
    ImGui::Begin("Simulation");
    {
        ImGui::BeginChild("GameRender");

        m_WindowBounds = { Vec2F(ImGui::GetWindowPos()), Size2F(ImGui::GetContentRegionAvail()) };

        ImGui::Image(
            static_cast<ImTextureID>(m_Graphics.TextureID()),
            ImGui::GetContentRegionAvail(),
            ImVec2(0, 1),
            ImVec2(1, 0)
        );
    }
    ImGui::EndChild();
    ImGui::End();
}

gol::Rect gol::SimulationEditor::WindowBounds() const
{
    return Rect(
        static_cast<int32_t>(m_WindowBounds.X),
        static_cast<int32_t>(m_WindowBounds.Y),
        static_cast<int32_t>(m_WindowBounds.Width),
        static_cast<int32_t>(m_WindowBounds.Height)
    );
}

gol::Rect gol::SimulationEditor::ViewportBounds() const
{
    return WindowBounds();
}

std::optional<gol::Vec2> gol::SimulationEditor::CursorGridPos()
{
    Rect view = ViewportBounds();
    Vec2F cursor = ImGui::GetMousePos();
    if (!view.InBounds(cursor.X, cursor.Y))
        return std::nullopt;

    m_Graphics.Camera.Center = { ViewportBounds().Width / 2.f, ViewportBounds().Height / 2.f};
    glm::vec2 vec = { 
        (cursor.X - view.X) , 
        (cursor.Y - view.Y) };
    glm::vec2 center = m_Graphics.Camera.Center;
    vec -= center;
    vec /= m_Graphics.Camera.Zoom;
    vec += center;
    vec /= glm::vec2 { (static_cast<float>(view.Width) / m_Grid.Width()), (static_cast<float>(view.Height) / m_Grid.Height()) };

    return Vec2(static_cast<int32_t>(vec.x), static_cast<int32_t>(vec.y));
}

gol::GameState gol::SimulationEditor::UpdateState(GameAction action)
{
    switch (action)
    {
    case GameAction::Start:
        m_InitialGrid = m_Grid;
        return GameState::Simulation;
    case GameAction::Clear:
        m_Grid = GameGrid(m_Grid.Size());
        return GameState::Paint;
    case GameAction::Reset:
        m_Grid = m_InitialGrid;
        return GameState::Paint;
    case GameAction::Restart:
        m_Grid = m_InitialGrid;
        return GameState::Simulation;
    case GameAction::Pause:
        return GameState::Paused;
    case GameAction::Resume:
        return GameState::Simulation;
    case GameAction::None:
        throw std::exception("Cannot pass 'None' as action to UpdateState");
    }
}

void gol::SimulationEditor::UpdateMouseState(Vec2 gridPos)
{
    bool mouseState = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    
    if (mouseState)
    {
        if (m_DrawMode == DrawMode::None)
            m_DrawMode = *m_Grid.Get(gridPos.X, gridPos.Y) ? DrawMode::Delete : DrawMode::Insert;

        m_Grid.Set(gridPos.X, gridPos.Y, m_DrawMode == DrawMode::Insert);
        return;
    }
    m_DrawMode = DrawMode::None;
}

void gol::SimulationEditor::UpdateViewport()
{
    Rect bounds = ViewportBounds();
    
    if (bounds.InBounds(ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y))
        m_Graphics.ZoomBy(ImGui::GetIO().MouseWheel / 10.f);

    glViewport(bounds.X - m_WindowBounds.X, bounds.Y - m_WindowBounds.Y, bounds.Width, bounds.Height);
}