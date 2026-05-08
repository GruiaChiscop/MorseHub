#include "TrainerSettingsDialog.h"

#include "Utilities.h"

namespace
{
const std::string kDigitsAndSymbols = "1234567890.,/?=+";

std::string charsetForPreset(int selection)
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

int detectPreset(const std::string& allowedSymbols)
{
    for (int selection = 0; selection < 4; ++selection)
    {
        if (allowedSymbols == charsetForPreset(selection))
            return selection;
    }
    return 2;
}
}

TrainerSettingsDialog::TrainerSettingsDialog(wxWindow* parent, User& value) : wxDialog(parent, wxID_ANY, "Trainer mode settings"), user(value)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(new wxStaticText(this, wxID_ANY, "Configure trainer mode generation and speed behavior"), 0, wxALL | wxCENTER, 10);

    chkFixedSpeed = new wxCheckBox(this, wxID_ANY, "Use fixed speed");
    chkFixedSpeed->SetValue(user.moduleSettings.rufz.trainer.fixedSpeed);
    sizer->Add(chkFixedSpeed, 0, wxALL, 5);

    auto createNumberCombo = [this, sizer](const wxString& label, int from, int to, int selected) {
        sizer->Add(new wxStaticText(this, wxID_ANY, label), 0, wxALL, 5);
        auto* combo = new wxComboBox(this, wxID_ANY);
        wxArrayString values;
        for (int i = from; i <= to; ++i)
            values.Add(wxString::Format("%d", i));
        combo->Append(values);
        combo->SetStringSelection(wxString::Format("%d", selected));
        sizer->Add(combo, 0, wxALL | wxEXPAND, 5);
        return combo;
    };

    cbxSpeed = createNumberCombo(
        "Trainer speed (" + Utils::speedUnitLabel(user.speedDisplayMode) + ")",
        1,
        user.speedDisplayMode == SpeedDisplayMode::Wpm ? 100 : 500,
        Utils::speedToDisplayUnits(user.moduleSettings.rufz.trainer.speed, user.speedDisplayMode));
    cbxAdaptiveStep = createNumberCombo(
        "Adaptive speed step (" + Utils::speedUnitLabel(user.speedDisplayMode) + ")",
        1,
        user.speedDisplayMode == SpeedDisplayMode::Wpm ? 20 : 100,
        Utils::speedToDisplayUnits(user.moduleSettings.rufz.trainer.adaptiveStep, user.speedDisplayMode));
    cbxMinSpeed = createNumberCombo(
        "Minimum adaptive speed (" + Utils::speedUnitLabel(user.speedDisplayMode) + ")",
        1,
        user.speedDisplayMode == SpeedDisplayMode::Wpm ? 100 : 500,
        Utils::speedToDisplayUnits(user.moduleSettings.rufz.trainer.minSpeed, user.speedDisplayMode));
    cbxGroupLength = createNumberCombo("Group length", 1, 20, user.moduleSettings.rufz.trainer.groupLength);

    wxArrayString presetChoices;
    presetChoices.Add("Digits");
    presetChoices.Add("Letters");
    presetChoices.Add("Letters + Digits");
    presetChoices.Add("Digits + Symbols");
    rbxCharsetPreset = new wxRadioBox(this, wxID_ANY, "Quick symbol filters", wxDefaultPosition, wxDefaultSize, presetChoices, 1, wxRA_SPECIFY_COLS);
    rbxCharsetPreset->SetSelection(detectPreset(user.moduleSettings.rufz.trainer.allowedSymbols));
    sizer->Add(rbxCharsetPreset, 0, wxALL | wxEXPAND, 5);

    sizer->Add(new wxStaticText(this, wxID_ANY, "Symbols included in generated groups"), 0, wxALL, 5);
    const std::string& allSymbols = StringGen::charsetForMode(GroupCharsetMode::LettersDigitsPunctuation);
    wxArrayString symbolChoices;
    for (std::size_t i = 0; i < allSymbols.size(); ++i)
        symbolChoices.Add(wxString(allSymbols[i]));
    lstSymbols = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxSize(320, 260), symbolChoices);
    for (std::size_t i = 0; i < allSymbols.size(); ++i)
        lstSymbols->Check(static_cast<unsigned int>(i), user.moduleSettings.rufz.trainer.allowedSymbols.find(allSymbols[i]) != std::string::npos);
    sizer->Add(lstSymbols, 1, wxALL | wxEXPAND, 5);

    wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    auto* saveButton = new wxButton(this, wxID_OK, "Save");
    auto* cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
    buttons->Add(saveButton, 0, wxALL, 5);
    buttons->Add(cancelButton, 0, wxALL, 5);
    sizer->Add(buttons, 0, wxALIGN_CENTER);

    chkFixedSpeed->Bind(wxEVT_CHECKBOX, &TrainerSettingsDialog::OnFixedSpeedChanged, this);
    rbxCharsetPreset->Bind(wxEVT_RADIOBOX, &TrainerSettingsDialog::OnPresetChanged, this);
    saveButton->Bind(wxEVT_BUTTON, &TrainerSettingsDialog::OnSave, this);
    cancelButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { EndModal(wxCANCEL); });

    SetSizerAndFit(sizer);
    refreshEnabledState();
}

