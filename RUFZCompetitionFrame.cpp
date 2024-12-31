#include "RUFZCompetitionFrame.h"
#include "MorseGenerator.h"
#include "Utilities.h"
#include <string>

void RUFZCompetitionFrame::play()
{
    m_pitch = Utils::randomInteger(600, 1000);
    generator->setFrequency(m_pitch);
    m_text = strgen.generate(5);
    generator->transmitAsync(m_text);
}

void RUFZCompetitionFrame::play(const std::string& text)
{
generator->transmitAsync(text);
}
void RUFZCompetitionFrame::finish()
{

}
void RUFZCompetitionFrame::OnAppear(wxEvent& event)
{
    play();
    event.Skip();
}
void RUFZCompetitionFrame::OnKeyPress(wxKeyEvent& event)
{
    if(event.GetKeyCode() == WXK_F6 && !played) play(m_text);
}
void RUFZCompetitionFrame::OnExit(wxEvent& event)
{
    finish();
}