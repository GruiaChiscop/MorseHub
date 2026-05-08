#pragma once

#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/radiobox.h>
#include <wx/spinctrl.h>

#include "User.h"

class RunnerStartDialog : public wxDialog
{
public:
    RunnerStartDialog(wxWindow* parent, User& user);

private:
    User& user;
    wxSpinCtrl* spnInitialSpeed{};
    wxRadioBox* rbxMode{};
    wxCheckBox* chkQrm{};
    wxCheckBox* chkQrn{};
    wxCheckBox* chkQsb{};
    wxCheckBox* chkFlutter{};
    wxCheckBox* chkLids{};
    wxSpinCtrl* spnCompetitionMinutes{};
    wxComboBox* cbxPitch{};
    wxComboBox* cbxSignalType{};

    void refreshModeState();
    void OnModeChanged(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
};
