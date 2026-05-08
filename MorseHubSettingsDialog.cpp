#include "MorseHubSettingsDialog.h"

#include <sstream>

#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "StringGen.h"
#include "Utilities.h"

namespace
{
const std::string kDigitsAndSymbols = "1234567890.,/?=+";

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

std::string trainerCharsetForPreset(int selection)
{
    switch (selection)
    {
    case 0:
        return StringGen::charsetForMode(GroupCharsetMode::Digits);
    case 1:
        return StringGen::charsetForMode(GroupCharsetMode::Letters);
    case 2:
        return StringGen::charsetForMode(GroupCharsetMode::LettersDigits);
    default:
        return kDigitsAndSymbols;
    }
}

int detectTrainerPreset(const std::string& allowedSymbols)
{
    for (int selection = 0; selection < 4; ++selection)
    {
        if (allowedSymbols == trainerCharsetForPreset(selection))
            return selection;
    }
    return 2;
}
}

MorseHubSettingsDialog::MorseHubSettingsDialog(wxWindow* parent, User& value, int initialTab)
    : wxDialog(parent, wxID_ANY, "MorseHub settings", wxDefaultPosition, wxSize(760, 640)), user(value)
{
    wxBoxSizer* root = new wxBoxSizer(wxVERTICAL);
    notebook = new wxNotebook(this, wxID_ANY);
    notebook->AddPage(createGeneralPage(), "General");
    notebook->AddPage(createRufzPage(), "RUFZ");
    notebook->AddPage(createTrainerPage(), "Trainer");
    notebook->AddPage(createReceptionPage(), "Reception");
    notebook->AddPage(createRunnerPage(), "Runner");
    if (initialTab >= 0 && initialTab < static_cast<int>(notebook->GetPageCount()))
        notebook->SetSelection(initialTab);
    root->Add(notebook, 1, wxALL | wxEXPAND, 10);

    wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    auto* saveButton = new wxButton(this, wxID_OK, "Save");
    auto* cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
    buttons->Add(saveButton, 0, wxALL, 5);
    buttons->Add(cancelButton, 0, wxALL, 5);
    root->Add(buttons, 0, wxALIGN_CENTER | wxBOTTOM, 10);

    saveButton->Bind(wxEVT_BUTTON, &MorseHubSettingsDialog::OnSave, this);
    cancelButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { EndModal(wxCANCEL); });

    SetSizer(root);
    CentreOnParent();
}

