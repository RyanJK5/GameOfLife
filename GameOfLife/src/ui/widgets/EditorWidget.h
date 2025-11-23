#ifndef __EditorWidget_h__
#define __EditorWidget_h__

#include <imgui/imgui.h>
#include <span>
#include <unordered_map>
#include <vector>

#include "ActionButton.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"


namespace gol
{
    class CopyButton : public ActionButton<SelectionAction, "Copy", SelectionAction::Copy, true>
    {
    public:
        CopyButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 4.f, ActionButtonInternal::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return state == GameState::Paint || state == GameState::Empty || state == GameState::Paused; }
    };

    class CutButton : public ActionButton<SelectionAction, "Cut", SelectionAction::Cut, false>
    {
    public:
        CutButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 3.f, ActionButtonInternal::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return state == GameState::Paint || state == GameState::Empty; }
    };

    class PasteButton : public ActionButton<SelectionAction, "Paste", SelectionAction::Paste, false>
    {
    public:
        PasteButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButtonInternal(shortcuts)
        {}
    public:
        bool ClipboardCopied = false;
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x / 2.f, ActionButtonInternal::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return ClipboardCopied && (state == GameState::Paint || state == GameState::Empty); }
    };

    class DeleteButton : public ActionButton<SelectionAction, "Delete", SelectionAction::Delete, false>
    {
    public:
        DeleteButton(std::span<const ImGuiKeyChord> shortcuts = {})
            : ActionButtonInternal(shortcuts)
        {}
    protected:
        virtual Size2F Dimensions() const final { return { ImGui::GetContentRegionAvail().x, ActionButtonInternal::DefaultButtonHeight }; }
        virtual bool Enabled(GameState state) const final { return state == GameState::Paint || state == GameState::Empty; }
    };

    class EditorWidget
    {
    public:
        EditorWidget(const std::unordered_map<ActionVariant, std::vector<ImGuiKeyChord>>& shortcuts = {})
            : m_CopyButton  (shortcuts.at(SelectionAction::Copy  ))
            , m_CutButton   (shortcuts.at(SelectionAction::Cut   ))
            , m_PasteButton (shortcuts.at(SelectionAction::Paste ))
            , m_DeleteButton(shortcuts.at(SelectionAction::Delete))
        {}

        SimulationControlResult Update(GameState state);
    private:
        CopyButton m_CopyButton;
        CutButton m_CutButton;
        PasteButton m_PasteButton;
        DeleteButton m_DeleteButton;
    };
}

#endif