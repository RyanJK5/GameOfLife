#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "KeyShortcut.h"
#include <string>
#include <span>

std::string gol::KeyShortcut::StringRepresentation(std::span<const KeyShortcut> shortcuts)
{
	auto tooltip = std::string {};
	if (!shortcuts.empty())
	{
		for (size_t i = 0; i < shortcuts.size(); ++i)
		{
			tooltip += ImGui::GetKeyChordName(shortcuts[i].Shortcut());
			if (i < shortcuts.size() - 1)
				tooltip += ", ";
		}
	}
	return tooltip;
}

bool gol::KeyShortcut::Active()
{
	bool keyCombo = ImGui::IsKeyChordPressed(m_Shortcut);
	bool result = (!m_OnRelease && keyCombo) || (m_Down && !keyCombo);

	if (keyCombo)
		m_Down = true;
	else if (m_Down)
		m_Down = false;

	return result;
}