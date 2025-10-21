#ifndef __GameActionButton_h__
#define __GameActionButton_h__

#include <string_view>
#include <vector>
#include <functional>

#include "Graphics2D.h"
#include "GameEnums.h"
#include "KeyShortcut.h"

namespace gol
{
	class GameActionButton
	{
	public:
		GameActionButton() = default;

		GameActionButton(
			std::string_view label, 
			GameAction actionReturn,
			Size2F size,
			const std::function<bool(GameState)>& enabledCheck,
			const std::vector<ImGuiKeyChord>& shortcuts,
			bool lineBreak = false
		);

		GameAction Update(GameState state);
	private:
		std::string_view m_Label;
		GameAction m_Return;
		Size2F m_Size;

		std::function<bool(GameState)> m_Enabled;

		std::vector<KeyShortcut> m_Shortcuts;
		bool m_LineBreak;
	};
}

#endif