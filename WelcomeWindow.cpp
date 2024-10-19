#include "WelcomeWindow.h"
#include "SineWaveGenerator.h"
void WelcomeWindow::OnClose(wxCloseEvent& event)
{
	wxMessageBox("you must click finish to close this window", "Can't close the dialog", wxICON_ERROR | wxOK);
}
void WelcomeWindow::OnTest(wxEvent& event)
{

}