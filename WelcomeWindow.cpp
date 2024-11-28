#include "WelcomeWindow.h"
#include "MorseGenerator.h"
#include "MainWindow.h"
#include <string>
#include "User.h"
#include <memory>
#include "Utilities.h"
std::unique_ptr<MorseGenerator> gen = nullptr;

void WelcomeWindow::OnClose(wxCloseEvent& event)
{
	wxMessageBox("you must click finish to close this window", "Can't close the dialog", wxICON_ERROR | wxOK);
}

void WelcomeWindow::OnTest(wxEvent& event)
{
	int speed;
	cbxSpeed->GetValue().ToInt(&speed);
	int pitch;
	cbxPitch->GetValue().ToInt(&pitch);
	wxString signal = cbxSignalType->GetValue();
	MorseGenerator::SignalType type;
	if(signal=="Sine") type = MorseGenerator::SignalType_Sine;
	else if(signal == "Square") type = MorseGenerator::SignalType_Square;
	else if(signal == "Triangle") type = MorseGenerator::SignalType_Triangle;
	else type = MorseGenerator::SignalType_sawtooth;
	gen = std::make_unique<MorseGenerator>(speed, pitch, type);
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
	MorseGenerator::SignalType type;
	wxString signal = cbxSignalType->GetValue();
	if(signal=="Sine") type = MorseGenerator::SignalType_Sine;
	else if(signal == "Square") type = MorseGenerator::SignalType_Square;
	else if(signal == "Triangle") type = MorseGenerator::SignalType_Triangle;
	else type = MorseGenerator::SignalType_sawtooth;
	
	Utils::morseGenerator = std::make_unique<MorseGenerator>(user.defaultSpeed, user.defaultPitch, static_cast<MorseGenerator::SignalType>(type));
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