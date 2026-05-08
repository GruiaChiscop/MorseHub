#include "RUFZCompetitionFrame.h"
#include "SessionDetailsDialog.h"
#include "Utilities.h"
#include <string>
#include <algorithm>

using std::string;

void RUFZCompetitionFrame::play()
{
    m_pitch = Utils::randomInteger(600, 1000);
    generator->setFrequency(m_pitch);
    generator->setSpeed(sessionState.currentSpeedCpm);
    m_text = Utils::toUpper(strgen.generate(5, GroupCharsetMode::LettersDigitsPunctuation));
    m_acceptingInput = false;
    m_repeatUsed = false;
    editField->SetValue("");
    resultText->SetLabel("Listen...");
    generator->transmitAsync(m_text);
    const int playbackDurationMs = static_cast<int>(generator->estimateDurationSeconds(m_text) * 1000.0);
    playbackTimer.StartOnce(std::max(playbackDurationMs, 1));
}

void RUFZCompetitionFrame::play(const std::string& text)
{
    m_acceptingInput = false;
    resultText->SetLabel("Repeating...");
    generator->transmitAsync(text);
    const int playbackDurationMs = static_cast<int>(generator->estimateDurationSeconds(text) * 1000.0);
    playbackTimer.StartOnce(std::max(playbackDurationMs, 1));
}
void RUFZCompetitionFrame::finish(bool saveSession)
{
    if (m_finished)
        return;

    m_finished = true;
    playbackTimer.Stop();
    m_acceptingInput = false;
    if (generator)
        generator->waitForPlaybackStop();

    if (!saveSession || m_results.empty())
    {
        Destroy();
        return;
    }

    int repeatCount = 0;
    int maxSpeed = user.defaultSpeed;
    for (const Result& result : m_results)
    {
        if (result.usedRepeat)
            repeatCount += 1;
        maxSpeed = std::max(maxSpeed, result.speed);
    }

    SessionRecord session;
    session.mode = "rufz";
    session.startedAt = sessionStartedAt.ToStdString();
    session.rounds = sessionState.roundIndex;
    session.totalPoints = sessionState.totalPoints;
    session.totalErrors = sessionState.totalErrors;
    session.maxSpeed = maxSpeed;
    session.repeatCount = repeatCount;
    session.results = m_results;

    try
    {
        SessionHistoryStore::append(session);
    }
    catch (...)
    {
    }

    SessionDetailsDialog details(this, user, session);
    details.ShowModal();
    Destroy();
}
void RUFZCompetitionFrame::OnAppear(wxShowEvent& event)
{
    if (event.IsShown() && sessionState.roundIndex == 0 && m_text.empty())
    {
        play();
    }
    event.Skip();
}
void RUFZCompetitionFrame::OnKeyPress(wxKeyEvent& event)
{
    if(event.GetKeyCode() == WXK_F6 && m_acceptingInput && !m_repeatUsed && sessionConfig.allowRepeat)
    {
        m_repeatUsed = true;
        play(m_text);
        return;
    }
    if(event.GetKeyCode()==WXK_RETURN)
    {
        evaluate();
        return;
    }
    event.Skip();
}
void RUFZCompetitionFrame::OnExit(wxCommandEvent& event)
{
    (void)event;
    if (m_results.empty())
    {
        finish(false);
        return;
    }

    if (sessionState.roundIndex < sessionConfig.maxRounds)
    {
        const int answer = wxMessageBox(
            "Save the current RUFZ session before exiting?\nChoose No to discard it.",
            "Exit RUFZ session",
            wxYES_NO | wxCANCEL | wxICON_QUESTION,
            this);
        if (answer == wxCANCEL)
            return;
        finish(answer == wxYES);
        return;
    }

    finish(true);
}

void RUFZCompetitionFrame::OnCloseWindow(wxCloseEvent& event)
{
    (void)event;
    if (m_results.empty())
    {
        finish(false);
        return;
    }

    if (sessionState.roundIndex < sessionConfig.maxRounds)
    {
        const int answer = wxMessageBox(
            "Save the current RUFZ session before exiting?\nChoose No to discard it.",
            "Exit RUFZ session",
            wxYES_NO | wxCANCEL | wxICON_QUESTION,
            this);
        if (answer == wxCANCEL)
            return;
        finish(answer == wxYES);
        return;
    }

    finish(true);
}

void RUFZCompetitionFrame::evaluate()
{
    if (!m_acceptingInput)
        return;

    const string typedText = editField->GetValue().ToStdString();
    RufzRoundInput input;
    input.expected = m_text;
    input.typed = typedText;
    input.speedCpm = sessionState.currentSpeedCpm;
    input.elapsedMs = static_cast<int>(stopWatch.Time());
    input.usedRepeat = m_repeatUsed;

    const RufzRoundResult roundResult = scoringEngine.evaluate(sessionState, input);
    const RufzRoundResult perfectResult = scoringEngine.evaluate(sessionState, {m_text, m_text, input.speedCpm, input.elapsedMs, false});
    Result result;
    result.frequency = m_pitch;
    result.speed = input.speedCpm;
    result.signalType = m_signalType;
    result.typedText = typedText;
    result.text = m_text;
    result.possiblePoints = perfectResult.awardedPoints;
    result.points = roundResult.awardedPoints;
    result.elapsedMilliseconds = input.elapsedMs;
    result.usedRepeat = input.usedRepeat;
    result.errorCount = roundResult.errorCount;
    m_results.push_back(result);

    scoringEngine.applyResult(sessionState, roundResult);
    refreshScoreDisplay();

    wxString feedback = wxString::Format(
        "%s | %s -> %s | %d ms | +%d",
        wxString::FromUTF8(roundResult.feedbackText),
        wxString::FromUTF8(m_text),
        wxString::FromUTF8(typedText),
        input.elapsedMs,
        roundResult.awardedPoints);
    resultText->SetLabel(feedback);

    m_acceptingInput = false;
    editField->SetValue("");

    if(sessionState.roundIndex >= sessionConfig.maxRounds)
    {
        finish();
        return;
    }

    play();
}

void RUFZCompetitionFrame::refreshScoreDisplay()
{
    speedText->SetLabel(Utils::formatSpeed(sessionState.currentSpeedCpm, user.speedDisplayMode));
    pointsText->SetLabel(wxString::Format("%d", sessionState.totalPoints));
}

void RUFZCompetitionFrame::OnSubmit(wxCommandEvent& event)
{
    evaluate();
}

void RUFZCompetitionFrame::OnPlaybackFinished(wxTimerEvent& event)
{
    stopWatch.Start();
    m_acceptingInput = true;
    resultText->SetLabel("Type what you heard");
    editField->SetFocus();
}
