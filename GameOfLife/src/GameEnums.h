#ifndef __GameEnums_h__
#define __GameEnums_h__

namespace gol
{
	enum class GameState
	{
		Paint, Simulation, Paused, Empty
	};

	enum class GameAction
	{
		None, 
		Start, 
		Pause, 
		Resume, 
		Restart, 
		Reset, 
		Clear,

		Step, 
		Resize,
		
		Delete,
		Copy,
		Cut,
		Paste,

		NudgeLeft,
		NudgeRight,
		NudgeUp,
		NudgeDown,
	};

	enum class EditorMode
	{
		None, Insert, Delete, Select
	};
}

#endif