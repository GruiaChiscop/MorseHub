#include "ReceptionCompetitionFrame.h"

#include <algorithm>

#include "Utilities.h"

namespace
{
constexpr char kPrefixText[] = "VVV=";
constexpr char kSuffixText[] = "+";

std::string separatorForDisplay(const std::string& separator)
{
    return separator.empty() ? ";" : separator;
}
}

ReceptionCompetitionFrame::ReceptionCompetitionFrame(wxWindow* parent, User& value)
    : wxFrame(parent, wxID_ANY, "Reception mode"), user(value), secondTimer(this)
{
    const std::string configuredSeparator = separatorForDisplay(user.moduleSettings.reception.groupSeparator);
    const auto outputDevice = user.preferredOutputDeviceIndex >= 0 ? std::optional<AudioOutputDevice>(AudioOutputDeviceService::resolvePlaybackDevice(user.preferredOutputDeviceIndex)) : std::nullopt;
    generator = std::make_unique<MorseGenerator>(user.defaultSpeed, user.moduleSettings.reception.pitch, user.moduleSettings.reception.signalType, outputDevice);

    wxPanel* panel = new wxPanel(this);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(new wxStaticText(panel, wxID_ANY, "Zeus / Reception mode"), 0, wxALL | wxCENTER, 10);

    phaseLabel = new wxStaticText(panel, wxID_ANY, "Preparing session...");
    speedLabel = new wxStaticText(panel, wxID_ANY, "Speed: -");
    countdownLabel = new wxStaticText(panel, wxID_ANY, "Time remaining: -");
    sizer->Add(phaseLabel, 0, wxALL, 5);
    sizer->Add(speedLabel, 0, wxALL, 5);
    sizer->Add(countdownLabel, 0, wxALL, 5);

    sizer->Add(new wxStaticText(panel, wxID_ANY, "Notes grid"), 0, wxALL, 5);
    notesGrid = new wxGrid(panel, wxID_ANY);
    initializeRows();
    notesGrid->CreateGrid(static_cast<int>(rows.size()), 2);
    notesGrid->SetColLabelValue(0, "Speed");
    notesGrid->SetColLabelValue(1, "Copied groups");
    for (int row = 0; row < static_cast<int>(rows.size()); ++row)
    {
        notesGrid->SetCellValue(row, 0, Utils::formatSpeed(rows[static_cast<std::size_t>(row)].speedCpm, user.speedDisplayMode));
        notesGrid->SetReadOnly(row, 0, true);
    }
    notesGrid->EnableEditing(user.moduleSettings.reception.useInternalInput);
    sizer->Add(notesGrid, 1, wxALL | wxEXPAND, 10);

    sizer->Add(
        new wxStaticText(
            panel,
            wxID_ANY,
            wxString::Format(
                "In-app notes: use one row per speed and separate groups with '%s'.\nExamples: ABCDE%sFGHIJ%sKLMNO or ABC%sFGHIJ when one group is incomplete.",
                configuredSeparator.c_str(),
                configuredSeparator.c_str(),
                configuredSeparator.c_str(),
                configuredSeparator.c_str())),
        0,
        wxALL,
        10);

    panel->SetSizer(sizer);
    sizer->Fit(this);
    SetSize(wxSize(980, 520));

    Bind(wxEVT_TIMER, &ReceptionCompetitionFrame::OnTimer, this);
    Bind(wxEVT_CLOSE_WINDOW, &ReceptionCompetitionFrame::OnClose, this);

    beginInitialPause();
    secondTimer.Start(1000);
}

void ReceptionCompetitionFrame::initializeRows()
{
    rows.clear();
    for (int speed : user.moduleSettings.reception.speeds)
    {
        rows.push_back(ReceptionSessionRow{speed, {}, {}, {}});
    }
}

void ReceptionCompetitionFrame::beginInitialPause()
{
    phase = SessionPhase::InitialPause;
    currentRow = -1;
    secondsRemaining = user.moduleSettings.reception.pauseSeconds;
    updateLabels();
}

