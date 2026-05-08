#include "RunnerCompetitionFrame.h"

#include <algorithm>
#include <random>

#include "SessionDetailsDialog.h"
#include "Utilities.h"

namespace
{
constexpr int kRunnerSampleRate = 11025;

double randomUnitInterval()
{
    static std::mt19937 generator(std::random_device{}());
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(generator);
}
}

RunnerCompetitionFrame::RunnerCompetitionFrame(wxWindow* parent, User& value)
    : wxFrame(
          parent,
          wxID_ANY,
          user.moduleSettings.runner.mode == RunnerMode::SingleCalls
              ? "Morse Runner - Single Calls"
              : (user.moduleSettings.runner.mode == RunnerMode::WpxCompetition ? "Morse Runner - WPX Competition" : "Morse Runner - Pile-Up")),
      playbackTimer(this),
      operatorTimeoutTimer(this),
      sessionTimer(this),
      sessionSecondsRemaining(user.moduleSettings.runner.competitionMinutes * 60),
      user(value)
{
    const auto outputDevice = user.preferredOutputDeviceIndex >= 0 ? std::optional<AudioOutputDevice>(AudioOutputDeviceService::resolvePlaybackDevice(user.preferredOutputDeviceIndex)) : std::nullopt;
    generator = std::make_unique<MorseGenerator>(user.moduleSettings.runner.initialSpeed, user.defaultPitch, user.signalType, outputDevice, kRunnerSampleRate);

    wxPanel* panel = new wxPanel(this);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(
        new wxStaticText(
            panel,
            wxID_ANY,
            user.moduleSettings.runner.mode == RunnerMode::SingleCalls
                ? "Morse Runner - Single Calls"
                : (user.moduleSettings.runner.mode == RunnerMode::WpxCompetition ? "Morse Runner - WPX Competition" : "Morse Runner - Pile-Up")),
        0,
        wxALL | wxCENTER,
        10);

    sizer->Add(new wxStaticText(panel, wxID_ANY, "Call"), 0, wxLEFT | wxRIGHT | wxTOP, 5);
    callField = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    sizer->Add(callField, 0, wxALL | wxEXPAND, 5);

    wxBoxSizer* exchangeSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* rstSizer = new wxBoxSizer(wxVERTICAL);
    rstSizer->Add(new wxStaticText(panel, wxID_ANY, "RST"), 0, wxBOTTOM, 3);
    rstField = new wxTextCtrl(panel, wxID_ANY, "599", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    rstSizer->Add(rstField, 0, wxEXPAND);
    exchangeSizer->Add(rstSizer, 1, wxRIGHT, 5);

    wxBoxSizer* nrSizer = new wxBoxSizer(wxVERTICAL);
    nrSizer->Add(new wxStaticText(panel, wxID_ANY, "NR"), 0, wxBOTTOM, 3);
    nrField = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    nrSizer->Add(nrField, 0, wxEXPAND);
    exchangeSizer->Add(nrSizer, 1, wxLEFT, 5);
    sizer->Add(exchangeSizer, 0, wxALL | wxEXPAND, 5);

    statusText = new wxStaticText(panel, wxID_ANY, "No call yet");
    sizer->Add(statusText, 0, wxALL, 5);

    sizer->Add(new wxStaticText(panel, wxID_ANY, "Current speed"), 0, wxALL, 5);
    speedText = new wxStaticText(panel, wxID_ANY, "");
    sizer->Add(speedText, 0, wxALL, 5);

    sizer->Add(new wxStaticText(panel, wxID_ANY, "Current serial"), 0, wxALL, 5);
    serialText = new wxStaticText(panel, wxID_ANY, "001");
    sizer->Add(serialText, 0, wxALL, 5);

    sizer->Add(new wxStaticText(panel, wxID_ANY, "Round type"), 0, wxALL, 5);
    roundTypeText = new wxStaticText(panel, wxID_ANY, "Single caller");
    sizer->Add(roundTypeText, 0, wxALL, 5);

    sizer->Add(new wxStaticText(panel, wxID_ANY, "Points"), 0, wxALL, 5);
    pointsText = new wxStaticText(panel, wxID_ANY, "0");
    sizer->Add(pointsText, 0, wxALL, 5);

    scoreBreakdownText = new wxStaticText(panel, wxID_ANY, "");
    sizer->Add(scoreBreakdownText, 0, wxALL, 5);

    sessionTimeText = new wxStaticText(panel, wxID_ANY, "");
    sizer->Add(sessionTimeText, 0, wxALL, 5);

    sizer->Add(new wxStaticText(panel, wxID_ANY, "QSO log"), 0, wxALL, 5);
    logList = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxSize(860, 260), wxLC_REPORT | wxLC_SINGLE_SEL);
    logList->AppendColumn("#", wxLIST_FORMAT_LEFT, 45);
    logList->AppendColumn("DX", wxLIST_FORMAT_LEFT, 110);
    logList->AppendColumn("Call", wxLIST_FORMAT_LEFT, 110);
    logList->AppendColumn("RST", wxLIST_FORMAT_LEFT, 70);
    logList->AppendColumn("NR", wxLIST_FORMAT_LEFT, 70);
    logList->AppendColumn("Sent", wxLIST_FORMAT_LEFT, 170);
    logList->AppendColumn("DX reply", wxLIST_FORMAT_LEFT, 170);
    logList->AppendColumn("State", wxLIST_FORMAT_LEFT, 140);
    logList->AppendColumn("Points", wxLIST_FORMAT_LEFT, 80);
    sizer->Add(logList, 1, wxALL | wxEXPAND, 5);

    auto* submitButton = new wxButton(panel, wxID_OK, "Enter / Send");
    auto* exitButton = new wxButton(panel, wxID_CANCEL, "Exit");
    sizer->Add(submitButton, 0, wxALL | wxCENTER, 10);
    sizer->Add(exitButton, 0, wxALL | wxCENTER, 10);

    panel->SetSizer(sizer);
    sizer->Fit(this);

    updateSpeedDisplay();
    updateInputState();

    Bind(wxEVT_SHOW, &RunnerCompetitionFrame::OnAppear, this);
    Bind(wxEVT_TIMER, &RunnerCompetitionFrame::OnPlaybackFinished, this);
    Bind(wxEVT_TIMER, &RunnerCompetitionFrame::OnOperatorTimeout, this, operatorTimeoutTimer.GetId());
    Bind(wxEVT_TIMER, &RunnerCompetitionFrame::OnSessionTimer, this, sessionTimer.GetId());
    Bind(wxEVT_CLOSE_WINDOW, &RunnerCompetitionFrame::OnCloseWindow, this);
    callField->Bind(wxEVT_TEXT_ENTER, &RunnerCompetitionFrame::OnSubmit, this);
    rstField->Bind(wxEVT_TEXT_ENTER, &RunnerCompetitionFrame::OnSubmit, this);
    nrField->Bind(wxEVT_TEXT_ENTER, &RunnerCompetitionFrame::OnSubmit, this);
    submitButton->Bind(wxEVT_BUTTON, &RunnerCompetitionFrame::OnSubmit, this);
    exitButton->Bind(wxEVT_BUTTON, &RunnerCompetitionFrame::OnExit, this);
}

