#ifndef __GameEnums_h__
#define __GameEnums_h__

#include <concepts>
#include <variant>

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
		Step
	};

	enum class EditorAction
	{
		Resize,
		Undo,
		Redo
	};

	enum class SelectionAction 
	{
		Rotate,
		Select,
		Deselect,
		Delete,
		Copy,
		Cut,
		Paste,

		NudgeLeft,
		NudgeRight,
		NudgeUp,
		NudgeDown
	};

	using ActionVariant = std::variant<GameAction, EditorAction, SelectionAction>;

	template <typename T>
	concept ActionType = 
		std::same_as<T, GameAction>   || 
		std::same_as<T, EditorAction> || 
		std::same_as<T, SelectionAction>;

	enum class EditorMode
	{
		None, 
		Insert, 
		Delete, 
		Select
	};
}

#endif