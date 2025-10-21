#ifndef __SimulationControl_h__
#define __SimulationControl_h__

#include <array>
#include <filesystem>

#include "GameEnums.h"
#include "GameActionButton.h"

namespace gol
{
	class SimulationControl
	{
	public:
		SimulationControl(const std::filesystem::path& shortcuts);

		GameAction Update(GameState state);
	private:
		void CreateButtons(const std::unordered_map<GameAction, std::vector<ImGuiKeyChord>>& shortcuts);
	private:
		static constexpr int8_t ButtonCount = 6;
	private:
		std::array<GameActionButton, ButtonCount> m_Buttons;
	};
}

#endif