void RunnerCompetitionFrame::playNext()
{
    acceptingInput = false;
    operatorTimeoutTimer.Stop();
    currentDxState = RunnerDxState::NeedQso;
    roundUsedCorrection = false;
    clearInputFields();
    rstField->SetValue("599");

    if (user.moduleSettings.runner.mode == RunnerMode::Pileup || user.moduleSettings.runner.mode == RunnerMode::WpxCompetition)
    {
        currentRound = RunnerSupport::generatePileupRound(user.moduleSettings.runner.initialSpeed, user.defaultPitch, user.signalType);
    }
    else
    {
        currentRound = {};
        currentRound.kind = RunnerRoundKind::SingleCall;
        currentRound.stations.push_back(RunnerSupport::generateSingleCallStation(user.moduleSettings.runner.initialSpeed));
        currentRound.targetIndex = 0;
        currentRound.transmittedText = currentRound.stations.front().callsign;
        currentRound.playbackSpeedCpm = currentRound.stations.front().speedCpm;
        currentRound.pitch = user.defaultPitch;
        currentRound.signalType = user.signalType;
    }

    generator->setSpeed(currentRound.playbackSpeedCpm);
    generator->setFrequency(static_cast<float>(currentRound.pitch));
    updateSpeedDisplay();
    updateInputState();

    if (user.moduleSettings.runner.mode == RunnerMode::Pileup || user.moduleSettings.runner.mode == RunnerMode::WpxCompetition)
        playText("CQ TEST DE " + user.callsign + " " + user.callsign, RunnerPhase::SendingCaller, "Sending CQ...");
    else
        playIncomingText(currentRound.transmittedText, RunnerPhase::WaitingForOperator, "Incoming caller...");
}

