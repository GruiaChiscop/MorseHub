#ifndef MAINWINDOW_H
#define MAINWINDOW_H
/* the main window class
* Currently I will just try to port the app from .net, without any menu bars or further optimization
*/
#include <wx/wx.h>
#include <wx/menu.h>
#include "User.h"

class MainFrame : public wxFrame {
User user;
public:
MainFrame(User& u) : wxFrame(NULL, wxID_ANY, "Main window") {
    wxMenuBar* menubar = new wxMenuBar;
    wxMenu* fileMenu = new wxMenu;
    wxMenu* optionsMenu = new wxMenu;
 fileMenu->Append(wxID_ANY, "&New user");
 fileMenu->Append(wxID_ANY, "Edit &profile");
 fileMenu->Append(wxID_ANY, "I&mport an existing scoreboard file");
 fileMenu->Append(wxID_ANY, "Export &scoreboard file");
 wxButton* startBTN = new wxButton(NULL, wxID_ANY, "Start");
 wxButton* exitBTN = new wxButton(NULL, wxID_ANY, "Exit");
}
};
#endif