#pragma once

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "StringGen.h"
#include "User.h"

class TrainerSettingsDialog : public wxDialog
{
    User& user;
    wxCheckBox* chkFixedSpeed{};
    wxComboBox* cbxSpeed{};
    wxComboBox* cbxAdaptiveStep{};
    wxComboBox* cbxMinSpeed{};
    wxComboBox* cbxGroupLength{};
    wxRadioBox* rbxCharsetPreset{};
    wxCheckListBox* lstSymbols{};

public:
    TrainerSettingsDialog(wxWindow* parent, User& user);

private:
    void applyPresetSelection(int selection);
    void OnFixedSpeedChanged(wxCommandEvent& event);
    void OnPresetChanged(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void refreshEnabledState();
};
