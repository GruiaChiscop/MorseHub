#include <wx/wx.h>
#include "MorseHub.h"
#include "WelcomeWindow.h"
#include "User.h"
#include "MainWindow.h"
#include "Utilities.h"
#include <memory>

bool MorseHubApp::OnInit()
{
	User u;
Utils::currentWD = Utils::getWD();
Utils::appDataPath = Utils::getAppData();

	try
	{
		deserialize(u);
		Utils::morseGenerator  = std::make_unique<MorseGenerator>(u.defaultSpeed, u.defaultPitch, u.signalType);
			MainFrame* f = new MainFrame(u);
SetTopWindow(f);
		f->Show();
		return true;
	}
catch(const std::exception& e)
{
		WelcomeWindow w(u);
	w.ShowModal();
	return true;
	}
}

wxIMPLEMENT_APP(MorseHubApp);			