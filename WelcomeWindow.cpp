#include "WelcomeWindow.h"

#include "MainWindow.h"
#include "SignalGenerator.h"
#include "Utilities.h"

namespace
{
SType signalTypeFromSelection(const wxString& signal)
{
    if (signal == "Sine")
        return Sine;
    if (signal == "Square")
        return Square;
    if (signal == "Triangle")
        return Triangle;
    return Sawtooth;
}
}

WelcomeWindow::WelcomeWindow(wxWindow* parent, User& u, bool onboarding, const std::string& title, const std::string& introText, const std::string& confirmLabel)
    : wxDialog(parent, wxID_ANY, title), user(u), onboardingMode(onboarding)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* welcomeLabel = new wxStaticText(this, wxID_ANY, wxString::FromUTF8(introText));
    sizer->Add(welcomeLabel, 0, wxALL | wxCENTER, 10);

    wxStaticText* nameLabel = new wxStaticText(this, wxID_ANY, "Name");
    sizer->Add(nameLabel, 0, wxALL, 5);
    txtBoxName = new wxTextCtrl(this, wxID_ANY, user.name.c_str());
    sizer->Add(txtBoxName, 0, wxALL | wxEXPAND, 5);

    wxStaticText* callsignLabel = new wxStaticText(this, wxID_ANY, "Callsign");
    sizer->Add(callsignLabel, 0, wxALL, 5);
    txtBoxCallsign = new wxTextCtrl(this, wxID_ANY, user.callsign.c_str());
    sizer->Add(txtBoxCallsign, 0, wxALL | wxEXPAND, 5);

    wxStaticText* speedLabel = new wxStaticText(this, wxID_ANY, "Default speed (" + Utils::speedUnitLabel(user.speedDisplayMode) + ")");
    sizer->Add(speedLabel, 0, wxALL, 5);
    cbxSpeed = new wxComboBox(this, wxID_ANY);
    wxArrayString speeds;
    const int speedMin = user.speedDisplayMode == SpeedDisplayMode::Wpm ? 2 : 10;
    const int speedMax = user.speedDisplayMode == SpeedDisplayMode::Wpm ? 60 : 300;
    for (int i = speedMin; i <= speedMax; i++)
        speeds.Add(wxString::Format("%d", i));
    cbxSpeed->Append(speeds);
    cbxSpeed->SetStringSelection(wxString::Format("%d", Utils::speedToDisplayUnits(user.defaultSpeed, user.speedDisplayMode)));
    sizer->Add(cbxSpeed, 0, wxALL | wxEXPAND, 5);

    wxStaticText* pitchLabel = new wxStaticText(this, wxID_ANY, "Default pitch");
    sizer->Add(pitchLabel, 0, wxALL, 5);
    cbxPitch = new wxComboBox(this, wxID_ANY);
    wxArrayString pitches;
    for (int i = 440; i <= 1000; i++)
        pitches.Add(wxString::Format("%d", i));
    cbxPitch->Append(pitches);
    cbxPitch->SetStringSelection(wxString::Format("%d", user.defaultPitch));
    sizer->Add(cbxPitch, 0, wxALL | wxEXPAND, 5);

    wxStaticText* signalLabel = new wxStaticText(this, wxID_ANY, "Signal type");
    sizer->Add(signalLabel, 0, wxALL, 5);
    cbxSignalType = new wxComboBox(this, wxID_ANY);
    wxArrayString types;
    types.Add("Sine");
    types.Add("Square");
    types.Add("Triangle");
    types.Add("Sawtooth");
    cbxSignalType->Append(types);
    cbxSignalType->SetSelection(static_cast<int>(user.signalType));
    sizer->Add(cbxSignalType, 0, wxALL | wxEXPAND, 5);

    wxStaticText* outputDeviceLabel = new wxStaticText(this, wxID_ANY, "Audio output device");
    sizer->Add(outputDeviceLabel, 0, wxALL, 5);
    cbxOutputDevice = new wxComboBox(this, wxID_ANY);
    wxArrayString outputChoices;
    outputChoices.Add("System default");
    try
    {
        outputDevices = AudioOutputDeviceService::listPlaybackDevices();
        for (const AudioOutputDevice& device : outputDevices)
            outputChoices.Add(wxString::Format("%d: %s", device.index, wxString::FromUTF8(device.name)));
    }
    catch (...)
    {
        outputDevices.clear();
    }
    cbxOutputDevice->Append(outputChoices);
    cbxOutputDevice->SetSelection(0);
    if (user.preferredOutputDeviceIndex >= 0)
    {
        for (std::size_t i = 0; i < outputDevices.size(); ++i)
        {
            if (outputDevices[i].index == user.preferredOutputDeviceIndex)
            {
                cbxOutputDevice->SetSelection(static_cast<int>(i + 1));
                break;
            }
        }
    }
    sizer->Add(cbxOutputDevice, 0, wxALL | wxEXPAND, 5);

    wxButton* testButton = new wxButton(this, wxID_ANY, "Test");
    sizer->Add(testButton, 0, wxALL | wxCENTER, 10);

    wxStaticText* dateLabel = new wxStaticText(this, wxID_ANY, "Your date of birth");
    sizer->Add(dateLabel, 0, wxALL, 5);
    datePickerDateOfBirth = new wxDatePickerCtrl(this, wxID_ANY, user.dateOfBirth);
    sizer->Add(datePickerDateOfBirth, 0, wxALL | wxEXPAND, 5);

    wxButton* confirmButton = new wxButton(this, wxID_OK, wxString::FromUTF8(confirmLabel));
    sizer->Add(confirmButton, 0, wxALL | wxCENTER, 10);
    if (!onboardingMode)
    {
        wxButton* cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
        sizer->Add(cancelButton, 0, wxALL | wxCENTER, 10);
        cancelButton->Bind(wxEVT_BUTTON, [this](wxEvent&) { EndModal(wxCANCEL); });
    }

    this->Bind(wxEVT_CLOSE_WINDOW, &WelcomeWindow::OnClose, this);
    testButton->Bind(wxEVT_BUTTON, &WelcomeWindow::OnTest, this);
    confirmButton->Bind(wxEVT_BUTTON, &WelcomeWindow::onFinish, this);

    SetSizer(sizer);
    sizer->Fit(this);
}