void TrainerSettingsDialog::applyPresetSelection(int selection)
{
    const std::string selectedCharset = charsetForPreset(selection);
    const std::string& allSymbols = StringGen::charsetForMode(GroupCharsetMode::LettersDigitsPunctuation);
    for (std::size_t i = 0; i < allSymbols.size(); ++i)
        lstSymbols->Check(static_cast<unsigned int>(i), selectedCharset.find(allSymbols[i]) != std::string::npos);
}

void TrainerSettingsDialog::OnFixedSpeedChanged(wxCommandEvent& event)
{
    (void)event;
    refreshEnabledState();
}

void TrainerSettingsDialog::OnPresetChanged(wxCommandEvent& event)
{
    (void)event;
    applyPresetSelection(rbxCharsetPreset->GetSelection());
}

void TrainerSettingsDialog::refreshEnabledState()
{
    const bool fixedSpeed = chkFixedSpeed->GetValue();
    cbxAdaptiveStep->Enable(!fixedSpeed);
    cbxMinSpeed->Enable(!fixedSpeed);
}

void TrainerSettingsDialog::OnSave(wxCommandEvent& event)
{
    (void)event;
    user.moduleSettings.rufz.trainer.fixedSpeed = chkFixedSpeed->GetValue();
    int displaySpeed = 0;
    int displayStep = 0;
    int displayMinSpeed = 0;
    cbxSpeed->GetValue().ToInt(&displaySpeed);
    cbxAdaptiveStep->GetValue().ToInt(&displayStep);
    cbxMinSpeed->GetValue().ToInt(&displayMinSpeed);
    user.moduleSettings.rufz.trainer.speed = Utils::displayUnitsToCpm(displaySpeed, user.speedDisplayMode);
    user.moduleSettings.rufz.trainer.adaptiveStep = Utils::displayUnitsToCpm(displayStep, user.speedDisplayMode);
    user.moduleSettings.rufz.trainer.minSpeed = Utils::displayUnitsToCpm(displayMinSpeed, user.speedDisplayMode);
    cbxGroupLength->GetValue().ToInt(&user.moduleSettings.rufz.trainer.groupLength);

    std::string allowedSymbols;
    const std::string& allSymbols = StringGen::charsetForMode(GroupCharsetMode::LettersDigitsPunctuation);
    for (std::size_t i = 0; i < allSymbols.size(); ++i)
    {
        if (lstSymbols->IsChecked(static_cast<unsigned int>(i)))
            allowedSymbols.push_back(allSymbols[i]);
    }
    if (allowedSymbols.empty())
        allowedSymbols = allSymbols;

    user.moduleSettings.rufz.trainer.allowedSymbols = allowedSymbols;
    EndModal(wxOK);
}
