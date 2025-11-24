#ifndef __SelectionManager_h__
#define __SelectionManager_h__

#include <cstdint>
#include <concepts>
#include <expected>
#include <imgui/imgui.h>
#include <optional>
#include <set>

#include "GameEnums.h"
#include "GameGrid.h"
#include "Graphics2D.h"
#include "VersionManager.h"
#include "RLEEncoder.h"

namespace gol
{
	struct SelectionUpdateResult
	{
		std::optional<VersionChange> Change;
		bool BeginSelection = false;
	};

	class SelectionManager
	{
	public:
		SelectionUpdateResult UpdateSelectionArea(GameGrid& grid, Vec2 gridPos);

		bool TryResetSelection();

		std::optional<VersionChange> Deselect(GameGrid& grid);

		std::optional<VersionChange> Copy(GameGrid& grid);

		std::optional<VersionChange> Cut();

		std::expected<VersionChange, uint32_t> Paste(std::optional<Vec2> gridPos, uint32_t warnThreshold);

		std::optional<VersionChange> Delete();

		std::optional<VersionChange> Rotate(bool clockwise);

		std::optional<VersionChange> Nudge(Vec2 translation);
		
		std::optional<VersionChange> HandleAction(SelectionAction action, GameGrid& grid, int32_t nudgeSize);
		void HandleVersionChange(EditorAction undoRedo, GameGrid& grid, const VersionChange& change);

		Rect SelectionBounds() const;

		bool GridAlive() const;
		const std::set<Vec2>& GridData() const;
		int64_t SelectedPopulation() const;

		bool CanDrawSelection() const;
		bool CanDrawLargeSelection() const;
		bool CanDrawGrid() const;
	private:
		template <std::unsigned_integral EncodingType>
		std::optional<VersionChange> Copy(GameGrid& grid, bool);

		template <std::unsigned_integral EncodingType>
		std::optional<VersionChange> Cut(bool);

		template <std::unsigned_integral EncodingType>
		std::expected<VersionChange, EncodingType> Paste(std::optional<Vec2> gridPos, EncodingType warnThreshold, bool);

		void RestoreGridVersion(EditorAction undoRedo, GameGrid& grid, const VersionChange& change);

		void SetSelectionBounds(const Rect& bounds);

		Vec2 RotatePoint(Vec2F center, Vec2F point, bool clockwise);
	private:
		bool m_RotationParity = false;

		std::optional<Vec2> m_AnchorSelection;
		std::optional<Vec2> m_SentinelSelection;
		std::optional<GameGrid> m_Selected;
	};
}

template <std::unsigned_integral EncodingType>
std::optional<gol::VersionChange> gol::SelectionManager::Copy(GameGrid& grid, bool)
{
	if (!m_Selected)
		return std::nullopt;

	ImGui::SetClipboardText(
		reinterpret_cast<const char*>(
			RLEEncoder::EncodeRegion<EncodingType>(*m_Selected, { 0, 0, m_Selected->Width(), m_Selected->Height() }).data()
			)
	);

	return Deselect(grid);
}

template <std::unsigned_integral EncodingType>
std::optional<gol::VersionChange> gol::SelectionManager::Cut(bool)
{
	ImGui::SetClipboardText(
		reinterpret_cast<const char*>(
			RLEEncoder::EncodeRegion<EncodingType>(*m_Selected, { 0, 0, m_Selected->Width(), m_Selected->Height() }).data()
			)
	);
	return Delete();
}

template <std::unsigned_integral EncodingType>
std::expected<gol::VersionChange, EncodingType> gol::SelectionManager::Paste(std::optional<Vec2> gridPos, EncodingType warnThreshold, bool)
{
	if (!gridPos)
		gridPos = m_AnchorSelection;
	if (!gridPos)
		gridPos = { 0, 0 };

	auto decodeResult = RLEEncoder::DecodeRegion<EncodingType>(ImGui::GetClipboardText(), warnThreshold);
	if (!decodeResult)
		return std::unexpected{ decodeResult.error() };

	m_Selected = *decodeResult;
	m_AnchorSelection = gridPos;
	m_SentinelSelection = { gridPos->X + m_Selected->Width() - 1, gridPos->Y + m_Selected->Height() - 1 };
	return VersionChange
	{
		.Action = SelectionAction::Paste,
		.SelectionBounds = SelectionBounds(),
		.CellsInserted = m_Selected->Data(),
		.CellsDeleted = {}
	};
}

#endif
