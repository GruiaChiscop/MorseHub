#include "WelcomeWindow.h"
#include "MorseGenerator.h"
#include "MainWindow.h"
#include <string>
#include "User.h"
#include <memory>
#include "Utilities.h"
#include "SignalGenerator.h"

void WelcomeWindow::OnClose(wxCloseEvent& event)
{
	wxMessageBox("you must click finish to close this window", "Can't close the dialog", wxICON_ERROR | wxOK);
}

void WelcomeWindow::OnTest(wxEvent& event)
{
	int speed;
	cbxSpeed->GetValue().ToInt(&speed);
	int frequency;
	cbxPitch->GetValue().ToInt((&frequency);
	wxString signal = cbxSignalType->GetValue();
	SType type;
	if(signal=="Sine") type = Sine;
	else if(signal == "Square") type = Square;
	else if(signal == "Triangle") type = Triangle;
	else type = Sawtooth;
	gen = std::make_unique<MorseGenerator>(speed, frequency, type);
		gen->transmitAsync("vvv");
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
	serialize(user);
	SType type;
	wxString signal = cbxSignalType->GetValue();
		if(signal=="Sine") type = Sine;
	else if(signal == "Square") type = Square;
	else if(signal == "Triangle") type = Triangle;
	else type = Sawtooth;

	Utils::morseGenerator = std::make_unique<MorseGenerator>(user.defaultSpeed, user.defaultPitch, type);
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
	MainFrame* f = new MainFrame(user);
	f->Show();
	wxTheApp->SetTopWindow(f);
	updateUser();


}