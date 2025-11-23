#include <imgui/imgui.h>
#include <optional>

#include "ExecutionWidget.h"
#include "GameEnums.h"
#include "SimulationControlResult.h"

gol::SimulationControlResult gol::ExecutionWidget::Update(GameState state)
{
	auto result = std::optional<GameAction> {};
	const auto updateIfNone = [&result](std::optional<GameAction> update)
	{
		if (!result)
			result = update;
	};

	ImGui::Text("Execution");
	updateIfNone(m_StartButton.Update(state));
	updateIfNone(m_ClearButton.Update(state));
	updateIfNone(m_ResetButton.Update(state));
	updateIfNone(m_PauseButton.Update(state));
	updateIfNone(m_ResumeButton.Update(state));
	updateIfNone(m_RestartButton.Update(state));

	return { .Action = result };
}