void RunnerCompetitionFrame::playText(const std::string& text, RunnerPhase nextPhase, const wxString& status)
{
    playbackTimer.Stop();
    acceptingInput = false;
    operatorTimeoutTimer.Stop();
    generator->stopPlayback();
    phase = nextPhase;
    statusText->SetLabel(status);
    if (!text.empty())
        generator->transmitAsync(text);
    const int playbackDurationMs = static_cast<int>(generator->estimateDurationSeconds(text) * 1000.0);
    playbackTimer.StartOnce(std::max(playbackDurationMs, 1));
}

void RunnerCompetitionFrame::playIncomingText(const std::string& text, RunnerPhase nextPhase, const wxString& status)
{
    playbackTimer.Stop();
    acceptingInput = nextPhase == RunnerPhase::WaitingForOperator;
    operatorTimeoutTimer.Stop();
    generator->stopPlayback();
    phase = nextPhase;
    statusText->SetLabel(status);

    if (nextPhase == RunnerPhase::WaitingForOperator)
    {
        stopWatch.Start();
        updateInputState();
    }

    RunnerBandEffectsConfig config;
    config.enableQrm = user.moduleSettings.runner.enableQrm;
    config.enableQrn = user.moduleSettings.runner.enableQrn;
    config.enableQsb = user.moduleSettings.runner.enableQsb;
    config.enableFlutter = user.moduleSettings.runner.enableFlutter;
    config.sampleRate = kRunnerSampleRate;
    config.centerPitchHz = static_cast<float>(currentRound.pitch > 0 ? currentRound.pitch : user.defaultPitch);
    config.bandwidthHz = 500.0f;
    std::vector<float> effected = RunnerAudioEffects::renderIncomingSignal(
        text,
        std::max(60, currentRound.playbackSpeedCpm),
        config,
        pickQrmTexts(),
        currentRound.signalType);

    const std::size_t messageSamples = effected.size();
    if (nextPhase == RunnerPhase::WaitingForOperator)
    {
        std::vector<float> background = RunnerAudioEffects::createStationBackground(
            kRunnerSampleRate,
            8.0,
            user.moduleSettings.runner.enableQrn,
            user.moduleSettings.runner.enableFlutter);
        effected.insert(effected.end(), background.begin(), background.end());
    }

    const int playbackDurationMs = static_cast<int>((static_cast<double>(messageSamples) / static_cast<double>(kRunnerSampleRate)) * 1000.0);
    generator->transmitBufferAsync(std::move(effected));
    playbackTimer.StartOnce(std::max(playbackDurationMs, 1));
}

void RunnerCompetitionFrame::playWaitingStatic()
{
    std::vector<float> background = RunnerAudioEffects::createStationBackground(
        kRunnerSampleRate,
        8.0,
        user.moduleSettings.runner.enableQrn,
        user.moduleSettings.runner.enableFlutter);
    generator->transmitBufferAsync(std::move(background));
}

