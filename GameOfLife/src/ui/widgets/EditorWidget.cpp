#include <optional>
#include <imgui/imgui.h>

#include "EditorWidget.h"
#include "GameEnums.h"
#include "SimulationControlResult.h"

gol::SimulationControlResult gol::EditorWidget::Update(GameState state)
{
	auto result = std::optional<SelectionAction> {};
	const auto updateIfNone = [&result](std::optional<SelectionAction> update)
	{
		if (!result)
			result = update;
	};
	
	updateIfNone(m_CopyButton.Update(state));
	updateIfNone(m_CutButton.Update(state));
	if (result)
		m_PasteButton.ClipboardCopied = true;
	updateIfNone(m_PasteButton.Update(state));
	updateIfNone(m_DeleteButton.Update(state));
	
	return { .Action = result };
}
