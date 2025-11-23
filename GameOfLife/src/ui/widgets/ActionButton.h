#ifndef __GameActionButton_h__
#define __GameActionButton_h__

#include <cstdint>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <optional>
#include <string_view>
#include <span>
#include <string>
#include <vector>

#include "GameEnums.h"
#include "Graphics2D.h"
#include "KeyShortcut.h"

namespace gol
{
	template <ActionType ActType, auto Label, ActType Action, bool LineBreak>
	class ActionButtonInternal
	{
	public:
		static constexpr int32_t DefaultButtonHeight = 50;
	
		ActionButtonInternal(std::span<const ImGuiKeyChord> shortcuts)
			: m_Shortcuts(shortcuts | KeyShortcut::MapChordsToVector)
		{ }

		virtual std::optional<ActType> Update(GameState state)
		{
			if (!Enabled(state))
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (!m_LineBreak)
				ImGui::SameLine();

			std::optional<ActType> result = [this, state]()
				{
					bool active = false;
					for (auto& shortcut : m_Shortcuts)
						active = shortcut.Active() || active;

					if (ImGui::Button(m_Label.c_str(), Dimensions()) || (Enabled(state) && active))
						return std::optional<ActType> { m_Return };
					return std::optional<ActType> {};
				}();

			if (!Enabled(state))
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}

			return result;
		}
	protected:
		virtual Size2F Dimensions() const = 0;
		virtual	bool Enabled(GameState state) const = 0;
	private:
		std::string m_Label = Label;
		ActType m_Return = Action;
		bool m_LineBreak = LineBreak;

		std::vector<KeyShortcut> m_Shortcuts;
	};

    template <size_t Length>
    struct StringLiteral
    {
        constexpr StringLiteral(const char (&str)[Length])
        {
            std::copy_n(str, Length, value);
        }

        char value[Length];
    };

    template <ActionType ActType, StringLiteral Label, ActType Action, bool LineBreak>
    using ActionButton = ActionButtonInternal<ActType, Label.value, Action, LineBreak>;
}

#endif