void RunnerCompetitionFrame::finish()
{
    if (finished)
        return;

    finished = true;
    playbackTimer.Stop();
    operatorTimeoutTimer.Stop();
    sessionTimer.Stop();
    acceptingInput = false;
    if (generator)
        generator->waitForPlaybackStop();

    if (results.empty() && completedRounds == 0)
    {
        Destroy();
        return;
    }

    SessionRecord session;
    session.mode = user.moduleSettings.runner.mode == RunnerMode::WpxCompetition ? "runner_wpx" : "runner";
    session.startedAt = sessionStartedAt.ToStdString();
    session.rounds = completedRounds;
    session.totalPoints = totalPoints;
    session.totalErrors = 0;
    session.maxSpeed = user.moduleSettings.runner.initialSpeed;
    session.results = results;
    for (const Result& result : results)
    {
        session.totalErrors += result.errorCount;
        session.maxSpeed = std::max(session.maxSpeed, result.speed);
        if (result.usedRepeat)
            ++session.repeatCount;
    }

    try
    {
        SessionHistoryStore::append(session);
    }
    catch (...)
    {
    }

    SessionDetailsDialog dialog(this, user, session);
    dialog.ShowModal();
    Destroy();
}

void RunnerCompetitionFrame::updateSpeedDisplay()
{
    const int displayedSpeed = currentRound.playbackSpeedCpm > 0 ? currentRound.playbackSpeedCpm : user.moduleSettings.runner.initialSpeed;
    speedText->SetLabel(Utils::formatSpeed(displayedSpeed, user.speedDisplayMode));
    pointsText->SetLabel(wxString::Format("%d", totalPoints));
    serialText->SetLabel(wxString::Format("%03d", nextSerialNumber));
    if (currentRound.kind == RunnerRoundKind::Pileup)
        roundTypeText->SetLabel(wxString::Format("Pile-up (%zu callers)", currentRound.stations.size()));
    else
        roundTypeText->SetLabel("Single caller");

    if (user.moduleSettings.runner.mode == RunnerMode::WpxCompetition)
    {
        scoreBreakdownText->SetLabel(wxString::Format(
            "WPX score: %d QSOs x %zu prefixes = %d",
            rawQsoCount,
            workedPrefixes.size(),
            totalPoints));
        sessionTimeText->SetLabel(wxString::Format(
            "Session time remaining: %d:%02d",
            std::max(0, sessionSecondsRemaining) / 60,
            std::max(0, sessionSecondsRemaining) % 60));
    }
    else
    {
        scoreBreakdownText->SetLabel("");
        sessionTimeText->SetLabel("");
    }
}

void RunnerCompetitionFrame::updateInputState()
{
    callField->Enable(acceptingInput);
    rstField->Enable(acceptingInput);
    nrField->Enable(acceptingInput);

    if (!acceptingInput)
        return;

    if ((user.moduleSettings.runner.mode == RunnerMode::Pileup || user.moduleSettings.runner.mode == RunnerMode::WpxCompetition) &&
        currentRound.stations.empty())
    {
        statusText->SetLabel("Press Enter to send CQ");
        callField->SetFocus();
        return;
    }

    switch (currentDxState)
    {
    case RunnerDxState::NeedQso:
    case RunnerDxState::NeedPrevEnd:
        statusText->SetLabel("Type the caller callsign and press Enter");
        callField->SetFocus();
        break;
    case RunnerDxState::NeedNr:
        statusText->SetLabel("Send the exchange number and press Enter");
        nrField->SetFocus();
        break;
    case RunnerDxState::NeedCall:
        statusText->SetLabel("Correct the callsign and press Enter");
        callField->SetFocus();
        break;
    case RunnerDxState::NeedCorrectedCall:
        statusText->SetLabel("Correct the call and send the exchange");
        callField->SetFocus();
        break;
    case RunnerDxState::NeedEnd:
        statusText->SetLabel("Press Enter on empty fields to send TU");
        callField->SetFocus();
        break;
    case RunnerDxState::Done:
    case RunnerDxState::Failed:
        break;
    }
}