void ReceptionCompetitionFrame::beginTransmissionRow(int rowIndex)
{
    currentRow = rowIndex;
    phase = SessionPhase::Transmission;
    secondsRemaining = user.moduleSettings.reception.transmissionSeconds;
    speedLabel->SetLabel("Speed: " + Utils::formatSpeed(rows[static_cast<std::size_t>(rowIndex)].speedCpm, user.speedDisplayMode));

    std::vector<std::string> groups;
    rows[static_cast<std::size_t>(rowIndex)].sentTransmission = buildTransmissionText(rows[static_cast<std::size_t>(rowIndex)].speedCpm, groups);
    rows[static_cast<std::size_t>(rowIndex)].sentGroups = groups;

    if (user.moduleSettings.reception.useInternalInput)
    {
        notesGrid->SetGridCursor(rowIndex, 1);
        notesGrid->EnableCellEditControl();
        notesGrid->MakeCellVisible(rowIndex, 1);
    }

    generator->setSpeed(rows[static_cast<std::size_t>(rowIndex)].speedCpm);
    generator->setFrequency(user.moduleSettings.reception.pitch);
    generator->transmitAsync(rows[static_cast<std::size_t>(rowIndex)].sentTransmission);
    updateLabels();
}

void ReceptionCompetitionFrame::beginPauseAfterRow(int rowIndex)
{
    if (rowIndex + 1 >= static_cast<int>(rows.size()))
    {
        finishSession();
        return;
    }

    phase = SessionPhase::BetweenSpeeds;
    secondsRemaining = user.moduleSettings.reception.pauseSeconds;
    currentRow = rowIndex;
    updateLabels();
}

void ReceptionCompetitionFrame::finishSession()
{
    if (phase == SessionPhase::Finished)
        return;

    phase = SessionPhase::Finished;
    secondTimer.Stop();
    if (generator)
        generator->waitForPlaybackStop();
    for (int row = 0; row < static_cast<int>(rows.size()); ++row)
    {
        rows[static_cast<std::size_t>(row)].enteredGroups = notesGrid->GetCellValue(row, 1).ToStdString();
    }

    ReceptionResultsDialog dialog(this, user, rows);
    dialog.ShowModal();
    Destroy();
}

void ReceptionCompetitionFrame::tick()
{
    if (secondsRemaining > 0)
    {
        --secondsRemaining;
        updateLabels();
        return;
    }

    if (phase == SessionPhase::InitialPause)
    {
        beginTransmissionRow(0);
    }
    else if (phase == SessionPhase::Transmission)
    {
        beginPauseAfterRow(currentRow);
    }
    else if (phase == SessionPhase::BetweenSpeeds)
    {
        beginTransmissionRow(currentRow + 1);
    }
}

std::string ReceptionCompetitionFrame::buildTransmissionText(int speedCpm, std::vector<std::string>& groupsOut)
{
    generator->setSpeed(speedCpm);
    std::string text = std::string(kPrefixText) + " ";
    std::string group;
    while (true)
    {
        group = generatorSource.generateFromCharset(user.moduleSettings.reception.groupLength, user.moduleSettings.reception.allowedSymbols);
        std::transform(group.begin(), group.end(), group.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

        const std::string candidate = text + group + " " + kSuffixText;
        if (!groupsOut.empty() && generator->estimateDurationSeconds(candidate) > static_cast<double>(user.moduleSettings.reception.transmissionSeconds))
            break;

        text += group + " ";
        groupsOut.push_back(group);
    }

    text += kSuffixText;
    return text;
}

void ReceptionCompetitionFrame::updateLabels()
{
    switch (phase)
    {
    case SessionPhase::InitialPause:
        phaseLabel->SetLabel("Phase: initial pause before the first speed");
        speedLabel->SetLabel("Speed: -");
        break;
    case SessionPhase::Transmission:
        phaseLabel->SetLabel(wxString::Format("Phase: transmitting row %d", currentRow + 1));
        break;
    case SessionPhase::BetweenSpeeds:
        phaseLabel->SetLabel(wxString::Format("Phase: pause before row %d", currentRow + 2));
        break;
    case SessionPhase::Finished:
        phaseLabel->SetLabel("Phase: finished");
        break;
    }

    countdownLabel->SetLabel(wxString::Format("Time remaining in this phase: %d:%02d", secondsRemaining / 60, secondsRemaining % 60));
}

void ReceptionCompetitionFrame::OnTimer(wxTimerEvent& event)
{
    (void)event;
    tick();
}

void ReceptionCompetitionFrame::OnClose(wxCloseEvent& event)
{
    secondTimer.Stop();
    if (generator)
        generator->waitForPlaybackStop();
    event.Skip();
}
