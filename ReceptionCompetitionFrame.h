#pragma once

#include <wx/grid.h>
#include <wx/timer.h>
#include <wx/wx.h>

#include <optional>
#include <string>
#include <vector>

#include "AudioOutputDevice.h"
#include "MorseGenerator.h"
#include "ReceptionResultsDialog.h"
#include "ReceptionTypes.h"
#include "StringGen.h"
#include "User.h"

class ReceptionCompetitionFrame : public wxFrame
{
public:
    ReceptionCompetitionFrame(wxWindow* parent, User& user);

private:
    enum class SessionPhase
    {
        InitialPause,
        Transmission,
        BetweenSpeeds,
        Finished
    };

    User user;
    wxTimer secondTimer;
    wxStaticText* phaseLabel{};
    wxStaticText* speedLabel{};
    wxStaticText* countdownLabel{};
    wxGrid* notesGrid{};
    StringGen generatorSource;
    std::unique_ptr<MorseGenerator> generator = nullptr;
    std::vector<ReceptionSessionRow> rows;
    SessionPhase phase{SessionPhase::InitialPause};
    int currentRow{-1};
    int secondsRemaining{};

    void initializeRows();
    void beginInitialPause();
    void beginTransmissionRow(int rowIndex);
    void beginPauseAfterRow(int rowIndex);
    void finishSession();
    void tick();
    std::string buildTransmissionText(int speedCpm, std::vector<std::string>& groupsOut);
    void updateLabels();

    void OnTimer(wxTimerEvent& event);
    void OnClose(wxCloseEvent& event);
};
