#pragma once

#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "User.h"

class ReceptionSettingsDialog : public wxDialog
{
public:
    ReceptionSettingsDialog(wxWindow* parent, User& user);

private:
    User& user;
    ReceptionSettings lastSessionSettings;
    ReceptionSettings defaultSettings;
    wxCheckBox* chkUseLastSession{};
    wxTextCtrl* txtSpeeds{};
    wxCheckBox* chkInternalInput{};
    wxTextCtrl* txtSeparator{};
    wxSpinCtrl* spnResultEntryMinutes{};
    wxSpinCtrl* spnTransmissionSeconds{};
    wxSpinCtrl* spnPauseSeconds{};
    wxTextCtrl* txtAllowedSymbols{};
    wxComboBox* cbxPitch{};
    wxComboBox* cbxSignalType{};
    wxStaticText* lblHelp{};

    void loadSettingsIntoControls(const ReceptionSettings& settings);
    void OnUseLastSessionChanged(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
};
