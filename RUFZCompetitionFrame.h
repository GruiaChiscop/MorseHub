#pragma once
#include <wx/wx.h>
#include <string>
#include "MorseGenerator.h"
#include "StringGen.h"
#include <memory>

class RUFZCompetitionFrame : public wxFrame
{
    wxStopWatch stopWatch;
    wxTextCtrl* editField;
    wxStaticText* resultText;
    wxStaticText* speedText;
    wxStaticText* pointsText;
    int m_speed, m_pitch;
    int m_points{};
    SType m_type;
    StringGen strgen;
    bool played = false;
    std::string m_text;
    std::unique_ptr<MorseGenerator> generator = nullptr;
    public:
    RUFZCompetitionFrame(wxWindow* parent, int& speed, int& pitch, SType& type) : wxFrame(parent, wxID_ANY, "Competition"), m_speed{speed}, m_pitch{pitch}, m_type{type}
    {
        generator = std::make_unique<MorseGenerator>(speed, pitch, type);
        wxPanel* panel = new wxPanel(this);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxStaticText* windowLabel = new wxStaticText(panel, wxID_ANY, "RufzXP competition");
        sizer->Add(windowLabel, 0, wxALL | wxCENTER, 10);
        //the textbox
        editField = new wxTextCtrl(panel, wxID_ANY);
        editField->SetLabel("Type what you hear");
        sizer->Add(editField, 0, wxALL | wxEXPAND, 5);
        //we don't make only one label, because if we've done so, the label's text would've been too large and It would have occupied a lot of space on the screen
resultText = new wxStaticText(panel, wxID_ANY, "No result to show");
sizer->Add(resultText, 0, wxALL, 5);
wxString speedString = wxString::Format("%d", m_speed);
wxString pointsString = wxString::Format("%d", m_points);
speedText = new wxStaticText(panel, wxID_ANY, speedString);
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
this->Bind(wxEVT_CLOSE_WINDOW, &RUFZCompetitionFrame::OnExit, this);
this->Bind(wxEVT_SHOW, &RUFZCompetitionFrame::OnAppear, this);
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
    void finish();
    //events
    void OnKeyPress(wxKeyEvent& event);
    void OnAppear(wxEvent& event);
    void OnSubmit(wxCommandEvent& event);
    void OnExit(wxEvent& event);
};