#ifndef RUFX_START_DLG
#define RUFZ_START_DLG
#include <wx/wx.h>
#include "SignalGenerator.h"
#include <memory>
#include "MorseGenerator.h"

class RUFZStartDialog : public wxDialog
{
  int& m_speed;
  int& m_frequency;
  SType& m_type;
  bool useExistent;
  wxComboBox *cbxSpeed;
  wxComboBox *cbxPitch;
  wxComboBox *cbxSignalType;
  wxCheckBox *useExistentCheckBox;

std::unique_ptr<MorseGenerator> gen = nullptr;
public:
  RUFZStartDialog(wxWindow *parent, int &speed, int& frequency, SType &type) : wxDialog(parent, wxID_ANY, "RufzXP mode details"), m_speed{speed}, m_frequency{frequency}, m_type{type}
  {
    useExistent = true;
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText *welcomeLabel = new wxStaticText(this, wxID_ANY, "Here you select the settings for RufzXP mode");
    sizer->Add(welcomeLabel, 0, wxALL | wxCENTER, 10);
    useExistentCheckBox = new wxCheckBox(this, wxID_ANY, "Use the existent settings");
    useExistentCheckBox->SetValue(useExistent);
    wxStaticText *speedLabel = new wxStaticText(this, wxID_ANY, "Default speed");
    sizer->Add(speedLabel, 0, wxALL, 5);
    cbxSpeed = new wxComboBox(this, wxID_ANY);
    wxArrayString speeds;
    for (int i = 10; i < 1000; i++)
    {
      speeds.Add(wxString::Format("%d", i)); // Corrected string conversion
    }
    cbxSpeed->Append(speeds);
    cbxSpeed->SetStringSelection("20"); // Set default value
    sizer->Add(cbxSpeed, 0, wxALL | wxEXPAND, 5);
    cbxSpeed->Disable();

    // Pitch label and combo box
    wxStaticText *pitchLabel = new wxStaticText(this, wxID_ANY, "Default pitch");
    sizer->Add(pitchLabel, 0, wxALL, 5);
    cbxPitch = new wxComboBox(this, wxID_ANY);
    wxArrayString pitches;
    for (int i = 440; i <= 1000; i++)
    {
      pitches.Add(wxString::Format("%d", i)); // Corrected string conversion
    }
    cbxPitch->Append(pitches);
    cbxPitch->SetStringSelection("440"); // Set default value
    sizer->Add(cbxPitch, 0, wxALL | wxEXPAND, 5);
    cbxPitch->Disable();

    wxStaticText *signalLabel = new wxStaticText(this, wxID_ANY, "Signal type");
    sizer->Add(signalLabel, 0, wxALL, 5);
    cbxSignalType = new wxComboBox(this, wxID_ANY);
    wxArrayString types;
    types.Add("Sine");
    types.Add("Square");
    types.Add("Triangle");
    types.Add("Sawtooth"); // Fixed typo
    cbxSignalType->Append(types);
    cbxSignalType->SetSelection(0); // Set default value
    sizer->Add(cbxSignalType, 0, wxALL | wxEXPAND, 5);
cbxSignalType->Disable();
    wxButton *testButton = new wxButton(this, wxID_ANY, "Test");
    sizer->Add(testButton, 0, wxALL | wxCENTER, 10);
    wxButton* cancelBTN = new wxButton(this, wxID_CANCEL, "&Cancel");
    sizer->Add(cancelBTN, 0, wxALL | wxCENTER, 10);
    wxButton* okBTN = new wxButton(this, wxID_OK, "&OK");
    sizer->Add(okBTN, 0, wxALL | wxCENTER, 10);
    testButton->Bind(wxEVT_BUTTON, &RUFZStartDialog::OnTest, this);
    useExistentCheckBox->Bind(wxEVT_CHECKBOX, &RUFZStartDialog::OnCheckBox, this);
    okBTN->Bind(wxEVT_BUTTON, &RUFZStartDialog::OnOK, this);
    cancelBTN->Bind(wxEVT_BUTTON, &RUFZStartDialog::OnCancel, this);
    SetSizer(sizer);
    sizer->Fit(this);
  }
  void OnTest(wxEvent& event);
  void OnCheckBox(wxEvent& event);
  void OnOK(wxEvent& event);
  void OnCancel(wxEvent& event);
};
#endif