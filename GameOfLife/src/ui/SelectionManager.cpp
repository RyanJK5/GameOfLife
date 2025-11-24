#include <algorithm>
#include <cmath>
#include <cstdint>
#include <expected>
#include <imgui/imgui.h>
#include <optional>
#include <set>
#include <utility>
#include <variant>

#include "GameEnums.h"
#include "GameGrid.h"
#include "Graphics2D.h"
#include "RLEEncoder.h"
#include "SelectionManager.h"
#include "VersionManager.h"

gol::SelectionUpdateResult gol::SelectionManager::UpdateSelectionArea(GameGrid& grid, Vec2 gridPos)
{
    const bool shiftDown = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && shiftDown)
    {
        m_SentinelSelection = gridPos;
        if (!m_AnchorSelection)
            m_AnchorSelection = gridPos;
        return { .BeginSelection = true };
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && (m_AnchorSelection != m_SentinelSelection))
    {
        if (m_Selected)
            return { .BeginSelection = false };
    
        m_Selected = grid.SubRegion(SelectionBounds());
        auto change = VersionChange
        ({
            .Action = SelectionAction::Select,
            .SelectionBounds = SelectionBounds(),
            .CellsInserted = m_Selected->Data(),
            .CellsDeleted = grid.ReadRegion(SelectionBounds())
        });
        grid.ClearRegion(SelectionBounds());

        return { .Change = change, .BeginSelection = false };
    }

    Deselect(grid);
    m_AnchorSelection = gridPos;
    m_SentinelSelection = gridPos;
    return { .BeginSelection = false };
}

bool gol::SelectionManager::TryResetSelection()
{
    if (CanDrawGrid())
        return false;
    m_AnchorSelection = std::nullopt;
    m_SentinelSelection = std::nullopt;
    m_Selected = std::nullopt;
    return true;
}

std::optional<gol::VersionChange> gol::SelectionManager::Deselect(GameGrid& grid)
{
    const auto retValue = [this, &grid]()
    {
        if (!m_Selected)
            return std::optional<VersionChange> {};

        auto insertedCells = grid.InsertGrid(*m_Selected, SelectionBounds().UpperLeft());
        return std::optional<VersionChange>
        {{
            .Action = SelectionAction::Deselect,
            .SelectionBounds = SelectionBounds(),
            .CellsInserted = insertedCells,
            .CellsDeleted = m_Selected->Data()
        }};
    }();

    m_AnchorSelection = std::nullopt;
    m_SentinelSelection = std::nullopt;
    m_Selected = std::nullopt;

    return retValue;
}

std::optional<gol::VersionChange> gol::SelectionManager::Copy(GameGrid& grid)
{
    if (!m_Selected)
        return std::nullopt;

    auto encodeType = RLEEncoder::SelectStorageType(m_Selected->Width() * m_Selected->Height());
    if (std::get_if<uint8_t>(&encodeType))
		return Copy<uint8_t>(grid, false);
	else if (std::get_if<uint16_t>(&encodeType))
        return Copy<uint16_t>(grid, false);
    else if (std::get_if<uint32_t>(&encodeType))
        return Copy<uint32_t>(grid, false);
    else
		return Copy<uint64_t>(grid, false);
}

std::optional<gol::VersionChange> gol::SelectionManager::Cut()
{
    auto encodeType = RLEEncoder::SelectStorageType(m_Selected->Width() * m_Selected->Height());
    if (std::get_if<uint8_t>(&encodeType))
        return Cut<uint8_t>(false);
    else if (std::get_if<uint16_t>(&encodeType))
        return Cut<uint16_t>(false);
    else if (std::get_if<uint32_t>(&encodeType))
        return Cut<uint32_t>(false);
    else
        return Cut<uint64_t>(false);
}

std::expected<gol::VersionChange, uint32_t> gol::SelectionManager::Paste(std::optional<Vec2> gridPos, uint32_t warnThreshold)
{
	auto result8 =  Paste<uint8_t>(gridPos,  static_cast<uint8_t>(warnThreshold), false);
    if (result8 || result8.error() != std::numeric_limits<uint8_t>::max())
        return result8;
	
    auto result16 = Paste<uint16_t>(gridPos, static_cast<uint16_t>(warnThreshold), false);
    if (result16 || result16.error() != std::numeric_limits<uint16_t>::max())
		return result16;
    
    auto result32 = Paste<uint32_t>(gridPos, warnThreshold, false);
    if (result32 || result32.error() != std::numeric_limits<uint32_t>::max())
        return result32;
	
    auto result64 = Paste<uint64_t>(gridPos, warnThreshold, false);
    if (result64)
        return result64;
	
    return std::unexpected { result64.error() };
}

