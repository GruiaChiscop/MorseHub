#include <wx/wx.h>
#include "MorseHub.h"
#include "WelcomeWindow.h"
#include "User.h"
bool MorseHubApp::OnInit()
{
	//the initialization logic will come here
	User u;
	WelcomeWindow w(u);
	w.ShowModal();
	return true;
}
wxIMPLEMENT_APP(MorseHubApp);