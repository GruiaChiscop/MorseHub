#pragma once
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/datectrl.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/dynarray.h>
#include <wx/msgdlg.h>
#include "User.h"

class WelcomeWindow : public wxDialog
{
	wxTextCtrl* txtBoxName;
	wxTextCtrl* txtBoxCallsign;
	wxComboBox* cbxSpeed;
	wxComboBox* cbxPitch;
	wxComboBox* cbxSignalType;
	wxDatePickerCtrl* datePickerDateOfBirth;
	User user;
public:
	WelcomeWindow(User& u) : wxDialog(nullptr, wxID_ANY, "Welcome to Morse Hub")
	{
		user = u;

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

		wxStaticText* welcomeLabel = new wxStaticText(this, wxID_ANY, "Let's get started by setting up your profile");
		sizer->Add(welcomeLabel, 0, wxALL | wxCENTER, 10);

		wxStaticText* nameLabel = new wxStaticText(this, wxID_ANY, "Name");
		sizer->Add(nameLabel, 0, wxALL, 5);
		txtBoxName = new wxTextCtrl(this, wxID_ANY, u.name.c_str());
		sizer->Add(txtBoxName, 0, wxALL | wxEXPAND, 5);

		wxStaticText* callsignLabel = new wxStaticText(this, wxID_ANY, "Callsign");
		sizer->Add(callsignLabel, 0, wxALL, 5);
		txtBoxCallsign = new wxTextCtrl(this, wxID_ANY, u.callsign.c_str());
		sizer->Add(txtBoxCallsign, 0, wxALL | wxEXPAND, 5);

		wxStaticText* speedLabel = new wxStaticText(this, wxID_ANY, "Default speed");
		sizer->Add(speedLabel, 0, wxALL, 5);
		cbxSpeed = new wxComboBox(this, wxID_ANY);
		wxArrayString speeds;
		for (int i = 10; i < 1000; i++)
		{
			speeds.Add(wxString::Format("%d", i));  // Corrected string conversion
		}
		cbxSpeed->Append(speeds);
		cbxSpeed->SetStringSelection("20");  // Set default value
		sizer->Add(cbxSpeed, 0, wxALL | wxEXPAND, 5);

		// Pitch label and combo box
		wxStaticText* pitchLabel = new wxStaticText(this, wxID_ANY, "Default pitch");
		sizer->Add(pitchLabel, 0, wxALL, 5);
		cbxPitch = new wxComboBox(this, wxID_ANY);
		wxArrayString pitches;
		for (int i = 440; i <= 1000; i++)
		{
			pitches.Add(wxString::Format("%d", i));  // Corrected string conversion
		}
		cbxPitch->Append(pitches);
		cbxPitch->SetStringSelection("440");  // Set default value
		sizer->Add(cbxPitch, 0, wxALL | wxEXPAND, 5);

		wxStaticText* signalLabel = new wxStaticText(this, wxID_ANY, "Signal type");
		sizer->Add(signalLabel, 0, wxALL, 5);
		cbxSignalType = new wxComboBox(this, wxID_ANY);
		wxArrayString types;
		types.Add("Sine");
		types.Add("Square");
		types.Add("Triangle");
		types.Add("Sawtooth");  // Fixed typo
		cbxSignalType->Append(types);
		cbxSignalType->SetSelection(0);  // Set default value
		sizer->Add(cbxSignalType, 0, wxALL | wxEXPAND, 5);
wxButton* testButton = new wxButton(this, wxID_ANY, "Test");
sizer->Add(testButton, 0, wxALL | wxCENTER, 10);
		wxStaticText* dateLabel = new wxStaticText(this, wxID_ANY, "Your date of birth");
		sizer->Add(dateLabel, 0, wxALL, 5);
		datePickerDateOfBirth = new wxDatePickerCtrl(this, wxID_ANY, u.dateOfBirth);
		sizer->Add(datePickerDateOfBirth, 0, wxALL | wxEXPAND, 5);

		wxButton* finishButton = new wxButton(this, wxID_OK, "Finish");
		sizer->Add(finishButton, 0, wxALL | wxCENTER, 10);
		this->Bind(wxEVT_CLOSE_WINDOW, &WelcomeWindow::OnClose, this);
		testButton->Bind(wxEVT_BUTTON, &WelcomeWindow::OnTest, this);
		
		SetSizer(sizer);
		sizer->Fit(this);
	}
private:
	void OnClose(wxCloseEvent&									 event);
	void onFinish(wxEvent& event);
	void OnTest(wxEvent& event);
};