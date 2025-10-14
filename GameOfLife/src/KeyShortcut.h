#ifndef __KeyShortcut_h__
#define __KeyShortcut_h__

#include <unordered_map>

#include "vendor/imgui.h"

template <ImGuiKey ... Keys>
class KeyShortcut
{
public:
	bool Update()
	{
		bool keyCombo = KeyDown(Keys...);
		bool result = m_Down && !keyCombo;

		if (keyCombo)
			m_Down = true;
		else if (m_Down)
			m_Down = false;

		return result;
	}
private:
	template <typename... Args>
	static bool KeyDown(Args&&... keys)
	{
		return (... && ImGui::IsKeyDown(keys));
	}

	bool m_Down = false;
};
#endif