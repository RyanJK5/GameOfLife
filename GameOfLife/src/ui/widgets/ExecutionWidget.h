#ifndef __ExecutionWidget_h__
#define __ExecutionWidget_h__

#include <imgui/imgui.h>
#include <span>
#include <unordered_map>
#include <vector>

#include "GameActionButton.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"

namespace gol
{
    class StartButton : public ActionButton<GameAction, "Start", GameAction::Start, true>
    {
    public:
        StartButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x / 3.f, ActionButtonInternal::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state == GameState::Paint; }
    };

    class ClearButton : public ActionButton<GameAction, "Clear", GameAction::Clear, false>
    {
    public:
        ClearButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x / 2.f, ActionButtonInternal::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state != GameState::Empty; }
    };

    class ResetButton : public ActionButton<GameAction, "Reset", GameAction::Reset, false>
    {
    public:
        ResetButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x, ActionButtonInternal::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state == GameState::Simulation || state == GameState::Paused; }
    };

    class PauseButton : public ActionButton<GameAction, "Pause", GameAction::Pause, true>
    {
    public:
        PauseButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x / 3.f, ActionButtonInternal::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state == GameState::Simulation; }
    };

    class ResumeButton : public ActionButton<GameAction, "Resume", GameAction::Resume, false>
    {
    public:
        ResumeButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x / 2.f, ActionButtonInternal::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state == GameState::Paused; }
    };

    class RestartButton : public ActionButton<GameAction, "Restart", GameAction::Restart, false>
    {
    public:
        RestartButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const override final { return { ImGui::GetContentRegionAvail().x, ActionButtonInternal::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const override final { return state == GameState::Simulation || state == GameState::Paused; }
    };

	class ExecutionWidget
	{
	public:
        ExecutionWidget() = default;

        ExecutionWidget(const std::unordered_map<ActionVariant, std::vector<ImGuiKeyChord>>& shortcuts)
            : m_StartButton   (shortcuts.at(GameAction::Start   ))
            , m_ClearButton   (shortcuts.at(GameAction::Clear   ))
            , m_ResetButton   (shortcuts.at(GameAction::Reset   ))
            , m_PauseButton   (shortcuts.at(GameAction::Pause   ))
            , m_ResumeButton  (shortcuts.at(GameAction::Resume  ))
            , m_RestartButton (shortcuts.at(GameAction::Restart ))
        { }

        SimulationControlResult Update(GameState state);
	private:
        StartButton m_StartButton;
        ClearButton m_ClearButton;
        ResetButton m_ResetButton;
        PauseButton m_PauseButton;
        ResumeButton m_ResumeButton;
        RestartButton m_RestartButton;
	};
}

#endif