void WelcomeWindow::OnClose(wxCloseEvent& event)
{
    if (onboardingMode)
    {
        wxMessageBox("you must click finish to close this window", "Can't close the dialog", wxICON_ERROR | wxOK);
        return;
    }

    EndModal(wxCANCEL);
}

void WelcomeWindow::OnTest(wxEvent& event)
{
    int speed = 20;
    cbxSpeed->GetValue().ToInt(&speed);
    speed = Utils::displayUnitsToCpm(speed, user.speedDisplayMode);
    int frequency = 440;
    cbxPitch->GetValue().ToInt(&frequency);
    const SType type = signalTypeFromSelection(cbxSignalType->GetValue());
    gen = std::make_unique<MorseGenerator>(speed, frequency, type, selectedOutputDevice());
    gen->transmitAsync("vvv");
}

void WelcomeWindow::updateUser()
{
    user.callsign = txtBoxCallsign->GetValue().ToStdString();
    user.name = txtBoxName->GetValue().ToStdString();
    cbxPitch->GetValue().ToInt(&user.defaultPitch);
    int displaySpeed = 0;
    cbxSpeed->GetValue().ToInt(&displaySpeed);
    user.defaultSpeed = Utils::displayUnitsToCpm(displaySpeed, user.speedDisplayMode);
    user.signalType = signalTypeFromSelection(cbxSignalType->GetValue());
    user.preferredOutputDeviceIndex = cbxOutputDevice->GetSelection() <= 0 ? -1 : outputDevices.at(static_cast<std::size_t>(cbxOutputDevice->GetSelection() - 1)).index;
    user.dateOfBirth = datePickerDateOfBirth->GetValue();
    if (txtBoxCallsign->GetValue().Lower() == "none")
        wxMessageBox("You chose not to add any callsign. You can add one later in settings");

    serialize(user);
    Utils::configureGlobalMorseGenerator(user);
}

void WelcomeWindow::onFinish(wxEvent& event)
{
    wxString nameValue = txtBoxName->GetValue();
    wxString callValue = txtBoxCallsign->GetValue();
    if (nameValue.IsEmpty() || callValue.IsEmpty())
    {
        wxMessageBox("Either name or callsign field is empty. If you do not have a callsign, just type none", "Name or callsign not found");
        return;
    }

    updateUser();

    if (onboardingMode)
    {
        MainFrame* f = new MainFrame(user);
        f->Show();
        wxTheApp->SetTopWindow(f);
    }

    EndModal(wxOK);
}

std::optional<AudioOutputDevice> WelcomeWindow::selectedOutputDevice() const
{
    if (cbxOutputDevice->GetSelection() <= 0)
        return std::nullopt;

    return outputDevices.at(static_cast<std::size_t>(cbxOutputDevice->GetSelection() - 1));
}
