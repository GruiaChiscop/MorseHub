#pragma once

#include <wx/timer.h>
#include <wx/wx.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "AudioOutputDevice.h"
#include "MorseGenerator.h"
#include "Result.h"
#include "SessionHistory.h"
#include "StringGen.h"
#include "User.h"

class TrainerCompetitionFrame : public wxFrame
{
    wxStopWatch stopWatch;
    wxTimer playbackTimer;
    wxTextCtrl* editField{};
    wxStaticText* statusText{};
    wxStaticText* speedText{};
    User user;
    StringGen generatorSource;
    std::vector<Result> results;
    wxString sessionStartedAt{wxDateTime::Now().FormatISOCombined(' ')};
    std::unique_ptr<MorseGenerator> generator = nullptr;
    std::string currentText;
    int currentSpeed{};
    bool acceptingInput{false};
    bool repeatUsed{false};
    bool finished{false};

public:
    TrainerCompetitionFrame(wxWindow* parent, User& user);

private:
    void playNext();
    void playCurrent();
    void evaluate();
    void finish(bool saveSession = true);
    void updateSpeedDisplay();
    void OnAppear(wxShowEvent& event);
    void OnSubmit(wxCommandEvent& event);
    void OnKeyPress(wxKeyEvent& event);
    void OnPlaybackFinished(wxTimerEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
};
