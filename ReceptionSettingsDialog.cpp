#include "ReceptionSettingsDialog.h"

#include <sstream>

#include <wx/button.h>
#include <wx/msgdlg.h>

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

std::string separatorForDisplay(const std::string& separator)
{
    return separator.empty() ? ";" : separator;
}

std::string speedsToString(const std::vector<int>& speeds, SpeedDisplayMode mode)
{
    std::ostringstream stream;
    for (std::size_t i = 0; i < speeds.size(); ++i)
    {
        if (i != 0)
            stream << ", ";
        stream << Utils::speedToDisplayUnits(speeds[i], mode);
    }
    return stream.str();
}

std::vector<int> parseSpeeds(const wxString& value, SpeedDisplayMode mode)
{
    std::vector<int> speeds;
    wxString token;
    for (wxUniChar c : value)
    {
        if (c == ',' || c == ';' || c == ' ' || c == '\n' || c == '\t')
        {
            if (!token.empty())
            {
                long parsed = 0;
                if (!token.ToLong(&parsed) || parsed <= 0)
                    return {};
                speeds.push_back(Utils::displayUnitsToCpm(static_cast<int>(parsed), mode));
                token.clear();
            }
        }
        else
        {
            token += c;
        }
    }

    if (!token.empty())
    {
        long parsed = 0;
        if (!token.ToLong(&parsed) || parsed <= 0)
            return {};
        speeds.push_back(Utils::displayUnitsToCpm(static_cast<int>(parsed), mode));
    }

    return speeds;
}
}

