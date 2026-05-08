#pragma once

#include <wx/listctrl.h>
#include <wx/timer.h>
#include <wx/wx.h>

#include <memory>
#include <optional>
#include <set>
#include <vector>

#include "AudioOutputDevice.h"
#include "MorseGenerator.h"
#include "Result.h"
#include "RunnerAudioEffects.h"
#include "RunnerSupport.h"
#include "RunnerStation.h"
#include "SessionHistory.h"
#include "User.h"

class RunnerCompetitionFrame : public wxFrame
{
public:
    RunnerCompetitionFrame(wxWindow* parent, User& user);

private:
    enum class RunnerPhase
    {
        SendingCq,
        SendingCaller,
        WaitingForOperator,
        SendingOperatorMessage,
        SendingDxReply
    };

    User user;
    wxStopWatch stopWatch;
    wxTimer playbackTimer;
    wxTimer operatorTimeoutTimer;
    wxTimer sessionTimer;
    wxTextCtrl* callField{};
    wxTextCtrl* rstField{};
    wxTextCtrl* nrField{};
    wxStaticText* statusText{};
    wxStaticText* speedText{};
    wxStaticText* pointsText{};
    wxStaticText* serialText{};
    wxStaticText* roundTypeText{};
    wxStaticText* scoreBreakdownText{};
    wxStaticText* sessionTimeText{};
    wxListCtrl* logList{};
    std::unique_ptr<MorseGenerator> generator = nullptr;
    RunnerRound currentRound;
    std::vector<Result> results;
    wxString sessionStartedAt{wxDateTime::Now().FormatISOCombined(' ')};
    int totalPoints{};
    int completedRounds{};
    bool acceptingInput{false};
    bool finished{false};
    RunnerDxState currentDxState{RunnerDxState::NeedQso};
    RunnerPhase phase{RunnerPhase::SendingCq};
    int nextSerialNumber{1};
    bool roundUsedCorrection{};
    Result pendingResult;
    bool hasPendingResult{false};
    bool pendingRoundAdvance{};
    bool pendingRoundSuccess{};
    int sessionSecondsRemaining{};
    int rawQsoCount{};
    std::set<std::string> workedPrefixes;

    void playNext();
    void playText(const std::string& text, RunnerPhase nextPhase, const wxString& status);
    void playIncomingText(const std::string& text, RunnerPhase nextPhase, const wxString& status);
    void playWaitingStatic();
    void finish();
    void updateSpeedDisplay();
    void updateInputState();
    void appendLogEntry(const Result& result);
    int currentOperatorTimeoutMs() const;
    const RunnerStation* currentTargetStation() const;
    wxString runnerStatusLabel(const Result& result) const;
    std::string combinedDxReplyText(const Result& result) const;
    std::vector<std::string> pickQrmTexts() const;
    void clearInputFields();
    void finalizePendingStep();
    void applyWpxScore(Result& result);
    void OnAppear(wxShowEvent& event);
    void OnSubmit(wxCommandEvent& event);
    void OnPlaybackFinished(wxTimerEvent& event);
    void OnOperatorTimeout(wxTimerEvent& event);
    void OnSessionTimer(wxTimerEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
};
