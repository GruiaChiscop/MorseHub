#include "TrainerCompetitionFrame.h"

#include "Utilities.h"

#include <algorithm>

TrainerCompetitionFrame::TrainerCompetitionFrame(wxWindow* parent, User& value)
    : wxFrame(parent, wxID_ANY, "Trainer mode"), playbackTimer(this), user(value), currentSpeed(value.moduleSettings.rufz.trainer.speed)
{
    const auto outputDevice = user.preferredOutputDeviceIndex >= 0 ? std::optional<AudioOutputDevice>(AudioOutputDeviceService::resolvePlaybackDevice(user.preferredOutputDeviceIndex)) : std::nullopt;
    generator = std::make_unique<MorseGenerator>(currentSpeed, user.defaultPitch, user.signalType, outputDevice);

    wxPanel* panel = new wxPanel(this);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(new wxStaticText(panel, wxID_ANY, "Trainer mode"), 0, wxALL | wxCENTER, 10);

    editField = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    sizer->Add(editField, 0, wxALL | wxEXPAND, 5);

    statusText = new wxStaticText(panel, wxID_ANY, "No result to show");
    sizer->Add(statusText, 0, wxALL, 5);

    sizer->Add(new wxStaticText(panel, wxID_ANY, "Current speed"), 0, wxALL, 5);
    speedText = new wxStaticText(panel, wxID_ANY, wxString::Format("%d", currentSpeed));
    sizer->Add(speedText, 0, wxALL, 5);

    auto* submitButton = new wxButton(panel, wxID_OK, "Submit");
    auto* exitButton = new wxButton(panel, wxID_CANCEL, "Exit");
    sizer->Add(submitButton, 0, wxALL | wxCENTER, 10);
    sizer->Add(exitButton, 0, wxALL | wxCENTER, 10);

    panel->SetSizer(sizer);
    sizer->Fit(this);

    this->Bind(wxEVT_SHOW, &TrainerCompetitionFrame::OnAppear, this);
    this->Bind(wxEVT_TIMER, &TrainerCompetitionFrame::OnPlaybackFinished, this);
    this->Bind(wxEVT_CLOSE_WINDOW, &TrainerCompetitionFrame::OnCloseWindow, this);
    this->Bind(wxEVT_KEY_DOWN, &TrainerCompetitionFrame::OnKeyPress, this);
    editField->Bind(wxEVT_KEY_DOWN, &TrainerCompetitionFrame::OnKeyPress, this);
    editField->Bind(wxEVT_TEXT_ENTER, &TrainerCompetitionFrame::OnSubmit, this);
    submitButton->Bind(wxEVT_BUTTON, &TrainerCompetitionFrame::OnSubmit, this);
    exitButton->Bind(wxEVT_BUTTON, &TrainerCompetitionFrame::OnExit, this);
}

void TrainerCompetitionFrame::playNext()
{
    generator->setSpeed(currentSpeed);
    generator->setFrequency(user.defaultPitch);
    currentText = Utils::toUpper(generatorSource.generateFromCharset(user.moduleSettings.rufz.trainer.groupLength, user.moduleSettings.rufz.trainer.allowedSymbols));
    repeatUsed = false;
    acceptingInput = false;
    editField->SetValue("");
    statusText->SetLabel("Listen...");
    generator->transmitAsync(currentText);
    const int playbackDurationMs = static_cast<int>(generator->estimateDurationSeconds(currentText) * 1000.0);
    playbackTimer.StartOnce(std::max(playbackDurationMs, 1));
}

void TrainerCompetitionFrame::playCurrent()
{
    acceptingInput = false;
    statusText->SetLabel("Repeating...");
    generator->transmitAsync(currentText);
    const int playbackDurationMs = static_cast<int>(generator->estimateDurationSeconds(currentText) * 1000.0);
    playbackTimer.StartOnce(std::max(playbackDurationMs, 1));
}