std::optional<gol::VersionChange> gol::SelectionManager::Delete()
{
    if (!m_Selected)
        return std::nullopt;

    auto bounds = SelectionBounds();
    auto retValue = VersionChange
    {
        .Action = SelectionAction::Delete,
        .SelectionBounds = SelectionBounds(),
        .CellsInserted = {},
        .CellsDeleted = m_Selected->Data()
    };

    m_Selected = std::nullopt;
    m_AnchorSelection = std::nullopt;
    m_SentinelSelection = std::nullopt;

    return retValue;
}

std::optional<gol::VersionChange> gol::SelectionManager::Rotate(bool clockwise)
{
    if (!m_Selected)
        return std::nullopt;

    auto upperLeft = SelectionBounds().UpperLeft();
    auto width = static_cast<float>(m_Selected->Width());
    auto height = static_cast<float>(m_Selected->Height());

    auto gridCenter = Vec2F
    {
        static_cast<float>(upperLeft.X) + width / 2.f,
        static_cast<float>(upperLeft.Y) + height / 2.f
    };

    auto oldCorner = Vec2F{ clockwise ? SelectionBounds().LowerLeft() : SelectionBounds().UpperRight() };
    m_AnchorSelection = RotatePoint(gridCenter, oldCorner, clockwise);
    m_SentinelSelection = *m_AnchorSelection + Vec2{ m_Selected->Height() - 1, m_Selected->Width() - 1 };
    m_Selected->RotateGrid(clockwise);

    return VersionChange
    {
        .Action = SelectionAction::Rotate,
        .SelectionBounds = SelectionBounds(),
    };
}

std::optional<gol::VersionChange> gol::SelectionManager::Nudge(Vec2 translation)
{
    if (!m_AnchorSelection || (m_AnchorSelection == m_SentinelSelection))
        return std::nullopt;

    *m_AnchorSelection += translation;
    *m_SentinelSelection += translation;

    auto action = [translation]()
    {
        if (translation.X < 0)
            return SelectionAction::NudgeLeft;
        else if (translation.X > 0)
            return SelectionAction::NudgeRight;
        else if (translation.Y < 0)
            return SelectionAction::NudgeUp;
        return SelectionAction::NudgeDown;
    }();

    return VersionChange
    {
        .Action = action,
        .SelectionBounds = SelectionBounds(),
        .NudgeTranslation = translation
    };
}

std::optional<gol::VersionChange> gol::SelectionManager::HandleAction(SelectionAction action, GameGrid& grid, int32_t nudgeSize)
{
    switch (action)
    {
        using enum SelectionAction;
    case Copy:       return this->Copy(grid);
    case Cut:        return this->Cut();
    case Delete:     return this->Delete();
    case Deselect:   return this->Deselect(grid);
    case NudgeLeft:  return Nudge({ -nudgeSize, 0 });
    case NudgeRight: return Nudge({ nudgeSize, 0 });
    case NudgeUp:    return Nudge({ 0, -nudgeSize });
    case NudgeDown:  return Nudge({ 0, nudgeSize });
    case Rotate:     return this->Rotate(true);
    }
    return std::nullopt;
}

