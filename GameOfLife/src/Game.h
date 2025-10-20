#ifndef __Game_h__
#define __Game_h__

#include <optional>
#include <memory>

#include "GameWindow.h"
#include "GameGrid.h"
#include "GraphicsHandler.h"
#include "GameEnums.h"

namespace gol
{
	class Game
	{
	public:
		static constexpr uint32_t DefaultWindowWidth = 1920;
		static constexpr uint32_t DefaultWindowHeight = 1080;
		static constexpr uint32_t DefaultGridWidth = 64;
		static constexpr uint32_t DefaultGridHeight = 36;
	public:
		Game();

		Game(const Game& other) = delete;
		Game(const Game&& other) = delete;
		Game& operator=(const Game& other) = delete;
		Game& operator=(const Game&& other) = delete;
	
		void Begin();
	private:
		void InitImGUI();

		void UpdateState(const UpdateInfo& info);

		std::optional<Vec2> CursorGridPos();
		void UpdateMouseState(Vec2 gridPos);

		bool SimulationUpdate(double timeElapsedMs);
		void PaintUpdate();
		void PauseUpdate();
	private:
		GameGrid m_Grid = { DefaultGridWidth, DefaultGridHeight };
		GameGrid m_InitialGrid;

		GameState m_State = GameState::Paint;
		DrawMode m_DrawMode = DrawMode::None;
		double m_TickDelayMs = 10;

		GameWindow m_Window;
		GraphicsHandler m_Graphics;
	};
}

#endif