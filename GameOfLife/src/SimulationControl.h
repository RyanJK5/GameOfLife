#ifndef __SimulationControl_h__
#define __SimulationControl_h__

#include <memory>
#include <vector>

#include <ranges>

#include "GameEnums.h"
#include "EditorWidget.h"
#include "GUILoader.h"
#include "StepWidget.h"
#include "ResizeWidget.h"
#include "ExecutionWidget.h"
#include "DelayWidget.h"

namespace gol
{
	struct SelectionShortcuts
	{
		std::unordered_map<GameAction, std::vector<KeyShortcut>> Shortcuts;

		SelectionShortcuts(
			const std::vector<ImGuiKeyChord>& left, 
			const std::vector<ImGuiKeyChord>& right, 
			const std::vector<ImGuiKeyChord>& up, 
			const std::vector<ImGuiKeyChord>& down,
			const std::vector<ImGuiKeyChord>& deselect
		)
			: Shortcuts({
				{ GameAction::NudgeLeft,  left      | MapChordsToShortcuts },
				{ GameAction::NudgeRight, right     | MapChordsToShortcuts },
				{ GameAction::NudgeUp,    up        | MapChordsToShortcuts },
				{ GameAction::NudgeDown,  down      | MapChordsToShortcuts },
				{ GameAction::Deselect,   deselect  | MapChordsToShortcuts }
			})
		{ }

		SimulationControlResult Update(GameState state);

	private:
		constexpr static auto MapChordsToShortcuts =
			  std::views::transform([](auto chord) { return KeyShortcut(chord); })
			| std::ranges::to<std::vector>();
	};

	class SimulationControl
	{
	public:
		SimulationControl(const StyleLoader::StyleInfo<ImVec4>& fileInfo);

		SimulationControlResult Update(GameState state);
	private:
		void FillResults(SimulationControlResult& current, const SimulationControlResult& update) const;
	private:
		static constexpr int32_t BigStep = 100;
		static constexpr int32_t StepWarning = 100;
	private:
		SelectionShortcuts m_SelectionShortcuts;

		ExecutionWidget m_ExecutionWidget;
		ResizeWidget m_ResizeWidget;
		StepWidget m_StepWidget;
		DelayWidget m_DelayWidget;
		EditorWidget m_EditorWidget;
	};
}

#endif