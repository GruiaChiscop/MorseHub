#include "RUFZStartDialog.h"
#include "MorseGenerator.h"
#include <memory>

void RUFZStartDialog::OnTest(wxEvent &event)
{
    gen = std::make_unique<MorseGenerator>(m_speed, m_frequency, m_type);
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
    cbxSpeed->GetValue().ToInt(&m_speed);
    cbxPitch->GetValue().ToInt(&m_frequency);
    wxString signal = cbxSignalType->GetValue();
    SType type;
    if (signal == "Sine")
        type = Sine;
    else if (signal == "Square")
        type = Square;
    else if (signal == "Triangle")
        type = Triangle;
    else
        type = Sawtooth;
        EndModal(wxOK);
}
void RUFZStartDialog::OnCancel(wxEvent& event)
{
    EndModal(wxCANCEL);
}