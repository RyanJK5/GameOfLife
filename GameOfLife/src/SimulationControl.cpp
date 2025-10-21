#include "SimulationControl.h"
#include "GUILoader.h"

gol::SimulationControl::SimulationControl(const std::filesystem::path& shortcutsPath)
{
    auto fileInfo = StyleLoader::LoadYAML<ImVec4>(shortcutsPath);
    if (!fileInfo)
        throw std::exception(fileInfo.error().Description.data());
    CreateButtons(fileInfo->Shortcuts);
}

gol::GameAction gol::SimulationControl::Update(GameState state)
{
    ImGui::Begin("Simulation Control");

    GameAction result = GameAction::None;
    for (auto& button : m_Buttons)
    {
        GameAction action = button.Update(state);
        if (result == GameAction::None)
            result = action;
    }

    ImGui::End();
    return result;
}

void gol::SimulationControl::CreateButtons(const std::unordered_map<GameAction, std::vector<ImGuiKeyChord>>& shortcuts)
{
    Size2F topRow = { 100, 50 };
    Size2F bottomRow = { 155, 50 };
    m_Buttons[0] = {
        "Start",
        GameAction::Start,
        topRow,
        [](auto state) { return state == GameState::Paint; },
        shortcuts.at(GameAction::Start),
        true
    };
    m_Buttons[1] = {
        "Clear",
        GameAction::Clear,
        topRow,
        [](auto state) { return state != GameState::Empty; },
        shortcuts.at(GameAction::Clear)
    };
    m_Buttons[2] = {
        "Reset",
        GameAction::Reset,
        topRow,
        [](auto state) { return state == GameState::Simulation || state == GameState::Paused; },
        shortcuts.at(GameAction::Reset)
    };
    m_Buttons[3] = {
        "Pause",
        GameAction::Pause,
        topRow,
        [](auto state) { return state == GameState::Simulation; },
        shortcuts.at(GameAction::Pause),
        true
    };
    m_Buttons[4] = {
        "Resume",
        GameAction::Resume,
        topRow,
        [](auto state) { return state == GameState::Paused; },
        shortcuts.at(GameAction::Resume)
    };
    m_Buttons[5] = {
        "Restart",
        GameAction::Restart,
        topRow,
        [](auto state) { return state == GameState::Simulation || state == GameState::Paused; },
        shortcuts.at(GameAction::Restart)
    };
}