void RunnerCompetitionFrame::appendLogEntry(const Result& result)
{
    const long row = logList->InsertItem(logList->GetItemCount(), wxString::Format("%d", static_cast<int>(results.size())));
    logList->SetItem(row, 1, wxString::FromUTF8(result.text));
    logList->SetItem(row, 2, wxString::FromUTF8(result.runnerEnteredCall));
    logList->SetItem(row, 3, wxString::FromUTF8(result.runnerEnteredRst));
    logList->SetItem(row, 4, wxString::FromUTF8(result.runnerEnteredNr));
    logList->SetItem(row, 5, wxString::FromUTF8(result.runnerSentText));
    logList->SetItem(row, 6, wxString::FromUTF8(combinedDxReplyText(result)));
    logList->SetItem(row, 7, wxString::FromUTF8(result.runnerStateText));
    logList->SetItem(row, 8, wxString::Format("%d / %d", result.points, result.possiblePoints));
    logList->EnsureVisible(row);
}

int RunnerCompetitionFrame::currentOperatorTimeoutMs() const
{
    const int speed = std::max(60, currentRound.playbackSpeedCpm > 0 ? currentRound.playbackSpeedCpm : user.moduleSettings.runner.initialSpeed);
    const int timeout = 8000 - speed * 15;
    return std::clamp(timeout, 2500, 7000);
}

const RunnerStation* RunnerCompetitionFrame::currentTargetStation() const
{
    if (currentRound.stations.empty() || currentRound.targetIndex >= currentRound.stations.size())
        return nullptr;
    return &currentRound.stations[currentRound.targetIndex];
}

wxString RunnerCompetitionFrame::runnerStatusLabel(const Result& result) const
{
    switch (result.runnerResultKind)
    {
    case RunnerResultKind::Copied:
        return "Copied";
    case RunnerResultKind::AfterRepeat:
        return "Copied after correction";
    case RunnerResultKind::Agn:
        return "AGN";
    case RunnerResultKind::B4:
        return "QSO B4";
    case RunnerResultKind::Bust:
        return "Bust";
    case RunnerResultKind::None:
    default:
        return "Step";
    }
}

std::string RunnerCompetitionFrame::combinedDxReplyText(const Result& result) const
{
    if (result.runnerResponseText.empty())
        return result.runnerRetransmittedText;
    if (result.runnerRetransmittedText.empty())
        return result.runnerResponseText;
    return result.runnerResponseText + " " + result.runnerRetransmittedText;
}

std::vector<std::string> RunnerCompetitionFrame::pickQrmTexts() const
{
    if (!user.moduleSettings.runner.enableQrm)
        return {};

    static const std::vector<std::string> kMessages{
        "QRL",
        "QRL QRL",
        "CQ CQ TEST",
        "QSY QSY"
    };
    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<std::size_t> distribution(0, kMessages.size() - 1);
    std::uniform_int_distribution<int> countDistribution(1, 3);
    const int count = countDistribution(generator);
    std::vector<std::string> result;
    result.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i)
        result.push_back(kMessages[distribution(generator)]);
    return result;
}

void RunnerCompetitionFrame::clearInputFields()
{
    callField->ChangeValue("");
    nrField->ChangeValue("");
}

void RunnerCompetitionFrame::finalizePendingStep()
{
    if (!hasPendingResult)
        return;

    if (user.moduleSettings.runner.mode == RunnerMode::WpxCompetition && pendingRoundSuccess)
        applyWpxScore(pendingResult);

    results.push_back(pendingResult);
    appendLogEntry(pendingResult);

    if (pendingRoundSuccess)
    {
        totalPoints += pendingResult.points;
        ++completedRounds;
        ++nextSerialNumber;
    }
    else if (pendingRoundAdvance)
    {
        ++completedRounds;
    }

    updateSpeedDisplay();

    hasPendingResult = false;
    pendingRoundAdvance = false;
    pendingRoundSuccess = false;

    if (completedRounds >= 25)
    {
        finish();
        return;
    }

    if (currentDxState == RunnerDxState::Done || currentDxState == RunnerDxState::Failed)
        playNext();
    else
    {
        acceptingInput = true;
        updateInputState();
    }
}

void RunnerCompetitionFrame::applyWpxScore(Result& result)
{
    const int previousScore = totalPoints;
    workedPrefixes.insert(RunnerSupport::extractWpxPrefix(result.text));
    ++rawQsoCount;
    const int updatedScore = rawQsoCount * static_cast<int>(workedPrefixes.size());
    result.possiblePoints = static_cast<int>(workedPrefixes.size());
    result.points = updatedScore - previousScore;
}

