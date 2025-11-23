#ifndef __StepWidget_h__
#define __StepWidget_h__

#include <cstdint>
#include <imgui/imgui.h>
#include <span>

#include "ActionButton.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"

namespace gol
{
    class StepButton : public ActionButton<GameAction, "Step", GameAction::Step, false>
    {
    public:
        StepButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButtonInternal(shortcuts)
        { }
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x, ActionButtonInternal::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state == GameState::Paint || state == GameState::Paused; }
    };

	class StepWidget
	{
    public:
        static constexpr int32_t BigStep = 10;
        static constexpr int32_t StepWarning = 100;
	public:
        StepWidget(std::span<const ImGuiKeyChord> shortcuts = {})
            : m_Button(shortcuts)
        { }

		SimulationControlResult Update(GameState state);
	private:
        int32_t m_StepCount = 1;

        StepButton m_Button;
	};
}

#endif