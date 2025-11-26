#ifndef __FileWidget_h__
#define __FileWidget_h__

#include <imgui/imgui.h>
#include <string>
#include <span>

#include "ActionButton.h"
#include "ErrorWindow.h"
#include "GameEnums.h"
#include "Graphics2D.h"
#include "SimulationControlResult.h"

namespace gol
{
	class SaveButton : public ActionButton<EditorAction, true>
	{
	public:
		SaveButton(std::span<const ImGuiKeyChord> shortcuts);
	protected:
		virtual Size2F Dimensions() const override final;
		virtual std::string Label(EditorState state) const override final;
		virtual bool Enabled(EditorState state) const override final;
	};

	class LoadButton : public ActionButton<EditorAction, false>
	{
	public:
		LoadButton(std::span<const ImGuiKeyChord> shortcuts);
	protected:
		virtual Size2F Dimensions() const override final;
		virtual std::string Label(EditorState state) const override final;
		virtual bool Enabled(EditorState state) const override final;
	};

	class FileWidget
	{
	public:
		FileWidget(std::span<const ImGuiKeyChord> saveShortcuts, std::span<const ImGuiKeyChord> loadShortcuts);

		SimulationControlResult Update(EditorState state);
	private:
		SaveButton m_SaveButton;
		LoadButton m_LoadButton;

		ErrorWindow m_FileNotOpened;
	};
}

#endif