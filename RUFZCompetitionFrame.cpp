#include "RUFZCompetitionFrame.h"
#include "MorseGenerator.h"
#include "Utilities.h"
#include <string>
#include "Result.h"
#include "Scoreboard.h"

using std::string;

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
    if(event.GetKeyCode()==WXK_RETURN)
    {
evaluate();
    }
}
void RUFZCompetitionFrame::OnExit(wxEvent& event)
{
    finish();
}

void RUFZCompetitionFrame::evaluate()
{
if(rounds==user.maxRounds) finish();
else
{
string typedText = editField->GetValue().ToStdString();
Result r;
r.frequency = m_pitch;
r.speed = m_speed;
r.typedText = typedText;
r.text = m_text;
//get points
}
}