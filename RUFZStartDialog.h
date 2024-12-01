#ifndef RUFX_START_DLG
#define RUFZ_START_DLG
#include <wx/wx.h>
#include "User.h"
#include "Utilities.h"
#include "SignalGenerator.h"
class RUFZStartDialog: public wxDialog
{
int m_speed, m_pitch;
SType m_type;
bool useExistent;
public:
RUFZStartDialog(wxWindow* parent, int& speed, int& pitch, SType& type) : wxDialog(parent, wxID_ANY, "RufzXP mode details")
{
    m_speed = speed;
    m_pitch = pitch;
    m_type = type;
    useExistent = true;
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		wxStaticText* welcomeLabel = new wxStaticText(this, wxID_ANY, "Here you select the settings for RufzXP mode");
		sizer->Add(welcomeLabel, 0, wxALL | wxCENTER, 10);
wxCheckBox* useExistentCheckBox = new wxCheckBox(this, wxID_ANY, "Use the existent settings");
useExistentCheckBox->SetValue(useExistent);

}
};
#endif