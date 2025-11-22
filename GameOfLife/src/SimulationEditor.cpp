#include <exception>
#include <cmath>
#include <format>
#include <utility>

#include "RLEEncoder.h"
#include "SimulationEditor.h"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "GameEnums.h"
#include "GameGrid.h"
#include "Graphics2D.h"
#include "GraphicsHandler.h"
#include "SimulationControlResult.h"
#include "glm/fwd.hpp"
#include "imgui.h"
#include "VersionManager.h"
#include <vector>
#include <array>

gol::SimulationEditor::SimulationEditor(Size2 windowSize, Size2 gridSize)
    : m_Grid(gridSize)
    , m_Graphics(std::filesystem::path("resources") / "shader" / "default.shader", windowSize.Width, windowSize.Height)
{ }   

gol::GameState gol::SimulationEditor::Update(const SimulationControlResult& args)
{
    GraphicsHandlerArgs graphicsArgs = { .ViewportBounds = ViewportBounds(), .GridSize = m_Grid.Size() };

    UpdateViewport();
    UpdateDragState();
    m_Graphics.RescaleFrameBuffer(WindowBounds().Size());
    m_Graphics.ClearBackground(graphicsArgs);
    

    if (args.TickDelayMs)
        m_TickDelayMs = *args.TickDelayMs;

    GameState state = args.Action == GameAction::None 
        ? args.State 
        : UpdateState(args);

    state = [&]()
    {
        switch (state)
        {
        using enum GameState;
        case Simulation:
            return SimulationUpdate(graphicsArgs);
        case Paint:
            return PaintUpdate(graphicsArgs);
        case Paused:
            return PauseUpdate(graphicsArgs);
        case Empty:
            return PaintUpdate(graphicsArgs);
        };
        std::unreachable();
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
        if (m_Grid.Dead() && (!m_Selected || m_Selected->Dead()))
            return GameState::Empty;
    }
    m_Graphics.DrawGrid({ 0, 0 }, m_Grid.Data(), args);
    return GameState::Simulation;
}

gol::GameState gol::SimulationEditor::PaintUpdate(const GraphicsHandlerArgs& args)
{
    m_Graphics.DrawGrid({ 0, 0 }, m_Grid.Data(), args);
    if (m_AnchorSelection != m_SentinelSelection && m_Selected)
        m_Graphics.DrawGrid(SelectionBounds().UpperLeft(), m_Selected->Data(), args);

    auto gridPos = CursorGridPos();
    if (m_AnchorSelection && m_SentinelSelection && (m_AnchorSelection != m_SentinelSelection))
        m_Graphics.DrawSelection(SelectionBounds(), args);
    if (gridPos)
    {
        if (m_AnchorSelection && m_SentinelSelection)
            m_Graphics.DrawSelection(SelectionBounds(), args);
        UpdateMouseState(*gridPos);
    }

    return (m_Grid.Dead() && (!m_Selected || m_Selected->Dead()))
        ? GameState::Empty 
        : GameState::Paint;
}

gol::GameState gol::SimulationEditor::PauseUpdate(const GraphicsHandlerArgs& args)
{
    auto gridPos = CursorGridPos();
    if (gridPos)
        UpdateSelectionArea(*gridPos);
    if (m_AnchorSelection && m_SentinelSelection)
        m_Graphics.DrawSelection(SelectionBounds(), args);
    m_Graphics.DrawGrid({ 0, 0 }, m_Grid.Data(), args);
    if (m_AnchorSelection != m_SentinelSelection && m_Selected)
        m_Graphics.DrawGrid(std::min(*m_AnchorSelection, *m_SentinelSelection), m_Selected->Data(), args);
    return GameState::Paused;
}

void gol::SimulationEditor::DisplaySimulation()
{
    ImGui::Begin("Simulation");
    ImGui::BeginChild("Render");

    ImDrawListSplitter splitter;
    splitter.Split(ImGui::GetWindowDrawList(), 2);

    splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), 0);
    m_WindowBounds = { Vec2F(ImGui::GetWindowPos()), Size2F(ImGui::GetContentRegionAvail()) };

    ImGui::Image(
        static_cast<ImTextureID>(m_Graphics.TextureID()),
        ImGui::GetContentRegionAvail(),
        ImVec2(0, 1),
        ImVec2(1, 0)
    );

    splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), 1);
    ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin());
    ImGui::Text(std::format("Generation: {}", m_Grid.Generation()).c_str());
    ImGui::Text(std::format("Population: {}", m_Grid.Population() + (m_Selected ? m_Selected->Population() : 0)).c_str());

    auto cursorPos = CursorGridPos();
    if (cursorPos)
    {
        Vec2 pos = m_AnchorSelection ? *m_AnchorSelection : *cursorPos;
        std::string text = std::format("({}, {})", pos.X, pos.Y);
        if (m_SentinelSelection != m_AnchorSelection)
            text += std::format(" X ({}, {})", m_SentinelSelection->X, m_SentinelSelection->Y);
        ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - ImGui::CalcTextSize(text.c_str()).y);
        ImGui::Text(text.c_str());
    }

    splitter.Merge(ImGui::GetWindowDrawList());
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

