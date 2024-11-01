#include "WelcomeWindow.h"
#include "MorseGenerator.h"
#include <string>
#include "User.h"
MorseGenerator gen;

void WelcomeWindow::OnClose(wxCloseEvent& event)
{
	wxMessageBox("you must click finish to close this window", "Can't close the dialog", wxICON_ERROR | wxOK);
}

void WelcomeWindow::OnTest(wxEvent& event)
{
		gen.transmitAsync("vvv");
}

void WelcomeWindow::updateUser()
{
	user.callsign = txtBoxCallsign->GetValue().c_str();
	user.name = txtBoxName->GetValue().c_str();
	cbxPitch->GetValue().ToInt(&user.defaultPitch);
	cbxSpeed->GetValue().ToInt(&user.defaultSpeed);
	user.dateOfBirth = datePickerDateOfBirth->GetValue();
	if(txtBoxCallsign->GetValue().Lower() == "none")
	{
		wxMessageBox("You chose not to add any callsign. You can add one later in settings");
	}
	EndModal(wxID_OK);
}

void WelcomeWindow::onFinish(wxEvent& event)
{
	wxString nameValue = txtBoxName->GetValue();
	wxString callValue = txtBoxCallsign->GetValue();
	if(nameValue.IsEmpty() || callValue.IsEmpty())
	{
		wxMessageBox("Either name or callsign field is empty. If you do not have a callsign, just type none", "Name or callsign not found");
		return;
	}
	//updateUser();
}