ReceptionSettingsDialog::ReceptionSettingsDialog(wxWindow* parent, User& value)
    : wxDialog(parent, wxID_ANY, "Reception mode settings"), user(value), lastSessionSettings(value.moduleSettings.reception), defaultSettings()
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(new wxStaticText(this, wxID_ANY, "Configure Zeus-style reception mode"), 0, wxALL | wxCENTER, 10);

    chkUseLastSession = new wxCheckBox(this, wxID_ANY, "Apply settings from the last reception session");
    chkUseLastSession->SetValue(true);
    sizer->Add(chkUseLastSession, 0, wxALL, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, wxString::Format("Speeds (%s, up to 10 values separated by comma or space)", Utils::speedUnitLabel(user.speedDisplayMode))), 0, wxALL, 5);
    txtSpeeds = new wxTextCtrl(this, wxID_ANY);
    sizer->Add(txtSpeeds, 0, wxALL | wxEXPAND, 5);

    chkInternalInput = new wxCheckBox(this, wxID_ANY, "Use in-app writing grid during reception");
    sizer->Add(chkInternalInput, 0, wxALL, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Group separator used by the in-app notes parser"), 0, wxALL, 5);
    txtSeparator = new wxTextCtrl(this, wxID_ANY);
    sizer->Add(txtSeparator, 0, wxALL | wxEXPAND, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Allowed symbols for transmitted groups"), 0, wxALL, 5);
    txtAllowedSymbols = new wxTextCtrl(this, wxID_ANY);
    sizer->Add(txtAllowedSymbols, 0, wxALL | wxEXPAND, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Pitch for this reception session"), 0, wxALL, 5);
    cbxPitch = new wxComboBox(this, wxID_ANY);
    wxArrayString pitches;
    for (int i = 300; i <= 1200; i += 10)
        pitches.Add(wxString::Format("%d", i));
    cbxPitch->Append(pitches);
    sizer->Add(cbxPitch, 0, wxALL | wxEXPAND, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Signal type for this reception session"), 0, wxALL, 5);
    cbxSignalType = new wxComboBox(this, wxID_ANY);
    wxArrayString types;
    types.Add("Sine");
    types.Add("Square");
    types.Add("Triangle");
    types.Add("Sawtooth");
    cbxSignalType->Append(types);
    sizer->Add(cbxSignalType, 0, wxALL | wxEXPAND, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Result entry time (minutes)"), 0, wxALL, 5);
    spnResultEntryMinutes = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 120);
    sizer->Add(spnResultEntryMinutes, 0, wxALL | wxEXPAND, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Transmission time per speed (seconds)"), 0, wxALL, 5);
    spnTransmissionSeconds = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10, 600);
    sizer->Add(spnTransmissionSeconds, 0, wxALL | wxEXPAND, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Pause before each speed (seconds)"), 0, wxALL, 5);
    spnPauseSeconds = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 600);
    sizer->Add(spnPauseSeconds, 0, wxALL | wxEXPAND, 5);

    lblHelp = new wxStaticText(this, wxID_ANY, "");
    sizer->Add(lblHelp, 0, wxALL, 10);

    wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    auto* saveButton = new wxButton(this, wxID_OK, "Save");
    auto* cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
    buttons->Add(saveButton, 0, wxALL, 5);
    buttons->Add(cancelButton, 0, wxALL, 5);
    sizer->Add(buttons, 0, wxALIGN_CENTER);

    chkUseLastSession->Bind(wxEVT_CHECKBOX, &ReceptionSettingsDialog::OnUseLastSessionChanged, this);
    saveButton->Bind(wxEVT_BUTTON, &ReceptionSettingsDialog::OnSave, this);
    cancelButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { EndModal(wxCANCEL); });

    SetSizerAndFit(sizer);
    loadSettingsIntoControls(lastSessionSettings);
}

void ReceptionSettingsDialog::loadSettingsIntoControls(const ReceptionSettings& settings)
{
    txtSpeeds->SetValue(wxString::FromUTF8(speedsToString(settings.speeds, user.speedDisplayMode)));
    chkInternalInput->SetValue(settings.useInternalInput);
    txtSeparator->SetValue(wxString::FromUTF8(settings.groupSeparator));
    txtAllowedSymbols->SetValue(wxString::FromUTF8(settings.allowedSymbols));
    cbxPitch->SetStringSelection(wxString::Format("%d", settings.pitch));
    cbxSignalType->SetSelection(static_cast<int>(settings.signalType));
    spnResultEntryMinutes->SetValue(settings.resultEntryMinutes);
    spnTransmissionSeconds->SetValue(settings.transmissionSeconds);
    spnPauseSeconds->SetValue(settings.pauseSeconds);

    const wxString separatorText = wxString::FromUTF8(separatorForDisplay(settings.groupSeparator));
    lblHelp->SetLabel(
        "In-app notes format:\n"
        "Use one row per speed.\n"
        + wxString::Format("Separate groups with '%s'.\n", separatorText)
        + wxString::Format("Examples: ABCDE%sFGHIJ%sKLMNO or ABC%sFGHIJ when one group is incomplete.", separatorText, separatorText, separatorText));
}

void ReceptionSettingsDialog::OnUseLastSessionChanged(wxCommandEvent& event)
{
    (void)event;
    loadSettingsIntoControls(chkUseLastSession->GetValue() ? lastSessionSettings : defaultSettings);
}

void ReceptionSettingsDialog::OnSave(wxCommandEvent& event)
{
    (void)event;
    std::vector<int> speeds = parseSpeeds(txtSpeeds->GetValue(), user.speedDisplayMode);
    if (speeds.empty() || speeds.size() > 10)
    {
        wxMessageBox("Please enter between 1 and 10 valid speeds.", "Invalid speeds", wxOK | wxICON_ERROR, this);
        return;
    }

    const std::string separator = txtSeparator->GetValue().ToStdString();
    if (separator.size() != 1)
    {
        wxMessageBox("Please enter exactly one separator character.", "Invalid separator", wxOK | wxICON_ERROR, this);
        return;
    }

    if (txtAllowedSymbols->GetValue().Find(separator.front()) != wxNOT_FOUND)
    {
        wxMessageBox("The separator must not also be part of the transmitted symbol set.", "Invalid separator", wxOK | wxICON_ERROR, this);
        return;
    }

    user.moduleSettings.reception.speeds = speeds;
    user.moduleSettings.reception.useInternalInput = chkInternalInput->GetValue();
    user.moduleSettings.reception.groupSeparator = separator;
    user.moduleSettings.reception.allowedSymbols = txtAllowedSymbols->GetValue().ToStdString();
    cbxPitch->GetValue().ToInt(&user.moduleSettings.reception.pitch);
    user.moduleSettings.reception.signalType = signalTypeFromSelection(cbxSignalType->GetValue());
    user.moduleSettings.reception.resultEntryMinutes = spnResultEntryMinutes->GetValue();
    user.moduleSettings.reception.transmissionSeconds = spnTransmissionSeconds->GetValue();
    user.moduleSettings.reception.pauseSeconds = spnPauseSeconds->GetValue();
    user.moduleSettings.reception.groupLength = 5;

    EndModal(wxOK);
}