void gol::SelectionManager::HandleVersionChange(EditorAction undoRedo, GameGrid& grid, const VersionChange& change)
{
    if (!change.Action)
    {
        RestoreGridVersion(undoRedo, grid, change);
        return;
    }

    if (auto* editorAction = std::get_if<EditorAction>(&*change.Action))
    {
        if (!change.GridResize)
            return;

        switch (*editorAction)
        {
        case EditorAction::Resize:
            if (undoRedo == EditorAction::Undo)
                grid = change.GridResize->first;
            else
                grid = GameGrid(grid, change.GridResize->second);
            return;
        }
    }

    auto* action = std::get_if<SelectionAction>(&*change.Action);
    if (!action)
        return;

    switch (*action)
    {
    using enum SelectionAction;
    case NudgeLeft: [[fallthrough]];
    case NudgeRight: [[fallthrough]];
    case NudgeUp: [[fallthrough]];
    case NudgeDown:
        if (undoRedo == EditorAction::Undo)
            Nudge(-change.NudgeTranslation);
        else
            Nudge(change.NudgeTranslation);
        return;
    case Rotate:
        this->Rotate(undoRedo == EditorAction::Redo);
        return;
    case Paste: [[fallthrough]];
    case Delete:
        SetSelectionBounds(*change.SelectionBounds);
        m_Selected = GameGrid(change.SelectionBounds->Size());
        RestoreGridVersion(undoRedo, *m_Selected, change);

        if ((undoRedo == EditorAction::Redo) == (*action == Delete))
            this->Deselect(grid);
        return;
    case Select:
        if (undoRedo == EditorAction::Undo)
        {
            this->Deselect(grid);
            return;
        }

        SetSelectionBounds(*change.SelectionBounds);
        m_Selected = GameGrid(change.SelectionBounds->Size());

        for (auto pos : change.CellsInserted)
            m_Selected->Set(pos.X, pos.Y, true);
        for (auto pos : change.CellsDeleted)
            grid.Set(pos.X, pos.Y, false);
        return;
    case Deselect:
        if (undoRedo == EditorAction::Undo)
        {
            SetSelectionBounds(*change.SelectionBounds);
            m_Selected = GameGrid(change.SelectionBounds->Size());
            for (auto pos : change.CellsDeleted)
                m_Selected->Set(pos.X, pos.Y, true);
            for (auto pos : change.CellsInserted)
                grid.Set(pos.X, pos.Y, false);
            return;
        }
        
        this->Deselect(grid);
        return;
    }
}

void gol::SelectionManager::RestoreGridVersion(EditorAction undoRedo, GameGrid& grid, const VersionChange& change)
{
    auto& insertions = undoRedo == EditorAction::Undo
        ? change.CellsDeleted
        : change.CellsInserted;
    for (auto& pos : insertions)
        grid.Set(pos.X, pos.Y, true);

    auto& deletions = undoRedo == EditorAction::Undo
        ? change.CellsInserted
        : change.CellsDeleted;
    for (auto& pos : deletions)
        grid.Set(pos.X, pos.Y, false);
}

gol::Rect gol::SelectionManager::SelectionBounds() const
{
    return
    {
        std::min(m_AnchorSelection->X, m_SentinelSelection->X),
        std::min(m_AnchorSelection->Y, m_SentinelSelection->Y),
        std::abs(m_SentinelSelection->X - m_AnchorSelection->X) + 1,
        std::abs(m_SentinelSelection->Y - m_AnchorSelection->Y) + 1
    };
}

bool gol::SelectionManager::GridAlive() const
{
    return m_Selected && m_Selected->Population() > 0;
}

const std::set<gol::Vec2>& gol::SelectionManager::GridData() const
{
    return m_Selected->Data();
}

int64_t gol::SelectionManager::SelectedPopulation() const
{
    return m_Selected ? m_Selected->Population() : 0;
}

bool gol::SelectionManager::CanDrawSelection() const
{
    return m_AnchorSelection && m_SentinelSelection;
}

bool gol::SelectionManager::CanDrawLargeSelection() const
{
    return m_AnchorSelection && m_SentinelSelection && (*m_AnchorSelection != *m_SentinelSelection);
}

bool gol::SelectionManager::CanDrawGrid() const
{
    return m_Selected.has_value();
}

void gol::SelectionManager::SetSelectionBounds(const Rect& bounds)
{
    m_AnchorSelection = bounds.Pos();
    m_SentinelSelection = *m_AnchorSelection + Vec2
    {
        bounds.Width - 1,
        bounds.Height - 1
    };
}

gol::Vec2 gol::SelectionManager::RotatePoint(Vec2F center, Vec2F point, bool clockwise)
{
    auto offset = Vec2F{ static_cast<float>(point.X), static_cast<float>(point.Y) } - center;
    auto rotated = clockwise
        ? Vec2F{ -offset.Y,  offset.X }
    : Vec2F{ offset.Y, -offset.X };
    auto result = rotated + center;

    auto retValue = Vec2
    {
        static_cast<int32_t>(m_RotationParity ? std::floor(result.X) : std::ceil(result.X)),
        static_cast<int32_t>(!m_RotationParity ? std::floor(result.Y) : std::ceil(result.Y))
    };
    m_RotationParity = !m_RotationParity;
    return retValue;
}