void gol::SimulationEditor::RemoveSelection(bool updateVersion)
{
    if (m_Selected)
    {
        auto insertedCells = m_Grid.InsertGrid(*m_Selected, SelectionBounds().UpperLeft());
        if (updateVersion)
            m_VersionManager.AddSelectionChange
            ({
                .Action = GameAction::Deselect,
				.SelectionBounds = SelectionBounds(),
                .CellsInserted = insertedCells,
                .CellsDeleted = m_Selected->Data()
            });
    }

    m_AnchorSelection   = std::nullopt;
    m_SentinelSelection = std::nullopt;
    m_Selected          = std::nullopt;
}

void gol::SimulationEditor::CopySelection() {
    if (!m_Selected)
        return;
    ImGui::SetClipboardText(
        reinterpret_cast<const char*>(
            RLEEncoder::EncodeRegion<uint32_t>(*m_Selected, {0, 0, m_Selected->Width(), m_Selected->Height()}).data()
        )
    );
    RemoveSelection();
}

void gol::SimulationEditor::PasteSelection()
{
    auto gridPos = CursorGridPos();
    if (!gridPos)
        gridPos = m_AnchorSelection;
    if (!gridPos)
        return;

    RemoveSelection(true);

    m_Selected = RLEEncoder::DecodeRegion<uint32_t>(ImGui::GetClipboardText());
    m_AnchorSelection = gridPos;
    m_SentinelSelection = { gridPos->X + m_Selected->Width() - 1, gridPos->Y + m_Selected->Height() - 1 };
    
    m_VersionManager.AddSelectionChange
    ({
        .Action = GameAction::Paste,
        .SelectionBounds = SelectionBounds(),
        .CellsInserted = m_Selected->Data(),
        .CellsDeleted = {}
    });
}

void gol::SimulationEditor::DeleteSelection(bool cut)
{
    if (!m_Selected)
        return;
    
    auto bounds = SelectionBounds();
    m_VersionManager.AddSelectionChange
    ({
        .Action = GameAction::Delete,
        .SelectionBounds = SelectionBounds(),
        .CellsInserted = {},
        .CellsDeleted = m_Selected->Data()
    });
    
    if (cut)
        ImGui::SetClipboardText(
            reinterpret_cast<const char*>(
                RLEEncoder::EncodeRegion<uint32_t>(*m_Selected, { 0, 0, m_Selected->Width(), m_Selected->Height() }).data()
            )
        );

    m_Selected = std::nullopt;
	m_AnchorSelection = std::nullopt;
	m_SentinelSelection = std::nullopt;
}

void gol::SimulationEditor::RotateSelection(bool clockwise, bool updateVersion)
{
    if (!m_Selected)
        return;

    auto upperLeft = SelectionBounds().UpperLeft();
    auto width = static_cast<float>(m_Selected->Width());
    auto height = static_cast<float>(m_Selected->Height());

    auto gridCenter = Vec2F
    {
        static_cast<float>(upperLeft.X) + width / 2.f,
        static_cast<float>(upperLeft.Y) + height / 2.f
    };

    m_AnchorSelection = Graphics2D::Rotate(gridCenter, clockwise ? SelectionBounds().LowerLeft() : SelectionBounds().UpperRight(), clockwise);
    m_SentinelSelection = *m_AnchorSelection + Vec2{ m_Selected->Height() - 1, m_Selected->Width() - 1 };
    m_Selected->RotateGrid();

    if (updateVersion)
        m_VersionManager.AddSelectionChange
        ({
            .Action = GameAction::Rotate,
            .SelectionBounds = SelectionBounds(),
        });
}

void gol::SimulationEditor::NudgeSelection(GameAction nudgeType, Vec2 direction, bool updateVersion)
{
    if (!m_AnchorSelection || (m_AnchorSelection == m_SentinelSelection))
        return;

    *m_AnchorSelection += direction;
    *m_SentinelSelection += direction;

    if (!updateVersion)
        return;
    m_VersionManager.AddSelectionChange
    ({
        .Action = nudgeType,
        .SelectionBounds = SelectionBounds(),
        .NudgeTranslation = direction
    });
}

gol::Rect gol::SimulationEditor::SelectionBounds() const
{
    return 
    {
        std::min(m_AnchorSelection->X, m_SentinelSelection->X),
        std::min(m_AnchorSelection->Y, m_SentinelSelection->Y),
        std::abs(m_SentinelSelection->X - m_AnchorSelection->X) + 1,
        std::abs(m_SentinelSelection->Y - m_AnchorSelection->Y) + 1
    };
}

