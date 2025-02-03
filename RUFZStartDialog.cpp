#include "RUFZStartDialog.h"
#include "MorseGenerator.h"
#include <memory>

void RUFZStartDialog::OnTest(wxEvent &event)
{
    gen = std::make_unique<MorseGenerator>(user.defaultSpeed, user.defaultPitch, user.signalType);
    gen->transmitAsync("vvv= = +");
}
void RUFZStartDialog::OnCheckBox(wxEvent &event)
{
    if (useExistentCheckBox->IsChecked())
    {
        cbxSpeed->Disable();
        cbxPitch->Disable();
        cbxSignalType->Disable();
    }
    else
    {
        cbxSpeed->Enable();
        cbxPitch->Enable();
        cbxSignalType->Enable();
    }
}
void RUFZStartDialog::OnOK(wxEvent &event)
{
    cbxSpeed->GetValue().ToInt(&user.defaultSpeed);
    cbxPitch->GetValue().ToInt(&user.defaultPitch);
    wxString signal = cbxSignalType->GetValue();
    if (signal == "Sine")
        user.signalType = Sine;
    else if (signal == "Square")
        user.signalType = Square;
    else if (signal == "Triangle")
        user.signalType = Triangle;
    else if(signal=="Sawtooth")
        user.signalType = Sawtooth;
        else
        user.signalType = Sine;
        EndModal(wxOK);
}
void RUFZStartDialog::OnCancel(wxEvent& event)
{
    EndModal(wxCANCEL);
}