#include <wx/wx.h>
#include "MorseHub.h"
#include "WelcomeWindow.h"
#include "User.h"
#include "MainWindow.h"
bool MorseHubApp::OnInit()
{
	//the initialization logic will come here
	User u;

	User* loadedUser = deserialize();
	if(loadedUser)
	{
		u = *loadedUser;
		//we can directly show the main window here
		MainFrame* f = new MainFrame(u);
		f->Show();
		return true;
	}
else
{
		WelcomeWindow w(u);
	w.ShowModal();
	return true;
	}
}
wxIMPLEMENT_APP(MorseHubApp);