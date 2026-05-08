#pragma once

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/datectrl.h>
#include <wx/dialog.h>
#include <wx/dynarray.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "AudioOutputDevice.h"
#include "MorseGenerator.h"
#include "User.h"

class WelcomeWindow : public wxDialog
{
    wxTextCtrl* txtBoxName{};
    wxTextCtrl* txtBoxCallsign{};
    wxComboBox* cbxSpeed{};
    wxComboBox* cbxPitch{};
    wxComboBox* cbxSignalType{};
    wxComboBox* cbxOutputDevice{};
    wxDatePickerCtrl* datePickerDateOfBirth{};
    std::vector<AudioOutputDevice> outputDevices;
    User& user;
    bool onboardingMode;
    std::unique_ptr<MorseGenerator> gen = nullptr;

public:
    WelcomeWindow(wxWindow* parent, User& u, bool onboardingMode = true, const std::string& title = "Welcome to Morse Hub", const std::string& introText = "Let's get started by setting up your profile", const std::string& confirmLabel = "Finish");

private:
    void OnClose(wxCloseEvent& event);
    void onFinish(wxEvent& event);
    void OnTest(wxEvent& event);
    void updateUser();
    std::optional<AudioOutputDevice> selectedOutputDevice() const;
};
