#include <wx/wx.h>
#include "MorseHub.h"
#include "WelcomeWindow.h"
#include "User.h"
#include "MainWindow.h"
#include "AudioOutputDevice.h"
#include "Utilities.h"
#include <memory>
#include <optional>
#include <ctime>

bool MorseHubApp::OnInit()
{
	User u;
Utils::currentWD = Utils::getWD();
Utils::appDataPath = Utils::getAppData();
std::srand(static_cast<unsigned int>(std::time(nullptr)));
	try
	{
		deserialize(u);
		Utils::configureGlobalMorseGenerator(u);
		MainFrame* f = new MainFrame(u);
		f->Show();
SetTopWindow(f);
		return true;
	}
catch(const std::exception& e)
{
		WelcomeWindow w(nullptr, u, true);
	w.ShowModal();
	return true;
	}
}

wxIMPLEMENT_APP(MorseHubApp);