void TrainerCompetitionFrame::evaluate()
{
    if (!acceptingInput)
        return;

    const std::string typedText = Utils::toUpper(editField->GetValue().ToStdString());
    const bool exact = typedText == currentText;
    const int elapsedMs = static_cast<int>(stopWatch.Time());

    if (!user.moduleSettings.rufz.trainer.fixedSpeed)
    {
        if (exact)
            currentSpeed += user.moduleSettings.rufz.trainer.adaptiveStep;
        else
            currentSpeed = std::max(user.moduleSettings.rufz.trainer.minSpeed, currentSpeed - user.moduleSettings.rufz.trainer.adaptiveStep);
        generator->setSpeed(currentSpeed);
    }

    updateSpeedDisplay();
    statusText->SetLabel(wxString::Format("%s | %s -> %s | %d ms",
        exact ? "Correct" : "Try again",
        wxString::FromUTF8(currentText),
        wxString::FromUTF8(typedText),
        elapsedMs));

    Result result;
    result.text = currentText;
    result.typedText = typedText;
    result.points = exact ? currentSpeed : 0;
    result.elapsedMilliseconds = elapsedMs;
    result.speed = currentSpeed;
    result.frequency = user.defaultPitch;
    result.usedRepeat = repeatUsed;
    result.errorCount = exact ? 0 : 1;
    results.push_back(result);

    acceptingInput = false;
    playNext();
}

void TrainerCompetitionFrame::finish(bool saveSession)
{
    if (finished)
        return;

    finished = true;
    playbackTimer.Stop();
    acceptingInput = false;
    if (generator)
        generator->waitForPlaybackStop();

    if (!saveSession || results.empty())
    {
        Destroy();
        return;
    }

    int errorCount = 0;
    int repeatCount = 0;
    int maxSpeed = user.moduleSettings.rufz.trainer.speed;
    for (const Result& result : results)
    {
        errorCount += result.errorCount;
        if (result.usedRepeat)
            repeatCount += 1;
        maxSpeed = std::max(maxSpeed, result.speed);
    }

    SessionRecord session;
    session.mode = "rufz_trainer";
    session.startedAt = sessionStartedAt.ToStdString();
    session.rounds = static_cast<int>(results.size());
    session.totalPoints = 0;
    session.totalErrors = errorCount;
    session.maxSpeed = maxSpeed;
    session.repeatCount = repeatCount;
    session.results = results;

    try
    {
        SessionHistoryStore::append(session);
    }
    catch (...)
    {
    }

    wxMessageBox(
        wxString::Format("Trainer session saved.\nRounds: %d\nErrors: %d\nMax speed: %d", session.rounds, session.totalErrors, session.maxSpeed),
        "Trainer summary",
        wxOK | wxICON_INFORMATION,
        this);
    Destroy();
}

void TrainerCompetitionFrame::updateSpeedDisplay()
{
    speedText->SetLabel(Utils::formatSpeed(currentSpeed, user.speedDisplayMode));
}

void TrainerCompetitionFrame::OnAppear(wxShowEvent& event)
{
    if (event.IsShown() && currentText.empty())
        playNext();
    event.Skip();
}

void TrainerCompetitionFrame::OnSubmit(wxCommandEvent& event)
{
    evaluate();
}

void TrainerCompetitionFrame::OnKeyPress(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_F6 && acceptingInput && !repeatUsed)
    {
        repeatUsed = true;
        playCurrent();
        return;
    }
    if (event.GetKeyCode() == WXK_RETURN)
    {
        evaluate();
        return;
    }
    event.Skip();
}

void TrainerCompetitionFrame::OnPlaybackFinished(wxTimerEvent& event)
{
    stopWatch.Start();
    acceptingInput = true;
    statusText->SetLabel("Type what you heard");
    editField->SetFocus();
}

void TrainerCompetitionFrame::OnExit(wxCommandEvent& event)
{
    (void)event;
    finish(!results.empty());
}

void TrainerCompetitionFrame::OnCloseWindow(wxCloseEvent& event)
{
    (void)event;
    finish(!results.empty());
}