void RunnerCompetitionFrame::OnAppear(wxShowEvent& event)
{
    if (event.IsShown() && results.empty() && currentRound.stations.empty())
    {
        if (user.moduleSettings.runner.mode == RunnerMode::WpxCompetition && !sessionTimer.IsRunning())
            sessionTimer.Start(1000);
        if (user.moduleSettings.runner.mode == RunnerMode::SingleCalls)
            playNext();
        else
        {
            acceptingInput = true;
            phase = RunnerPhase::SendingCq;
            updateInputState();
        }
    }
    event.Skip();
}

void RunnerCompetitionFrame::OnSubmit(wxCommandEvent& event)
{
    (void)event;
    if (!acceptingInput)
        return;

    if ((user.moduleSettings.runner.mode == RunnerMode::Pileup || user.moduleSettings.runner.mode == RunnerMode::WpxCompetition) &&
        currentRound.stations.empty())
    {
        playNext();
        return;
    }

    const RunnerStation* target = currentTargetStation();
    if (target == nullptr)
        return;

    RunnerOperatorInput input{
        callField->GetValue().ToStdString(),
        rstField->GetValue().ToStdString(),
        nrField->GetValue().ToStdString()
    };

    const RunnerQsoStep step = RunnerSupport::processOperatorInput(
        currentDxState,
        *target,
        user.callsign,
        nextSerialNumber,
        input,
        user.moduleSettings.runner.mode != RunnerMode::SingleCalls,
        user.moduleSettings.runner.enableLids,
        randomUnitInterval());

    Result result;
    result.text = target->callsign;
    result.typedText = RunnerSupport::normalize(input.call);
    result.runnerEnteredCall = RunnerSupport::normalize(input.call);
    result.runnerEnteredRst = RunnerSupport::normalize(input.rst);
    result.runnerEnteredNr = RunnerSupport::normalize(input.nr);
    result.runnerSentText = step.sentText;
    result.runnerResponseText = step.dxReplyText;
    result.runnerRetransmittedText = step.retransmittedText;
    result.runnerStateText = RunnerSupport::stateLabel(step.nextState);
    result.runnerResultKind = step.resultKind;
    result.possiblePoints = target->speedCpm;
    result.points = step.isSuccessfulQso ? target->speedCpm : 0;
    if (roundUsedCorrection && step.isSuccessfulQso)
        result.runnerResultKind = RunnerResultKind::AfterRepeat;
    if (result.runnerResultKind == RunnerResultKind::AfterRepeat)
        result.points /= 2;
    result.elapsedMilliseconds = static_cast<int>(stopWatch.Time());
    result.speed = target->speedCpm;
    result.frequency = currentRound.pitch > 0 ? currentRound.pitch : user.defaultPitch;
    result.signalType = currentRound.signalType;
    result.usedRepeat = roundUsedCorrection && step.isSuccessfulQso;
    result.errorCount = step.isSuccessfulQso ? 0 : (step.resultKind == RunnerResultKind::Agn || step.resultKind == RunnerResultKind::B4 ? 0 : 1);

    if (step.resultKind == RunnerResultKind::AfterRepeat || step.nextState == RunnerDxState::NeedCorrectedCall)
        roundUsedCorrection = true;

    currentDxState = step.nextState;
    pendingResult = result;
    hasPendingResult = true;
    pendingRoundAdvance = step.isRoundComplete;
    pendingRoundSuccess = step.isSuccessfulQso;

    acceptingInput = false;

    clearInputFields();
    rstField->ChangeValue("599");

    if (!result.runnerSentText.empty())
    {
        generator->setSpeed(user.moduleSettings.runner.initialSpeed);
        generator->setFrequency(static_cast<float>(user.defaultPitch));
        playText(result.runnerSentText, RunnerPhase::SendingOperatorMessage, wxString::Format("Sending: %s", result.runnerSentText));
    }
    else if (!combinedDxReplyText(result).empty())
    {
        generator->setSpeed(target->speedCpm);
        generator->setFrequency(static_cast<float>(currentRound.pitch));
        playIncomingText(combinedDxReplyText(result), RunnerPhase::SendingDxReply, wxString::FromUTF8(runnerStatusLabel(result)));
    }
    else
    {
        finalizePendingStep();
    }
}

