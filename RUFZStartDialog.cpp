#include "RUFZStartDialog.h"
#include "MorseGenerator.h"
#include <memory>

void RUFZStartDialog::OnTest(wxEvent& event)
{
gen = std::make_unique<MorseGenerator>(m_speed, m_pitch, m_type);
gen->transmitAsync("vvv= = +");
}
void RUFZStartDialog::OnCheckBox(wxEvent& event)
{
    if(useExistentCheckBox->IsChecked())
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
