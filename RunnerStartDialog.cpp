#include "RunnerStartDialog.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

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

RunnerStartDialog::RunnerStartDialog(wxWindow* parent, User& value)
    : wxDialog(parent, wxID_ANY, "Morse Runner start"), user(value)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(new wxStaticText(this, wxID_ANY, "Configure the first Morse Runner session"), 0, wxALL | wxCENTER, 10);

    wxArrayString modeChoices;
    modeChoices.Add("Pile-Up");
    modeChoices.Add("Single Calls");
    modeChoices.Add("WPX Competition");
    rbxMode = new wxRadioBox(this, wxID_ANY, "Session mode", wxDefaultPosition, wxDefaultSize, modeChoices, 1, wxRA_SPECIFY_COLS);
    switch (user.moduleSettings.runner.mode)
    {
    case RunnerMode::SingleCalls:
        rbxMode->SetSelection(1);
        break;
    case RunnerMode::WpxCompetition:
        rbxMode->SetSelection(2);
        break;
    case RunnerMode::Pileup:
    default:
        rbxMode->SetSelection(0);
        break;
    }
    sizer->Add(rbxMode, 0, wxALL | wxEXPAND, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Initial speed (" + Utils::speedUnitLabel(user.speedDisplayMode) + ")"), 0, wxALL, 5);
    spnInitialSpeed = new wxSpinCtrl(
        this,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS,
        1,
        user.speedDisplayMode == SpeedDisplayMode::Wpm ? 200 : 1000,
        Utils::speedToDisplayUnits(user.moduleSettings.runner.initialSpeed, user.speedDisplayMode));
    sizer->Add(spnInitialSpeed, 0, wxALL | wxEXPAND, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Competition duration (minutes)"), 0, wxALL, 5);
    spnCompetitionMinutes = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 120, user.moduleSettings.runner.competitionMinutes);
    sizer->Add(spnCompetitionMinutes, 0, wxALL | wxEXPAND, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Pitch"), 0, wxALL, 5);
    cbxPitch = new wxComboBox(this, wxID_ANY);
    wxArrayString pitches;
    for (int i = 300; i <= 1200; i += 10)
        pitches.Add(wxString::Format("%d", i));
    cbxPitch->Append(pitches);
    cbxPitch->SetStringSelection(wxString::Format("%d", user.defaultPitch));
    sizer->Add(cbxPitch, 0, wxALL | wxEXPAND, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Signal type"), 0, wxALL, 5);
    cbxSignalType = new wxComboBox(this, wxID_ANY);
    cbxSignalType->Append("Sine");
    cbxSignalType->Append("Square");
    cbxSignalType->Append("Triangle");
    cbxSignalType->Append("Sawtooth");
    cbxSignalType->SetSelection(static_cast<int>(user.signalType));
    sizer->Add(cbxSignalType, 0, wxALL | wxEXPAND, 5);

    chkQrm = new wxCheckBox(this, wxID_ANY, "Enable QRM");
    chkQrm->SetValue(user.moduleSettings.runner.enableQrm);
    sizer->Add(chkQrm, 0, wxALL, 5);

    chkQrn = new wxCheckBox(this, wxID_ANY, "Enable QRN");
    chkQrn->SetValue(user.moduleSettings.runner.enableQrn);
    sizer->Add(chkQrn, 0, wxALL, 5);

    chkQsb = new wxCheckBox(this, wxID_ANY, "Enable QSB");
    chkQsb->SetValue(user.moduleSettings.runner.enableQsb);
    sizer->Add(chkQsb, 0, wxALL, 5);

    chkFlutter = new wxCheckBox(this, wxID_ANY, "Enable Flutter");
    chkFlutter->SetValue(user.moduleSettings.runner.enableFlutter);
    sizer->Add(chkFlutter, 0, wxALL, 5);

    chkLids = new wxCheckBox(this, wxID_ANY, "Enable LIDS");
    chkLids->SetValue(user.moduleSettings.runner.enableLids);
    sizer->Add(chkLids, 0, wxALL, 5);

    wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    auto* saveButton = new wxButton(this, wxID_OK, "Start");
    auto* cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
    buttons->Add(saveButton, 0, wxALL, 5);
    buttons->Add(cancelButton, 0, wxALL, 5);
    sizer->Add(buttons, 0, wxALIGN_CENTER);

    saveButton->Bind(wxEVT_BUTTON, &RunnerStartDialog::OnSave, this);
    cancelButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { EndModal(wxCANCEL); });
    rbxMode->Bind(wxEVT_RADIOBOX, &RunnerStartDialog::OnModeChanged, this);

    refreshModeState();
    SetSizerAndFit(sizer);
}

void RunnerStartDialog::refreshModeState()
{
    const bool isWpx = rbxMode->GetSelection() == 2;
    spnCompetitionMinutes->Enable(isWpx);

    if (isWpx)
    {
        chkQrm->SetValue(true);
        chkQrn->SetValue(true);
        chkQsb->SetValue(true);
        chkFlutter->SetValue(true);
        chkLids->SetValue(true);
    }

    chkQrm->Enable(!isWpx);
    chkQrn->Enable(!isWpx);
    chkQsb->Enable(!isWpx);
    chkFlutter->Enable(!isWpx);
    chkLids->Enable(!isWpx);
}

void RunnerStartDialog::OnModeChanged(wxCommandEvent& event)
{
    (void)event;
    refreshModeState();
}

void RunnerStartDialog::OnSave(wxCommandEvent& event)
{
    (void)event;
    user.moduleSettings.runner.initialSpeed = Utils::displayUnitsToCpm(spnInitialSpeed->GetValue(), user.speedDisplayMode);
    user.moduleSettings.runner.competitionMinutes = spnCompetitionMinutes->GetValue();
    switch (rbxMode->GetSelection())
    {
    case 1:
        user.moduleSettings.runner.mode = RunnerMode::SingleCalls;
        user.moduleSettings.runner.enableQrm = chkQrm->GetValue();
        user.moduleSettings.runner.enableQrn = chkQrn->GetValue();
        user.moduleSettings.runner.enableQsb = chkQsb->GetValue();
        user.moduleSettings.runner.enableFlutter = chkFlutter->GetValue();
        user.moduleSettings.runner.enableLids = chkLids->GetValue();
        break;
    case 2:
        user.moduleSettings.runner.mode = RunnerMode::WpxCompetition;
        user.moduleSettings.runner.enableQrm = true;
        user.moduleSettings.runner.enableQrn = true;
        user.moduleSettings.runner.enableQsb = true;
        user.moduleSettings.runner.enableFlutter = true;
        user.moduleSettings.runner.enableLids = true;
        break;
    case 0:
    default:
        user.moduleSettings.runner.mode = RunnerMode::Pileup;
        user.moduleSettings.runner.enableQrm = chkQrm->GetValue();
        user.moduleSettings.runner.enableQrn = chkQrn->GetValue();
        user.moduleSettings.runner.enableQsb = chkQsb->GetValue();
        user.moduleSettings.runner.enableFlutter = chkFlutter->GetValue();
        user.moduleSettings.runner.enableLids = chkLids->GetValue();
        break;
    }
    cbxPitch->GetValue().ToInt(&user.defaultPitch);
    user.signalType = signalTypeFromSelection(cbxSignalType->GetValue());
    EndModal(wxOK);
}
