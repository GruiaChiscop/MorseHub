#pragma once
#include <wx/wx.h>
#include <wx/timer.h>
#include <string>
#include "MorseGenerator.h"
#include "StringGen.h"
#include <memory>
#include "User.h"
#include <vector>

#include "Result.h"
#include "RufzScoring.h"
#include "SessionHistory.h"
#include "Utilities.h"

class RUFZCompetitionFrame : public wxFrame
{
    wxStopWatch stopWatch;
    wxTimer playbackTimer;
    wxTextCtrl* editField;
    wxStaticText* resultText;
    wxStaticText* speedText;
    wxStaticText* pointsText;
    User user;
    int m_pitch{};
    SType m_signalType;
    StringGen strgen;
    bool m_acceptingInput = false;
    bool m_repeatUsed = false;
    bool m_finished = false;
    std::string m_text;
    std::vector<Result> m_results;
    wxString sessionStartedAt;
    std::unique_ptr<MorseGenerator> generator = nullptr;
    RufzSessionConfig sessionConfig;
    RufzSessionState sessionState;
    RufzScoringEngine scoringEngine;
    public:
    RUFZCompetitionFrame(wxWindow* parent, User& u) : wxFrame(parent, wxID_ANY, "Competition"), playbackTimer(this), user(u),
        sessionStartedAt(wxDateTime::Now().FormatISOCombined(' ')),
        sessionConfig{user.defaultSpeed, user.moduleSettings.rufz.speedStepCpm, user.moduleSettings.rufz.maxRounds, user.moduleSettings.rufz.allowRepeat, 0.5, user.moduleSettings.rufz.minSpeedCpm, 2.0, 0.95, 1.05},
        sessionState{0, 0, user.defaultSpeed, 0, 0},
        scoringEngine(sessionConfig)
    {
        const auto outputDevice = user.preferredOutputDeviceIndex >= 0 ? std::optional<AudioOutputDevice>(AudioOutputDeviceService::resolvePlaybackDevice(user.preferredOutputDeviceIndex)) : std::nullopt;
        generator = std::make_unique<MorseGenerator>(user.defaultSpeed, user.defaultPitch, user.signalType, outputDevice);
        m_pitch=user.defaultPitch; m_signalType=user.signalType;
        wxPanel* panel = new wxPanel(this);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxStaticText* windowLabel = new wxStaticText(panel, wxID_ANY, "RufzXP competition");
        sizer->Add(windowLabel, 0, wxALL | wxCENTER, 10);
        //the textbox
        editField = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
        editField->SetHint("Type what you hear");
        sizer->Add(editField, 0, wxALL | wxEXPAND, 5);
        //we don't make only one label, because if we've done so, the label's text would've been too large and It would have occupied a lot of space on the screen
resultText = new wxStaticText(panel, wxID_ANY, "No result to show");
sizer->Add(resultText, 0, wxALL, 5);
wxString pointsString = wxString::Format("%d", sessionState.totalPoints);
speedText = new wxStaticText(panel, wxID_ANY, Utils::formatSpeed(sessionState.currentSpeedCpm, user.speedDisplayMode));
pointsText = new wxStaticText(panel, wxID_ANY, pointsString);
wxStaticText* speedLabel = new wxStaticText(panel, wxID_ANY, "Current speed");
wxStaticText* pointsLabel = new wxStaticText(panel, wxID_ANY, "Points accumulated");
sizer->Add(speedLabel, 0, wxALL, 5);
sizer->Add(speedText, 0, wxALL, 5);
sizer->Add(pointsLabel, 0, wxALL, 5);
sizer->Add(pointsText, 0, wxALL, 5);
wxButton* submitBTN = new wxButton(panel, wxID_OK, "Submit");
sizer->Add(submitBTN, 0, wxALL | wxCENTER, 10);
//the exit button
wxButton* exitBTN = new wxButton(panel, wxID_CANCEL, "E&xit");
sizer->Add(exitBTN, 0, wxALL | wxCENTER, 10);
//bind the events
this->Bind(wxEVT_KEY_DOWN, &RUFZCompetitionFrame::OnKeyPress, this);
this->Bind(wxEVT_CLOSE_WINDOW, &RUFZCompetitionFrame::OnCloseWindow, this);
this->Bind(wxEVT_SHOW, &RUFZCompetitionFrame::OnAppear, this);
this->Bind(wxEVT_TIMER, &RUFZCompetitionFrame::OnPlaybackFinished, this);
editField->Bind(wxEVT_KEY_DOWN, &RUFZCompetitionFrame::OnKeyPress, this);
editField->Bind(wxEVT_TEXT_ENTER, &RUFZCompetitionFrame::OnSubmit, this);
submitBTN->Bind(wxEVT_BUTTON, &RUFZCompetitionFrame::OnSubmit, this);
exitBTN->Bind(wxEVT_BUTTON, &RUFZCompetitionFrame::OnExit, this);
//set the sizer
panel->SetSizer(sizer);
sizer->Fit(this);
    }
    private:
    void play();
    void play(const std::string& text);
    void evaluate();
    void finish(bool saveSession = true);
    void refreshScoreDisplay();
    //events
    void OnKeyPress(wxKeyEvent& event);
    void OnAppear(wxShowEvent& event);
    void OnSubmit(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    void OnPlaybackFinished(wxTimerEvent& event);
};
