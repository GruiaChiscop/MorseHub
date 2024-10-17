#include "WelcomeWindow.h"
void WelcomeWindow::OnClose(wxCloseEvent& event)
{
	wxMessageBox("you must click finish to close this window", "Can't close the dialog", wxICON_ERROR | wxOK);
}