#include <expected>
#include <filesystem>
#include <font-awesome/IconsFontAwesome7.h>
#include <imgui.h>
#include <span>
#include <string>

#include "ActionButton.h"
#include "FileDialog.h"
#include "FileWidget.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "PopupWindow.h"
#include "SimulationControlResult.h"

gol::SaveButton::SaveButton(std::span<const ImGuiKeyChord> shortcuts) : ActionButton(EditorAction::Save, shortcuts) { }
gol::Size2F     gol::SaveButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x / 2.f, ActionButton::DefaultButtonHeight }; }
std::string     gol::SaveButton::Label(EditorState) const { return ICON_FA_FLOPPY_DISK; }
bool            gol::SaveButton::Enabled(EditorState state) const { return state.State == SimulationState::Paint || state.State == SimulationState::Paused; }

gol::LoadButton::LoadButton(std::span<const ImGuiKeyChord> shortcuts) : ActionButton(EditorAction::Load, shortcuts) {}
gol::Size2F     gol::LoadButton::Dimensions() const { return { ImGui::GetContentRegionAvail().x, ActionButton::DefaultButtonHeight }; }
std::string     gol::LoadButton::Label(EditorState) const { return ICON_FA_FOLDER_OPEN; }
bool            gol::LoadButton::Enabled(EditorState state) const { return state.State != SimulationState::Simulation; }

gol::FileWidget::FileWidget(std::span<const ImGuiKeyChord> saveShortcuts, std::span<const ImGuiKeyChord> loadShortcuts)
	: m_SaveButton(saveShortcuts)
	, m_LoadButton(loadShortcuts)
	, m_FileNotOpened("File Not Opened")
{ }

gol::SimulationControlResult gol::FileWidget::Update(EditorState state)
{
	auto popupState = m_FileNotOpened.Update();
	if (popupState == PopupWindowState::Success)
		m_FileNotOpened.Active = false;

	auto result = m_SaveButton.Update(state);

	ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 30.f);
	auto loadResult = m_LoadButton.Update(state);
	ImGui::Separator();
	ImGui::PopStyleVar();

	if (!result)
		result = loadResult;
	if (!result)
		return { };
	
	auto filePath = [&result] -> std::expected<std::filesystem::path, FileDialogFailure>
	{
		if (*result == EditorAction::Save)
			return FileDialog::SaveFileDialog("gol", "");
		else if (*result == EditorAction::Load)
			return FileDialog::OpenFileDialog("gol", "");
		return std::unexpected { FileDialogFailure { FileFailureType::Error, "Unknown action" } };
	}();

	if (!filePath)
	{
		if (filePath.error().Type == FileFailureType::Error)
		{
			m_FileNotOpened.Active = true;
			m_FileNotOpened.Message = filePath.error().Message;
		}
		return { };
	}

	return { .Action = result, .FilePath = *filePath };
}
