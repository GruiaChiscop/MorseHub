#include "RUFZStartDialog.h"

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

RUFZStartDialog::RUFZStartDialog(wxWindow* parent, User& u) : wxDialog(parent, wxID_ANY, "RufzXP mode details"), user(u), useExistent(true)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* welcomeLabel = new wxStaticText(this, wxID_ANY, "Here you select the settings for RufzXP mode");
    sizer->Add(welcomeLabel, 0, wxALL | wxCENTER, 10);

    useExistentCheckBox = new wxCheckBox(this, wxID_ANY, "Use the existent settings");
    useExistentCheckBox->SetValue(useExistent);
    sizer->Add(useExistentCheckBox, 0, wxALL, 5);

    wxStaticText* speedLabel = new wxStaticText(this, wxID_ANY, "Default speed (" + Utils::speedUnitLabel(user.speedDisplayMode) + ")");
    sizer->Add(speedLabel, 0, wxALL, 5);
    cbxSpeed = new wxComboBox(this, wxID_ANY);
    wxArrayString speeds;
    const int speedMin = user.speedDisplayMode == SpeedDisplayMode::Wpm ? 2 : 10;
    const int speedMax = user.speedDisplayMode == SpeedDisplayMode::Wpm ? 200 : 1000;
    for (int i = speedMin; i < speedMax; i++)
        speeds.Add(wxString::Format("%d", i));
    cbxSpeed->Append(speeds);
    cbxSpeed->SetStringSelection(wxString::Format("%d", Utils::speedToDisplayUnits(user.defaultSpeed, user.speedDisplayMode)));
    sizer->Add(cbxSpeed, 0, wxALL | wxEXPAND, 5);
    cbxSpeed->Disable();

    wxStaticText* pitchLabel = new wxStaticText(this, wxID_ANY, "Default pitch");
    sizer->Add(pitchLabel, 0, wxALL, 5);
    cbxPitch = new wxComboBox(this, wxID_ANY);
    wxArrayString pitches;
    for (int i = 440; i <= 1000; i++)
        pitches.Add(wxString::Format("%d", i));
    cbxPitch->Append(pitches);
    cbxPitch->SetStringSelection(wxString::Format("%d", user.defaultPitch));
    sizer->Add(cbxPitch, 0, wxALL | wxEXPAND, 5);
    cbxPitch->Disable();

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
    cbxSignalType->Disable();

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
    cbxOutputDevice->Disable();

    wxButton* testButton = new wxButton(this, wxID_ANY, "&Test");
    sizer->Add(testButton, 0, wxALL | wxCENTER, 10);
    wxButton* cancelBTN = new wxButton(this, wxID_CANCEL, "&Cancel");
    sizer->Add(cancelBTN, 0, wxALL | wxCENTER, 10);
    wxButton* okBTN = new wxButton(this, wxID_OK, "&OK");
    sizer->Add(okBTN, 0, wxALL | wxCENTER, 10);
    testButton->Bind(wxEVT_BUTTON, &RUFZStartDialog::OnTest, this);
    useExistentCheckBox->Bind(wxEVT_CHECKBOX, &RUFZStartDialog::OnCheckBox, this);
    okBTN->Bind(wxEVT_BUTTON, &RUFZStartDialog::OnOK, this);
    cancelBTN->Bind(wxEVT_BUTTON, &RUFZStartDialog::OnCancel, this);
    SetSizer(sizer);
    sizer->Fit(this);
}

void RUFZStartDialog::OnTest(wxEvent& event)
{
    int speed = user.defaultSpeed;
    int pitch = user.defaultPitch;
    SType signalType = user.signalType;

    if (!useExistentCheckBox->IsChecked())
    {
        int displaySpeed = 0;
        cbxSpeed->GetValue().ToInt(&displaySpeed);
        speed = Utils::displayUnitsToCpm(displaySpeed, user.speedDisplayMode);
        cbxPitch->GetValue().ToInt(&pitch);
        signalType = signalTypeFromSelection(cbxSignalType->GetValue());
    }

    gen = std::make_unique<MorseGenerator>(speed, pitch, signalType, selectedOutputDevice());
    gen->transmitAsync("vvv= = +");
}

void RUFZStartDialog::OnCheckBox(wxEvent& event)
{
    const bool useExistingSettings = useExistentCheckBox->IsChecked();
    cbxSpeed->Enable(!useExistingSettings);
    cbxPitch->Enable(!useExistingSettings);
    cbxSignalType->Enable(!useExistingSettings);
    cbxOutputDevice->Enable(!useExistingSettings);
}

void RUFZStartDialog::OnOK(wxEvent& event)
{
    int displaySpeed = 0;
    cbxSpeed->GetValue().ToInt(&displaySpeed);
    user.defaultSpeed = Utils::displayUnitsToCpm(displaySpeed, user.speedDisplayMode);
    cbxPitch->GetValue().ToInt(&user.defaultPitch);
    user.signalType = signalTypeFromSelection(cbxSignalType->GetValue());
    user.preferredOutputDeviceIndex = cbxOutputDevice->GetSelection() <= 0 ? -1 : outputDevices.at(static_cast<std::size_t>(cbxOutputDevice->GetSelection() - 1)).index;
    EndModal(wxOK);
}

void RUFZStartDialog::OnCancel(wxEvent& event)
{
    EndModal(wxCANCEL);
}

std::optional<AudioOutputDevice> RUFZStartDialog::selectedOutputDevice() const
{
    if (cbxOutputDevice->GetSelection() <= 0)
        return std::nullopt;

    return outputDevices.at(static_cast<std::size_t>(cbxOutputDevice->GetSelection() - 1));
}
