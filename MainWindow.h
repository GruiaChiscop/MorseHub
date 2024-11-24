#ifndef MAINWINDOW_H
#define MAINWINDOW_H
/* the main window class
* Currently I will just try to port the app from .net, without any menu bars or further optimization
*/
#include <wx/wx.h>
#include <wx/menu.h>
#include "User.h"

//IDS
#define ID_IMPORT 6000
#define ID_EXPORT 6001
#define ID_RUFZ 6002
#define ID_RUNNER 6003
#define ID_RECEPTION 6004

class MainFrame : public wxFrame {
User user;
public:
MainFrame(User& u) : wxFrame(NULL, wxID_ANY, "Main window") {
    wxMenuBar* menubar = new wxMenuBar;
    wxMenu* fileMenu = new wxMenu;
    wxMenu* optionsMenu = new wxMenu;
 fileMenu->Append(wxID_NEW, "&New user");
 fileMenu->Append(wxID_EDIT, "Edit &profile");
 fileMenu->Append(ID_IMPORT, "&Import an existing scoreboard file");
 fileMenu->Append(ID_EXPORT, "Export &scoreboard file");
 wxMenu* modeMenu = new wxMenu;
 modeMenu->Append(ID_RUFZ, "&RufzXP mode", "The RufzXP mode");
 modeMenu->Append(ID_RUNNER, "Morse Runner mode", "The QSOs simulator mode");
 modeMenu->Append(ID_RECEPTION, "Reception mode", "The zeus (HST2006) mode");
 optionsMenu->AppendSubMenu(modeMenu, "&Mode");
 optionsMenu->Append(wxID_ANY, "MorseHub settings");
 wxMenu* helpMenu = new wxMenu;
 helpMenu->Append(wxID_ANY, "About MorseHub");
 helpMenu->Append(wxID_ANY, "Open my github page");
 helpMenu->Append(wxID_ANY, "Open my website");
 helpMenu->Append(wxID_ANY, "Make a donation");
 helpMenu->Append(wxID_ANY, "Give feedback");
 helpMenu->Append(wxID_ANY, "Contact me");
 helpMenu->Append(wxID_ANY, "Check for updates");
helpMenu->Append(wxID_ANY, "Download the beta versions of MorseHub");
menubar->Append(fileMenu, "&Proffile");
menubar->Append(optionsMenu, "&Options");
menubar->Append(helpMenu, "&Help");
SetMenuBar(menubar);
wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
wxStaticText* mainPageLabel = new wxStaticText(this, wxID_ANY, "MorseHub start");
sizer->Add(mainPageLabel, 0, wxALL, 5);
 wxButton* startBTN = new wxButton(this, wxID_ANY, "Start");
 sizer->Add(startBTN, 0, wxALL | wxCenter, 10);
 wxButton* exitBTN = new wxButton(this, wxID_ANY, "Exit");
}
};
#endif