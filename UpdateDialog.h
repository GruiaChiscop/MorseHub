#pragma once
#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/string.h>
#include <wx/window.h>

class MorseHubUpdateDialog: public wxDialog
{
public:
MorseHubUpdateDialog(wxWindow* parent);
private:
void OnUpdateCommand(wxCommandEvent& event);
void Fetch();
wxButton* closeBTN;
wxButton* updateBTN;
wxTextCtrl* updateInfoTextCtrl;
};