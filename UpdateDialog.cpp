#include "UpdateDialog.h"
#include <wx/wx.h>
MorseHubUpdateDialog::MorseHubUpdateDialog(wxWindow* parent):wxDialog(parent, wxID_ANY, "New update available")
{
wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
wxStaticText* titleLabel = new wxStaticText(this, wxID_ANY, "A new version of MorseHub is available");
sizer->Add(titleLabel, 0, wxALL | wxCENTER, 10);
updateInfoTextCtrl = new wxTextCtrl(this, wxID_ANY, "No information provided for this update", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_BESTWRAP | wxTE_READONLY);
sizer->Add(updateInfoTextCtrl, 1, wxALL | wxEXPAND, 5);
closeBTN = new wxButton(this, wxID_CANCEL, "Close");
sizer->Add(closeBTN, 0, wxALL | wxCENTER, 10);
updateBTN = new wxButton(this, wxID_OK, "Update");
sizer->Add(updateBTN, 0, wxALL | wxCENTER, 10);
updateBTN->Bind(wxEVT_BUTTON, &MorseHubUpdateDialog::OnUpdateCommand, this);
SetSizer(sizer);
sizer->Fit(this);
}

void MorseHubUpdateDialog::OnUpdateCommand(wxCommandEvent& event)
{
//the update logic comes here
}
