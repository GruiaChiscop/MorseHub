#pragma once
#include <wx/wx.h>
#include "User.h"
#include "MorseGenerator.h"
#include "SignalGenerator.h"

class RUFZCompetitionFrame : public wxFrame
{
    wxStopWatch stopWatch;
    wxTextCtrl* editField;
    wxStaticText* resultText;
    int m_speed, m_pitch;
    SType m_type;
    public:
    RUFZCompetitionFrame(wxWindow* parent, int& speed, int& pitch, SType& type) : wxFrame(parent, wxID_ANY, "Competition"), m_speed{speed}, m_pitch{pitch}, m_type{type}
    {
        wxPanel* panel = new wxPanel(this);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxStaticText* windowLabel = new wxStaticText(panel, wxID_ANY, "RufzXP competition");
        sizer->Add(windowLabel, 0, wxALL | wxCENTER, 10);
    }
    private:
};