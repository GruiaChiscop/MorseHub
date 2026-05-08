#pragma once

#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/combobox.h>
#include <wx/datectrl.h>
#include <wx/dialog.h>
#include <wx/notebook.h>
#include <wx/radiobox.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "AudioOutputDevice.h"
#include "MorseGenerator.h"
#include "User.h"

class MorseHubSettingsDialog : public wxDialog
{
public:
    MorseHubSettingsDialog(wxWindow* parent, User& user, int initialTab = 0);

private:
    User& user;
    std::vector<AudioOutputDevice> outputDevices;
    std::unique_ptr<MorseGenerator> generator = nullptr;

    wxNotebook* notebook{};
    wxTextCtrl* txtName{};
    wxTextCtrl* txtCallsign{};
    wxDatePickerCtrl* datePickerDateOfBirth{};
    wxComboBox* cbxSpeed{};
    wxComboBox* cbxSpeedDisplayMode{};
    wxComboBox* cbxPitch{};
    wxComboBox* cbxSignalType{};
    wxComboBox* cbxOutputDevice{};
    wxSlider* sldVolume{};

    wxSpinCtrl* spnRufzMaxRounds{};
    wxCheckBox* chkRufzAllowRepeat{};
    wxSpinCtrl* spnRufzSpeedStep{};
    wxSpinCtrl* spnRufzMinSpeed{};

    wxCheckBox* chkTrainerFixedSpeed{};
    wxSpinCtrl* spnTrainerSpeed{};
    wxSpinCtrl* spnTrainerAdaptiveStep{};
    wxSpinCtrl* spnTrainerMinSpeed{};
    wxSpinCtrl* spnTrainerGroupLength{};
    wxRadioBox* rbxTrainerCharsetPreset{};
    wxCheckListBox* lstTrainerSymbols{};

    wxTextCtrl* txtReceptionSpeeds{};
    wxCheckBox* chkReceptionInternalInput{};
    wxTextCtrl* txtReceptionSeparator{};
    wxTextCtrl* txtReceptionAllowedSymbols{};
    wxSpinCtrl* spnReceptionResultEntryMinutes{};
    wxSpinCtrl* spnReceptionTransmissionSeconds{};
    wxSpinCtrl* spnReceptionPauseSeconds{};
    wxSpinCtrl* spnReceptionGroupLength{};

    wxSpinCtrl* spnRunnerInitialSpeed{};
    wxRadioBox* rbxRunnerMode{};
    wxSpinCtrl* spnRunnerCompetitionMinutes{};
    wxCheckBox* chkRunnerQrm{};
    wxCheckBox* chkRunnerQrn{};
    wxCheckBox* chkRunnerQsb{};
    wxCheckBox* chkRunnerFlutter{};
    wxCheckBox* chkRunnerLids{};

    wxPanel* createGeneralPage();
    wxPanel* createRufzPage();
    wxPanel* createTrainerPage();
    wxPanel* createReceptionPage();
    wxPanel* createRunnerPage();

    void OnSave(wxCommandEvent& event);
    void OnTestAudio(wxCommandEvent& event);
    void OnTrainerFixedSpeedChanged(wxCommandEvent& event);
    void OnTrainerPresetChanged(wxCommandEvent& event);
    void OnRunnerModeChanged(wxCommandEvent& event);
    void refreshTrainerState();
    void refreshRunnerState();
    void applyTrainerPresetSelection(int selection);
    std::optional<AudioOutputDevice> selectedOutputDevice() const;
};
