#include "VersionManager.h"

gol::SimulationControlResult gol::VersionShortcutManager::Update(GameState state)
{
	if (state != GameState::Paint && state != GameState::Empty)
		return { };
	auto result = CheckShortcuts(m_UndoShortcuts, GameAction::Undo);
	auto redoShortcuts = CheckShortcuts(m_RedoShortcuts, GameAction::Redo);
	if (result == GameAction::None)
		result = redoShortcuts;
	return { .Action = result };
}

void gol::VersionManager::BeginChange()
{
	m_UndoStack.push({});
	while (!m_RedoStack.empty())
		m_RedoStack.pop();
}

void gol::VersionManager::AddChange(Vec2 pos)
{
	if (m_UndoStack.empty())
		return;

	m_UndoStack.top().push_back(pos);
}

std::optional<std::vector<gol::Vec2>> gol::VersionManager::Undo()
{
	if (m_UndoStack.empty())
		return std::nullopt;

	std::vector<Vec2> state = std::move(m_UndoStack.top());
	m_UndoStack.pop();
	m_RedoStack.push(state);

	return state;
}

std::optional<std::vector<gol::Vec2>> gol::VersionManager::Redo()
{
	if (m_RedoStack.empty())
		return std::nullopt;

	std::vector<Vec2> state = std::move(m_RedoStack.top());
	m_RedoStack.pop();
	m_UndoStack.push(state);
	return state;
}

gol::GameAction gol::VersionShortcutManager::CheckShortcuts(std::span<KeyShortcut> shortcuts, GameAction targetAction)
{
	auto result = GameAction::None;
	for (auto&& shortcut : shortcuts)
	{
		auto active = shortcut.Active();
		if (active && result == GameAction::None)
			result = targetAction;
	}
	return result;
}