wxPanel* MorseHubSettingsDialog::createGeneralPage()
{
    wxPanel* page = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Profile and audio defaults used across MorseHub"), 0, wxALL, 10);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Name"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    txtName = new wxTextCtrl(page, wxID_ANY, wxString::FromUTF8(user.name));
    sizer->Add(txtName, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Callsign"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    txtCallsign = new wxTextCtrl(page, wxID_ANY, wxString::FromUTF8(user.callsign));
    sizer->Add(txtCallsign, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Date of birth"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    datePickerDateOfBirth = new wxDatePickerCtrl(page, wxID_ANY, user.dateOfBirth);
    sizer->Add(datePickerDateOfBirth, 0, wxALL | wxEXPAND, 8);

    auto addNumberCombo = [page, sizer](const wxString& label, int from, int to, int selected) {
        sizer->Add(new wxStaticText(page, wxID_ANY, label), 0, wxLEFT | wxRIGHT | wxTOP, 8);
        auto* combo = new wxComboBox(page, wxID_ANY);
        wxArrayString values;
        for (int i = from; i <= to; ++i)
            values.Add(wxString::Format("%d", i));
        combo->Append(values);
        combo->SetStringSelection(wxString::Format("%d", selected));
        sizer->Add(combo, 0, wxALL | wxEXPAND, 8);
        return combo;
    };

    const int currentDisplaySpeed = Utils::speedToDisplayUnits(user.defaultSpeed, user.speedDisplayMode);
    const int speedMin = user.speedDisplayMode == SpeedDisplayMode::Wpm ? 2 : 10;
    const int speedMax = user.speedDisplayMode == SpeedDisplayMode::Wpm ? 60 : 300;
    cbxSpeed = addNumberCombo("Default speed (" + Utils::speedUnitLabel(user.speedDisplayMode) + ")", speedMin, speedMax, currentDisplaySpeed);
    sizer->Add(new wxStaticText(page, wxID_ANY, "Speed display mode"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    cbxSpeedDisplayMode = new wxComboBox(page, wxID_ANY);
    cbxSpeedDisplayMode->Append("CPM");
    cbxSpeedDisplayMode->Append("WPM");
    cbxSpeedDisplayMode->SetSelection(user.speedDisplayMode == SpeedDisplayMode::Wpm ? 1 : 0);
    sizer->Add(cbxSpeedDisplayMode, 0, wxALL | wxEXPAND, 8);
    cbxPitch = addNumberCombo("Default pitch", 300, 1200, user.defaultPitch);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Signal type"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    cbxSignalType = new wxComboBox(page, wxID_ANY);
    cbxSignalType->Append("Sine");
    cbxSignalType->Append("Square");
    cbxSignalType->Append("Triangle");
    cbxSignalType->Append("Sawtooth");
    cbxSignalType->SetSelection(static_cast<int>(user.signalType));
    sizer->Add(cbxSignalType, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Audio output device"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    cbxOutputDevice = new wxComboBox(page, wxID_ANY);
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
    sizer->Add(cbxOutputDevice, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Global volume"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    sldVolume = new wxSlider(page, wxID_ANY, user.outputVolumePercent, 0, 100);
    sizer->Add(sldVolume, 0, wxALL | wxEXPAND, 8);

    auto* testButton = new wxButton(page, wxID_ANY, "Test audio");
    sizer->Add(testButton, 0, wxALL | wxALIGN_LEFT, 8);
    testButton->Bind(wxEVT_BUTTON, &MorseHubSettingsDialog::OnTestAudio, this);

    page->SetSizer(sizer);
    return page;
}

wxPanel* MorseHubSettingsDialog::createRufzPage()
{
    wxPanel* page = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Core RUFZ session behavior"), 0, wxALL, 10);

    chkRufzAllowRepeat = new wxCheckBox(page, wxID_ANY, "Allow repeat / F6 penalty");
    chkRufzAllowRepeat->SetValue(user.moduleSettings.rufz.allowRepeat);
    sizer->Add(chkRufzAllowRepeat, 0, wxALL, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Maximum rounds"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    spnRufzMaxRounds = new wxSpinCtrl(page, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 500, user.moduleSettings.rufz.maxRounds);
    sizer->Add(spnRufzMaxRounds, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Speed step (" + Utils::speedUnitLabel(user.speedDisplayMode) + ")"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    spnRufzSpeedStep = new wxSpinCtrl(
        page,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS,
        1,
        user.speedDisplayMode == SpeedDisplayMode::Wpm ? 20 : 100,
        Utils::speedToDisplayUnits(user.moduleSettings.rufz.speedStepCpm, user.speedDisplayMode));
    sizer->Add(spnRufzSpeedStep, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Minimum speed (" + Utils::speedUnitLabel(user.speedDisplayMode) + ")"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    spnRufzMinSpeed = new wxSpinCtrl(
        page,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS,
        1,
        user.speedDisplayMode == SpeedDisplayMode::Wpm ? 100 : 500,
        Utils::speedToDisplayUnits(user.moduleSettings.rufz.minSpeedCpm, user.speedDisplayMode));
    sizer->Add(spnRufzMinSpeed, 0, wxALL | wxEXPAND, 8);

    page->SetSizer(sizer);
    return page;
}

wxPanel* MorseHubSettingsDialog::createTrainerPage()
{
    wxPanel* page = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Trainer settings under the RUFZ module"), 0, wxALL, 10);

    chkTrainerFixedSpeed = new wxCheckBox(page, wxID_ANY, "Use fixed speed");
    chkTrainerFixedSpeed->SetValue(user.moduleSettings.rufz.trainer.fixedSpeed);
    sizer->Add(chkTrainerFixedSpeed, 0, wxALL, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Trainer speed (" + Utils::speedUnitLabel(user.speedDisplayMode) + ")"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    spnTrainerSpeed = new wxSpinCtrl(
        page,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS,
        1,
        user.speedDisplayMode == SpeedDisplayMode::Wpm ? 100 : 500,
        Utils::speedToDisplayUnits(user.moduleSettings.rufz.trainer.speed, user.speedDisplayMode));
    sizer->Add(spnTrainerSpeed, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Adaptive speed step (" + Utils::speedUnitLabel(user.speedDisplayMode) + ")"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    spnTrainerAdaptiveStep = new wxSpinCtrl(
        page,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS,
        1,
        user.speedDisplayMode == SpeedDisplayMode::Wpm ? 20 : 100,
        Utils::speedToDisplayUnits(user.moduleSettings.rufz.trainer.adaptiveStep, user.speedDisplayMode));
    sizer->Add(spnTrainerAdaptiveStep, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Minimum adaptive speed (" + Utils::speedUnitLabel(user.speedDisplayMode) + ")"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    spnTrainerMinSpeed = new wxSpinCtrl(
        page,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS,
        1,
        user.speedDisplayMode == SpeedDisplayMode::Wpm ? 100 : 500,
        Utils::speedToDisplayUnits(user.moduleSettings.rufz.trainer.minSpeed, user.speedDisplayMode));
    sizer->Add(spnTrainerMinSpeed, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Group length"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    spnTrainerGroupLength = new wxSpinCtrl(page, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 20, user.moduleSettings.rufz.trainer.groupLength);
    sizer->Add(spnTrainerGroupLength, 0, wxALL | wxEXPAND, 8);

    wxArrayString presetChoices;
    presetChoices.Add("Digits");
    presetChoices.Add("Letters");
    presetChoices.Add("Letters + Digits");
    presetChoices.Add("Digits + Symbols");
    rbxTrainerCharsetPreset = new wxRadioBox(page, wxID_ANY, "Quick symbol filters", wxDefaultPosition, wxDefaultSize, presetChoices, 1, wxRA_SPECIFY_COLS);
    rbxTrainerCharsetPreset->SetSelection(detectTrainerPreset(user.moduleSettings.rufz.trainer.allowedSymbols));
    sizer->Add(rbxTrainerCharsetPreset, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Allowed symbols"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    const std::string& allSymbols = StringGen::charsetForMode(GroupCharsetMode::LettersDigitsPunctuation);
    wxArrayString symbolChoices;
    for (std::size_t i = 0; i < allSymbols.size(); ++i)
        symbolChoices.Add(wxString(allSymbols[i]));
    lstTrainerSymbols = new wxCheckListBox(page, wxID_ANY, wxDefaultPosition, wxSize(320, 260), symbolChoices);
    for (std::size_t i = 0; i < allSymbols.size(); ++i)
        lstTrainerSymbols->Check(static_cast<unsigned int>(i), user.moduleSettings.rufz.trainer.allowedSymbols.find(allSymbols[i]) != std::string::npos);
    sizer->Add(lstTrainerSymbols, 1, wxALL | wxEXPAND, 8);

    chkTrainerFixedSpeed->Bind(wxEVT_CHECKBOX, &MorseHubSettingsDialog::OnTrainerFixedSpeedChanged, this);
    rbxTrainerCharsetPreset->Bind(wxEVT_RADIOBOX, &MorseHubSettingsDialog::OnTrainerPresetChanged, this);
    refreshTrainerState();

    page->SetSizer(sizer);
    return page;
}

wxPanel* MorseHubSettingsDialog::createReceptionPage()
{
    wxPanel* page = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Zeus / reception settings"), 0, wxALL, 10);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Speeds (" + Utils::speedUnitLabel(user.speedDisplayMode) + ", 1 to 10 values, comma or space separated)"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    txtReceptionSpeeds = new wxTextCtrl(page, wxID_ANY, wxString::FromUTF8(speedsToString(user.moduleSettings.reception.speeds, user.speedDisplayMode)));
    sizer->Add(txtReceptionSpeeds, 0, wxALL | wxEXPAND, 8);

    chkReceptionInternalInput = new wxCheckBox(page, wxID_ANY, "Allow in-app writing during reception");
    chkReceptionInternalInput->SetValue(user.moduleSettings.reception.useInternalInput);
    sizer->Add(chkReceptionInternalInput, 0, wxALL, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Group separator for in-app notes"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    txtReceptionSeparator = new wxTextCtrl(page, wxID_ANY, wxString::FromUTF8(user.moduleSettings.reception.groupSeparator));
    sizer->Add(txtReceptionSeparator, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Allowed symbols"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    txtReceptionAllowedSymbols = new wxTextCtrl(page, wxID_ANY, wxString::FromUTF8(user.moduleSettings.reception.allowedSymbols));
    sizer->Add(txtReceptionAllowedSymbols, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Group length"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    spnReceptionGroupLength = new wxSpinCtrl(page, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, user.moduleSettings.reception.groupLength);
    sizer->Add(spnReceptionGroupLength, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Result entry time (minutes)"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    spnReceptionResultEntryMinutes = new wxSpinCtrl(page, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 120, user.moduleSettings.reception.resultEntryMinutes);
    sizer->Add(spnReceptionResultEntryMinutes, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Transmission duration per speed (seconds)"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    spnReceptionTransmissionSeconds = new wxSpinCtrl(page, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10, 600, user.moduleSettings.reception.transmissionSeconds);
    sizer->Add(spnReceptionTransmissionSeconds, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Pause before each speed (seconds)"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    spnReceptionPauseSeconds = new wxSpinCtrl(page, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 600, user.moduleSettings.reception.pauseSeconds);
    sizer->Add(spnReceptionPauseSeconds, 0, wxALL | wxEXPAND, 8);

    page->SetSizer(sizer);
    return page;
}

wxPanel* MorseHubSettingsDialog::createRunnerPage()
{
    wxPanel* page = new wxPanel(notebook);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Placeholder runner settings for the future module"), 0, wxALL, 10);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Initial speed (" + Utils::speedUnitLabel(user.speedDisplayMode) + ")"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    spnRunnerInitialSpeed = new wxSpinCtrl(
        page,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS,
        1,
        user.speedDisplayMode == SpeedDisplayMode::Wpm ? 200 : 1000,
        Utils::speedToDisplayUnits(user.moduleSettings.runner.initialSpeed, user.speedDisplayMode));
    sizer->Add(spnRunnerInitialSpeed, 0, wxALL | wxEXPAND, 8);

    wxArrayString modeChoices;
    modeChoices.Add("Pile-Up");
    modeChoices.Add("Single Calls");
    modeChoices.Add("WPX Competition");
    rbxRunnerMode = new wxRadioBox(page, wxID_ANY, "Default runner mode", wxDefaultPosition, wxDefaultSize, modeChoices, 1, wxRA_SPECIFY_COLS);
    switch (user.moduleSettings.runner.mode)
    {
    case RunnerMode::SingleCalls:
        rbxRunnerMode->SetSelection(1);
        break;
    case RunnerMode::WpxCompetition:
        rbxRunnerMode->SetSelection(2);
        break;
    case RunnerMode::Pileup:
    default:
        rbxRunnerMode->SetSelection(0);
        break;
    }
    sizer->Add(rbxRunnerMode, 0, wxALL | wxEXPAND, 8);

    sizer->Add(new wxStaticText(page, wxID_ANY, "Competition duration (minutes)"), 0, wxLEFT | wxRIGHT | wxTOP, 8);
    spnRunnerCompetitionMinutes = new wxSpinCtrl(page, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 120, user.moduleSettings.runner.competitionMinutes);
    sizer->Add(spnRunnerCompetitionMinutes, 0, wxALL | wxEXPAND, 8);

    chkRunnerQrm = new wxCheckBox(page, wxID_ANY, "Enable QRM");
    chkRunnerQrm->SetValue(user.moduleSettings.runner.enableQrm);
    sizer->Add(chkRunnerQrm, 0, wxALL, 8);

    chkRunnerQrn = new wxCheckBox(page, wxID_ANY, "Enable QRN");
    chkRunnerQrn->SetValue(user.moduleSettings.runner.enableQrn);
    sizer->Add(chkRunnerQrn, 0, wxALL, 8);

    chkRunnerQsb = new wxCheckBox(page, wxID_ANY, "Enable QSB");
    chkRunnerQsb->SetValue(user.moduleSettings.runner.enableQsb);
    sizer->Add(chkRunnerQsb, 0, wxALL, 8);

    chkRunnerFlutter = new wxCheckBox(page, wxID_ANY, "Enable Flutter");
    chkRunnerFlutter->SetValue(user.moduleSettings.runner.enableFlutter);
    sizer->Add(chkRunnerFlutter, 0, wxALL, 8);

    chkRunnerLids = new wxCheckBox(page, wxID_ANY, "Enable LIDS");
    chkRunnerLids->SetValue(user.moduleSettings.runner.enableLids);
    sizer->Add(chkRunnerLids, 0, wxALL, 8);

    rbxRunnerMode->Bind(wxEVT_RADIOBOX, &MorseHubSettingsDialog::OnRunnerModeChanged, this);
    refreshRunnerState();

    page->SetSizer(sizer);
    return page;
}

void MorseHubSettingsDialog::OnSave(wxCommandEvent& event)
{
    (void)event;

    const wxString nameValue = txtName->GetValue();
    const wxString callValue = txtCallsign->GetValue();
    if (nameValue.IsEmpty() || callValue.IsEmpty())
    {
        wxMessageBox("Name and callsign must not be empty.", "Invalid profile", wxOK | wxICON_ERROR, this);
        notebook->SetSelection(0);
        return;
    }

    const SpeedDisplayMode selectedDisplayMode = cbxSpeedDisplayMode->GetSelection() == 1 ? SpeedDisplayMode::Wpm : SpeedDisplayMode::Cpm;
    std::vector<int> receptionSpeeds = parseSpeeds(txtReceptionSpeeds->GetValue(), selectedDisplayMode);
    if (receptionSpeeds.empty() || receptionSpeeds.size() > 10)
    {
        wxMessageBox("Reception speeds must contain between 1 and 10 positive values.", "Invalid reception speeds", wxOK | wxICON_ERROR, this);
        notebook->SetSelection(3);
        return;
    }

    const std::string separator = txtReceptionSeparator->GetValue().ToStdString();
    if (separator.size() != 1)
    {
        wxMessageBox("Reception separator must be exactly one character.", "Invalid separator", wxOK | wxICON_ERROR, this);
        notebook->SetSelection(3);
        return;
    }

    if (txtReceptionAllowedSymbols->GetValue().Find(separator.front()) != wxNOT_FOUND)
    {
        wxMessageBox("The reception separator must not also appear in the transmitted symbol set.", "Invalid separator", wxOK | wxICON_ERROR, this);
        notebook->SetSelection(3);
        return;
    }

    user.name = nameValue.ToStdString();
    user.callsign = callValue.ToStdString();
    user.dateOfBirth = datePickerDateOfBirth->GetValue();
    int selectedDefaultSpeed = 0;
    cbxSpeed->GetValue().ToInt(&selectedDefaultSpeed);
    user.defaultSpeed = Utils::displayUnitsToCpm(selectedDefaultSpeed, selectedDisplayMode);
    cbxPitch->GetValue().ToInt(&user.defaultPitch);
    user.signalType = signalTypeFromSelection(cbxSignalType->GetValue());
    user.speedDisplayMode = selectedDisplayMode;
    user.preferredOutputDeviceIndex = cbxOutputDevice->GetSelection() <= 0 ? -1 : outputDevices.at(static_cast<std::size_t>(cbxOutputDevice->GetSelection() - 1)).index;
    user.outputVolumePercent = sldVolume->GetValue();

    user.moduleSettings.rufz.maxRounds = spnRufzMaxRounds->GetValue();
    user.moduleSettings.rufz.allowRepeat = chkRufzAllowRepeat->GetValue();
    user.moduleSettings.rufz.speedStepCpm = Utils::displayUnitsToCpm(spnRufzSpeedStep->GetValue(), selectedDisplayMode);
    user.moduleSettings.rufz.minSpeedCpm = Utils::displayUnitsToCpm(spnRufzMinSpeed->GetValue(), selectedDisplayMode);

    user.moduleSettings.rufz.trainer.fixedSpeed = chkTrainerFixedSpeed->GetValue();
    user.moduleSettings.rufz.trainer.speed = Utils::displayUnitsToCpm(spnTrainerSpeed->GetValue(), selectedDisplayMode);
    user.moduleSettings.rufz.trainer.adaptiveStep = Utils::displayUnitsToCpm(spnTrainerAdaptiveStep->GetValue(), selectedDisplayMode);
    user.moduleSettings.rufz.trainer.minSpeed = Utils::displayUnitsToCpm(spnTrainerMinSpeed->GetValue(), selectedDisplayMode);
    user.moduleSettings.rufz.trainer.groupLength = spnTrainerGroupLength->GetValue();
    {
        std::string allowedSymbols;
        const std::string& allSymbols = StringGen::charsetForMode(GroupCharsetMode::LettersDigitsPunctuation);
        for (std::size_t i = 0; i < allSymbols.size(); ++i)
        {
            if (lstTrainerSymbols->IsChecked(static_cast<unsigned int>(i)))
                allowedSymbols.push_back(allSymbols[i]);
        }
        if (allowedSymbols.empty())
            allowedSymbols = StringGen::charsetForMode(GroupCharsetMode::LettersDigitsPunctuation);
        user.moduleSettings.rufz.trainer.allowedSymbols = allowedSymbols;
    }

    user.moduleSettings.reception.speeds = receptionSpeeds;
    user.moduleSettings.reception.useInternalInput = chkReceptionInternalInput->GetValue();
    user.moduleSettings.reception.groupSeparator = separator;
    user.moduleSettings.reception.allowedSymbols = txtReceptionAllowedSymbols->GetValue().ToStdString().empty()
        ? StringGen::charsetForMode(GroupCharsetMode::LettersDigitsPunctuation)
        : txtReceptionAllowedSymbols->GetValue().ToStdString();
    user.moduleSettings.reception.groupLength = spnReceptionGroupLength->GetValue();
    user.moduleSettings.reception.resultEntryMinutes = spnReceptionResultEntryMinutes->GetValue();
    user.moduleSettings.reception.transmissionSeconds = spnReceptionTransmissionSeconds->GetValue();
    user.moduleSettings.reception.pauseSeconds = spnReceptionPauseSeconds->GetValue();

    user.moduleSettings.runner.initialSpeed = Utils::displayUnitsToCpm(spnRunnerInitialSpeed->GetValue(), selectedDisplayMode);
    user.moduleSettings.runner.competitionMinutes = spnRunnerCompetitionMinutes->GetValue();
    switch (rbxRunnerMode->GetSelection())
    {
    case 1:
        user.moduleSettings.runner.mode = RunnerMode::SingleCalls;
        user.moduleSettings.runner.enableQrm = chkRunnerQrm->GetValue();
        user.moduleSettings.runner.enableQrn = chkRunnerQrn->GetValue();
        user.moduleSettings.runner.enableQsb = chkRunnerQsb->GetValue();
        user.moduleSettings.runner.enableFlutter = chkRunnerFlutter->GetValue();
        user.moduleSettings.runner.enableLids = chkRunnerLids->GetValue();
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
        user.moduleSettings.runner.enableQrm = chkRunnerQrm->GetValue();
        user.moduleSettings.runner.enableQrn = chkRunnerQrn->GetValue();
        user.moduleSettings.runner.enableQsb = chkRunnerQsb->GetValue();
        user.moduleSettings.runner.enableFlutter = chkRunnerFlutter->GetValue();
        user.moduleSettings.runner.enableLids = chkRunnerLids->GetValue();
        break;
    }

    EndModal(wxOK);
}

void MorseHubSettingsDialog::OnTestAudio(wxCommandEvent& event)
{
    (void)event;
    int speed = user.defaultSpeed;
    int pitch = user.defaultPitch;
    cbxSpeed->GetValue().ToInt(&speed);
    speed = Utils::displayUnitsToCpm(speed, cbxSpeedDisplayMode->GetSelection() == 1 ? SpeedDisplayMode::Wpm : SpeedDisplayMode::Cpm);
    cbxPitch->GetValue().ToInt(&pitch);
    generator = std::make_unique<MorseGenerator>(speed, pitch, signalTypeFromSelection(cbxSignalType->GetValue()), selectedOutputDevice());
    generator->transmitAsync("vvv");
}

void MorseHubSettingsDialog::OnTrainerFixedSpeedChanged(wxCommandEvent& event)
{
    (void)event;
    refreshTrainerState();
}

void MorseHubSettingsDialog::OnTrainerPresetChanged(wxCommandEvent& event)
{
    (void)event;
    applyTrainerPresetSelection(rbxTrainerCharsetPreset->GetSelection());
}

void MorseHubSettingsDialog::OnRunnerModeChanged(wxCommandEvent& event)
{
    (void)event;
    refreshRunnerState();
}

void MorseHubSettingsDialog::refreshTrainerState()
{
    const bool fixedSpeed = chkTrainerFixedSpeed->GetValue();
    spnTrainerAdaptiveStep->Enable(!fixedSpeed);
    spnTrainerMinSpeed->Enable(!fixedSpeed);
}

void MorseHubSettingsDialog::refreshRunnerState()
{
    const bool isWpx = rbxRunnerMode->GetSelection() == 2;
    spnRunnerCompetitionMinutes->Enable(isWpx);

    if (isWpx)
    {
        chkRunnerQrm->SetValue(true);
        chkRunnerQrn->SetValue(true);
        chkRunnerQsb->SetValue(true);
        chkRunnerFlutter->SetValue(true);
        chkRunnerLids->SetValue(true);
    }

    chkRunnerQrm->Enable(!isWpx);
    chkRunnerQrn->Enable(!isWpx);
    chkRunnerQsb->Enable(!isWpx);
    chkRunnerFlutter->Enable(!isWpx);
    chkRunnerLids->Enable(!isWpx);
}

void MorseHubSettingsDialog::applyTrainerPresetSelection(int selection)
{
    const std::string selectedCharset = trainerCharsetForPreset(selection);
    const std::string& allSymbols = StringGen::charsetForMode(GroupCharsetMode::LettersDigitsPunctuation);
    for (std::size_t i = 0; i < allSymbols.size(); ++i)
        lstTrainerSymbols->Check(static_cast<unsigned int>(i), selectedCharset.find(allSymbols[i]) != std::string::npos);
}

std::optional<AudioOutputDevice> MorseHubSettingsDialog::selectedOutputDevice() const
{
    if (cbxOutputDevice->GetSelection() <= 0)
        return std::nullopt;

    return outputDevices.at(static_cast<std::size_t>(cbxOutputDevice->GetSelection() - 1));
}