void RunnerCompetitionFrame::OnPlaybackFinished(wxTimerEvent& event)
{
    (void)event;
    switch (phase)
    {
    case RunnerPhase::SendingCaller:
        generator->setSpeed(currentRound.playbackSpeedCpm);
        generator->setFrequency(static_cast<float>(currentRound.pitch));
        playIncomingText(
            currentRound.transmittedText,
            RunnerPhase::WaitingForOperator,
            currentRound.kind == RunnerRoundKind::Pileup ? "Incoming pile-up..." : "Incoming caller...");
        break;
    case RunnerPhase::WaitingForOperator:
        acceptingInput = true;
        updateInputState();
        operatorTimeoutTimer.StartOnce(currentOperatorTimeoutMs());
        break;
    case RunnerPhase::SendingOperatorMessage:
        if (hasPendingResult && !combinedDxReplyText(pendingResult).empty())
        {
            const RunnerStation* target = currentTargetStation();
            if (target != nullptr)
            {
                generator->setSpeed(target->speedCpm);
                generator->setFrequency(static_cast<float>(currentRound.pitch));
            }
            playIncomingText(combinedDxReplyText(pendingResult), RunnerPhase::SendingDxReply, wxString::FromUTF8(runnerStatusLabel(pendingResult)));
        }
        else
            finalizePendingStep();
        break;
    case RunnerPhase::SendingDxReply:
        finalizePendingStep();
        break;
    case RunnerPhase::SendingCq:
        break;
    }
}

void RunnerCompetitionFrame::OnExit(wxCommandEvent& event)
{
    (void)event;
    finish();
}

void RunnerCompetitionFrame::OnCloseWindow(wxCloseEvent& event)
{
    (void)event;
    finish();
}

void RunnerCompetitionFrame::OnOperatorTimeout(wxTimerEvent& event)
{
    (void)event;
    if (!acceptingInput || finished)
        return;

    const RunnerStation* target = currentTargetStation();
    if (target == nullptr)
        return;

    acceptingInput = false;
    generator->stopPlayback();

    Result result;
    result.text = target->callsign;
    result.runnerEnteredCall = RunnerSupport::normalize(callField->GetValue().ToStdString());
    result.runnerEnteredRst = RunnerSupport::normalize(rstField->GetValue().ToStdString());
    result.runnerEnteredNr = RunnerSupport::normalize(nrField->GetValue().ToStdString());
    result.typedText = result.runnerEnteredCall;
    result.runnerSentText = "";
    result.runnerResponseText = "NIL";
    result.runnerRetransmittedText = "";
    result.runnerStateText = "Timed out";
    result.runnerResultKind = RunnerResultKind::Bust;
    result.possiblePoints = target->speedCpm;
    result.points = 0;
    result.elapsedMilliseconds = static_cast<int>(stopWatch.Time());
    result.speed = target->speedCpm;
    result.frequency = currentRound.pitch > 0 ? currentRound.pitch : user.defaultPitch;
    result.signalType = currentRound.signalType;
    result.usedRepeat = false;
    result.errorCount = 1;

    currentDxState = RunnerDxState::Failed;
    pendingResult = result;
    hasPendingResult = true;
    pendingRoundAdvance = true;
    pendingRoundSuccess = false;

    clearInputFields();
    rstField->ChangeValue("599");
    statusText->SetLabel("DX timed out and left");
    finalizePendingStep();
}

void RunnerCompetitionFrame::OnSessionTimer(wxTimerEvent& event)
{
    (void)event;
    if (finished || user.moduleSettings.runner.mode != RunnerMode::WpxCompetition)
        return;

    if (sessionSecondsRemaining > 0)
        --sessionSecondsRemaining;
    updateSpeedDisplay();

    if (sessionSecondsRemaining <= 0)
        finish();
}
