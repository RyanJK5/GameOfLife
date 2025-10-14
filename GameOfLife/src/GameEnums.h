#ifndef __GameEnums_h__
#define __GameEnums_h__

namespace gol
{
	enum class GameState
	{
		Paint, Simulation, Paused
	};

	enum class GameAction
	{
		None, Start, Pause, Resume, Restart, Reset, Clear
	};

	enum class DrawMode
	{
		None, Insert, Delete
	};
}

#endif