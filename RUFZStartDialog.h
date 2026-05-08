#ifndef RUFX_START_DLG
#define RUFZ_START_DLG

#include <wx/wx.h>

#include <memory>
#include <optional>
#include <vector>

#include "AudioOutputDevice.h"
#include "MorseGenerator.h"
#include "SignalGenerator.h"
#include "User.h"

class RUFZStartDialog : public wxDialog
{
  User& user;
  bool useExistent;
  wxComboBox* cbxSpeed;
  wxComboBox* cbxPitch;
  wxComboBox* cbxSignalType;
  wxComboBox* cbxOutputDevice;
  wxCheckBox* useExistentCheckBox;
  std::vector<AudioOutputDevice> outputDevices;
  std::unique_ptr<MorseGenerator> gen = nullptr;

public:
  RUFZStartDialog(wxWindow* parent, User& u);
  void OnTest(wxEvent& event);
  void OnCheckBox(wxEvent& event);
  void OnOK(wxEvent& event);
  void OnCancel(wxEvent& event);

private:
  std::optional<AudioOutputDevice> selectedOutputDevice() const;
};
#endif