void gol::SimulationEditor::SetSelectionBounds(const Rect& rect)
{
    m_AnchorSelection = rect.Pos();
    m_SentinelSelection = *m_AnchorSelection + Vec2
    {
        rect.Width - 1,
        rect.Height - 1
    };
}

std::optional<gol::Vec2> gol::SimulationEditor::CursorGridPos()
{
    Vec2F cursor = ImGui::GetMousePos();
    if (!ViewportBounds().InBounds(cursor.X, cursor.Y))
        return std::nullopt;

    glm::vec2 vec = m_Graphics.Camera.ScreenToWorldPos(cursor, ViewportBounds());
    glm::vec2 other = m_Graphics.Camera.ScreenToWorldPos(Vec2F{ (float)ViewportBounds().X, (float)ViewportBounds().Y }, ViewportBounds());
    vec /= glm::vec2 { DefaultCellWidth  , 
                       DefaultCellHeight };
    
    Vec2 result = { static_cast<int32_t>(std::floor(vec.x)), static_cast<int32_t>(std::floor(vec.y)) };
    if (!m_Grid.InBounds(result))
        return std::nullopt;
    return result;
}

void gol::SimulationEditor::UpdateVersion(const SimulationControlResult& args)
{
    auto versionChanges = args.Action == GameAction::Undo
        ? m_VersionManager.Undo()
		: m_VersionManager.Redo();
    if (!versionChanges)
        return;

    if (!versionChanges->InsertedIntoSelection())
    {
        RestoreGridVersion(args.Action, m_Grid, *versionChanges);
        return;
    }

    switch (versionChanges->Action)
    {
    using enum GameAction;
	case NudgeLeft:  [[fallthrough]];
	case NudgeRight: [[fallthrough]];
	case NudgeUp:    [[fallthrough]];
    case NudgeDown:
        if (args.Action == Undo)
            NudgeSelection(versionChanges->Action, -versionChanges->NudgeTranslation, false);
        else
            NudgeSelection(versionChanges->Action, versionChanges->NudgeTranslation, false);
        return;
    case Rotate:
        RotateSelection(args.Action == Redo, false);
        return;
    case Paste: [[fallthrough]];
    case Delete:
        SetSelectionBounds(*versionChanges->SelectionBounds);
        m_Selected = GameGrid(versionChanges->SelectionBounds->Size());
        RestoreGridVersion(args.Action, *m_Selected, *versionChanges);

        if ((args.Action == Redo) == (versionChanges->Action == Delete))
            RemoveSelection(false);
        return;
    case Select:
        if (args.Action == Undo)
        {
			RemoveSelection(false);
            return;
        }
        else
        {
        	SetSelectionBounds(*versionChanges->SelectionBounds);
            m_Selected = GameGrid(versionChanges->SelectionBounds->Size());
            for (auto pos : versionChanges->CellsInserted)
                m_Selected->Set(pos.X, pos.Y, true);
            for (auto pos : versionChanges->CellsDeleted)
                m_Grid.Set(pos.X, pos.Y, false);
        }
        return;
    case Deselect:
        if (args.Action == Undo)
        {
			SetSelectionBounds(*versionChanges->SelectionBounds);
            m_Selected = GameGrid(versionChanges->SelectionBounds->Size());
            for (auto pos : versionChanges->CellsDeleted)
                m_Selected->Set(pos.X, pos.Y, true);
            for (auto pos : versionChanges->CellsInserted)
                m_Grid.Set(pos.X, pos.Y, false);
        }
        else
        {
			RemoveSelection(false);
            return;
        }
    }
}

void gol::SimulationEditor::RestoreGridVersion(GameAction undoRedo, GameGrid& grid, const VersionChange& versionChanges)
{
    auto& insertions = undoRedo == GameAction::Undo
        ? versionChanges.CellsDeleted
        : versionChanges.CellsInserted;
    for (auto& pos : insertions)
        grid.Set(pos.X, pos.Y, true);

    auto& deletions = undoRedo == GameAction::Undo
        ? versionChanges.CellsInserted
        : versionChanges.CellsDeleted;
    for (auto& pos : deletions)
        grid.Set(pos.X, pos.Y, false);
}

