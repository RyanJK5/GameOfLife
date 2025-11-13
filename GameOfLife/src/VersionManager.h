#ifndef __VersionManager_h__
#define __VersionManager_h__

#include <vector>
#include <stack>
#include <ranges>
#include <optional>
#include <span>

#include "SimulationControlResult.h"
#include "KeyShortcut.h"
#include "GameEnums.h"
#include "Graphics2D.h"

namespace gol
{
	class VersionShortcutManager
	{
	public:
		SimulationControlResult Update(GameState state);
		
		VersionShortcutManager(std::span<const ImGuiKeyChord> undoShortcuts, std::span<const ImGuiKeyChord> redoShortcuts)
			: m_UndoShortcuts(undoShortcuts | KeyShortcut::MapChordsToVector)
			, m_RedoShortcuts(redoShortcuts | KeyShortcut::MapChordsToVector)
		{ }
	private:
		GameAction CheckShortcuts(std::span<KeyShortcut> shortcuts, GameAction targetAction);
	private:
		std::vector<KeyShortcut> m_UndoShortcuts;
		std::vector<KeyShortcut> m_RedoShortcuts;
	};

	class VersionManager
	{
	public:
		void BeginChange();
		void AddChange(Vec2 pos);

		std::optional<std::vector<Vec2>> Undo();
		std::optional<std::vector<Vec2>> Redo();
	private:
		std::stack<std::vector<Vec2>> m_UndoStack;
		std::stack<std::vector<Vec2>> m_RedoStack;
	};
}

#endif