gol::GameState gol::SimulationEditor::UpdateState(const SimulationControlResult& result)
{
    switch (result.Action)
    {
    using enum GameAction;
    case Start:
        RemoveSelection();
        m_InitialGrid = m_Grid;
        return GameState::Simulation;
    case Clear:
        RemoveSelection();
        m_Grid = GameGrid(m_Grid.Size());
        return GameState::Paint;
    case Reset:
        m_Grid = m_InitialGrid;
        return GameState::Paint;
    case Restart:
        m_Grid = m_InitialGrid;
        return GameState::Simulation;
    case Pause:
        return GameState::Paused;
    case Resume:
        RemoveSelection();
        return GameState::Simulation;
    case Step:
        for (int32_t i = 0; i < *result.StepCount; i++)
            m_Grid.Update();
        return m_Grid.Dead() ? GameState::Empty : GameState::Paused;
    case Resize:
        m_Grid = GameGrid(m_Grid, *result.NewDimensions);
        m_Graphics.Camera.Center = { result.NewDimensions->Width * DefaultCellWidth / 2.f, result.NewDimensions->Height * DefaultCellHeight / 2.f };
        return GameState::Paint;
    case Copy:
        CopySelection();
        return result.State;
    case Cut:
        DeleteSelection(true);
        return result.State;
    case Paste:
        PasteSelection();
        return result.State;
    case Delete:
        DeleteSelection(false);
        return result.State;
    case Deselect:
        RemoveSelection();
        return result.State;
    case NudgeLeft:
        NudgeSelection(result.Action, { -result.NudgeSize, 0 });
        return result.State;
    case NudgeRight:
        NudgeSelection(result.Action, { result.NudgeSize, 0 });
        return result.State;
    case NudgeUp:
        NudgeSelection(result.Action, { 0, -result.NudgeSize });
        return result.State;
    case NudgeDown:
        NudgeSelection(result.Action, { 0, result.NudgeSize });
        return result.State;
    case Rotate:
        RotateSelection(true);
        return result.State;
    case Undo:
        UpdateVersion(result);
        return result.State;
    case Redo:
		UpdateVersion(result);
        return result.State;
    }

    throw std::exception("Cannot pass 'None' as action to UpdateState");
}

void gol::SimulationEditor::UpdateMouseState(Vec2 gridPos)
{
    if (UpdateSelectionArea(gridPos))
        return;
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsKeyDown(ImGuiKey_LeftShift) && !ImGui::IsKeyDown(ImGuiKey_RightShift))
    {
        if (m_EditorMode == EditorMode::None)
        {
            if (m_SentinelSelection != m_AnchorSelection)
            {
                m_AnchorSelection = gridPos;
                m_SentinelSelection = gridPos;
            }
            m_EditorMode = *m_Grid.Get(gridPos.X, gridPos.Y) ? EditorMode::Delete : EditorMode::Insert;
            m_VersionManager.BeginPaintChange(gridPos, m_EditorMode == EditorMode::Insert);
        }
        if (*m_Grid.Get(gridPos.X, gridPos.Y) != (m_EditorMode == EditorMode::Insert))
			m_VersionManager.AddPaintChange(gridPos);
        m_Grid.Set(gridPos.X, gridPos.Y, m_EditorMode == EditorMode::Insert);
        return;
    }
    m_EditorMode = EditorMode::None;
}

bool gol::SimulationEditor::UpdateSelectionArea(Vec2 gridPos)
{ 
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift)))
    {
        m_EditorMode = EditorMode::Select;
        m_SentinelSelection = gridPos;
        if (!m_AnchorSelection)
            m_AnchorSelection = gridPos;
        return true;
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && (m_AnchorSelection != m_SentinelSelection))
    {
        if (!m_Selected)
        {
    		m_Selected = m_Grid.SubRegion(SelectionBounds());
            m_VersionManager.AddSelectionChange
            ({
                .Action = GameAction::Select,
                .SelectionBounds = SelectionBounds(),
                .CellsInserted = m_Selected->Data(),
                .CellsDeleted = m_Grid.ReadRegion(SelectionBounds())
            });
            m_Grid.ClearRegion(SelectionBounds());
        }
        return false;
    }

    RemoveSelection();
    m_AnchorSelection = gridPos;
    m_SentinelSelection = gridPos;
    return false;
}

void gol::SimulationEditor::UpdateDragState()
{
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        Vec2F delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
        m_Graphics.Camera.Translate(glm::vec2(delta) - m_DeltaLast);
        m_DeltaLast = delta;
    }
    else
    {
        m_DeltaLast = glm::vec2(0.f);
    }
}

void gol::SimulationEditor::UpdateViewport()
{
    const Rect bounds = ViewportBounds();
    
    auto mousePos = ImGui::GetIO().MousePos;
    if (bounds.InBounds(mousePos.x, mousePos.y) && ImGui::GetIO().MouseWheel != 0)
        m_Graphics.Camera.ZoomBy(mousePos, bounds, ImGui::GetIO().MouseWheel / 10.f);

    glViewport(static_cast<int32_t>(bounds.X - m_WindowBounds.X), static_cast<int32_t>(bounds.Y - m_WindowBounds.Y), bounds.Width, bounds.Height);
}