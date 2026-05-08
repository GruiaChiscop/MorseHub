#ifndef MAINWINDOW_H
#define MAINWINDOW_H
/* the main window class
 * Currently I will just try to port the app from .net, without any menu bars or further optimization
 */
#include <wx/wx.h>
#include "User.h"
#include "Utilities.h"
// IDS
enum MenuID
{
    ID_IMPORT = wxID_HIGHEST + 1,
    ID_EXPORT,
    ID_RUFZ,
    ID_TRAINER,
    ID_RUNNER,
    ID_RECEPTION,
    ID_TRANSMISION,
    ID_SETTINGS,
    ID_GITHUB,
    ID_DONATE,
    ID_UPDATE,
    ID_CONTACT,
    ID_WEBSITE,
    ID_FEEDBACK,
    ID_BETA,
    ID_JOINSERVER,
    ID_SCOREBOARD,
    ID_TRAINER_SETTINGS
};
class MainFrame : public wxFrame
{
    User user;

public:
    MainFrame(User &u) : wxFrame(NULL, wxID_ANY, "MorseHub main window")
    {
        user = u;
        wxMenuBar *menubar = new wxMenuBar;
        wxMenu *fileMenu = new wxMenu;
        wxMenu *optionsMenu = new wxMenu;
        fileMenu->Append(wxID_NEW, "&New user");
        fileMenu->Append(wxID_EDIT, "Edit &profile");
        fileMenu->Append(ID_IMPORT, "&Import an existing scoreboard file");
        fileMenu->Append(ID_EXPORT, "Export &scoreboard file");
        fileMenu->Append(ID_SCOREBOARD, "Show &session history");
        wxMenu *modeMenu = new wxMenu;
        modeMenu->Append(ID_RUFZ, "&RufzXP mode", "The RufzXP mode", wxITEM_CHECK);
        modeMenu->Append(ID_TRAINER, "&Trainer mode", "Generate practice groups with configurable symbols", wxITEM_CHECK);
        modeMenu->Append(ID_RUNNER, "Morse Runner mode", "The QSOs simulator mode", wxITEM_CHECK);
        modeMenu->Append(ID_RECEPTION, "Reception mode", "The zeus (HST2006) mode", wxITEM_CHECK);
        modeMenu->Append(ID_JOINSERVER, "Competition", "Join a competition");
        optionsMenu->AppendSubMenu(modeMenu, "&Mode");
        optionsMenu->Append(ID_SETTINGS, "MorseHub settings");

        wxMenu *helpMenu = new wxMenu;
        helpMenu->Append(wxID_ABOUT, "About MorseHub");
        helpMenu->Append(ID_GITHUB, "Open my github page");
        helpMenu->Append(ID_WEBSITE, "Open my website");
        helpMenu->Append(ID_DONATE, "Make a donation");
        helpMenu->Append(ID_FEEDBACK, "Give feedback");
        helpMenu->Append(ID_CONTACT, "Get in touch");
        helpMenu->Append(ID_UPDATE, "Check for updates");
        helpMenu->Append(ID_BETA, "Download the prereleases of MorseHub", "Help me improving this application by joining the beta and giving feedback");
        menubar->Append(fileMenu, "&Proffile");
        menubar->Append(optionsMenu, "&Options");
        menubar->Append(helpMenu, "&Help");
        SetMenuBar(menubar);
        applySelectedModeToMenu();
        wxPanel *panel = new wxPanel(this);
        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        wxStaticText *mainPageLabel = new wxStaticText(panel, wxID_ANY, "MorseHub start");
        sizer->Add(mainPageLabel, 0, wxALL | wxCENTER, 10);
        wxButton *startBTN = new wxButton(panel, wxID_ANY, "Start");
        sizer->Add(startBTN, 0, wxALL | wxCENTER, 10);
        wxStaticText *volumeLabel = new wxStaticText(panel, wxID_ANY, "Global volume");
        sizer->Add(volumeLabel, 0, wxALL, 5);
        wxSlider *volumeSlider = new wxSlider(panel, wxID_ANY, 50, 0, 100);
        volumeSlider->SetValue(user.outputVolumePercent);
        sizer->Add(volumeSlider, 0, wxALL | wxEXPAND, 10);
        wxButton *exitBTN = new wxButton(panel, wxID_ANY, "Exit");
        sizer->Add(exitBTN, 0, wxALL | wxCENTER, 10);
        panel->SetSizer(sizer);
        Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
        Bind(wxEVT_MENU, &MainFrame::OnNewUser, this, wxID_NEW);
        Bind(wxEVT_MENU, &MainFrame::OnEditUser, this, wxID_EDIT);
        Bind(wxEVT_MENU, &MainFrame::OnImport, this, ID_IMPORT);
        Bind(wxEVT_MENU, &MainFrame::OnExport, this, ID_EXPORT);
        Bind(wxEVT_MENU, &MainFrame::OnScoreboard, this, ID_SCOREBOARD);
        Bind(wxEVT_MENU, &MainFrame::OnModeChanged, this, ID_RUFZ);
        Bind(wxEVT_MENU, &MainFrame::OnModeChanged, this, ID_TRAINER);
        Bind(wxEVT_MENU, &MainFrame::OnModeChanged, this, ID_RUNNER);
        Bind(wxEVT_MENU, &MainFrame::OnModeChanged, this, ID_RECEPTION);
        Bind(wxEVT_MENU, &MainFrame::OnGithub, this, ID_GITHUB);
        Bind(wxEVT_MENU, &MainFrame::OnContact, this, ID_CONTACT);
        Bind(wxEVT_MENU, &MainFrame::OnGoToWebsite, this, ID_WEBSITE);
        Bind(wxEVT_MENU, &MainFrame::OnSettings, this, ID_SETTINGS);
        Bind(wxEVT_MENU, &MainFrame::OnSettings, this, ID_TRAINER_SETTINGS);
        Bind(wxEVT_MENU, &MainFrame::OnUpdate, this, ID_UPDATE);
        startBTN->Bind(wxEVT_BUTTON, &MainFrame::OnStart, this);
        exitBTN->Bind(wxEVT_BUTTON, &MainFrame::OnExit, this);
        volumeSlider->Bind(wxEVT_SLIDER, &MainFrame::OnSliderValueChanged, this);
        this->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);
    }

private:
    void applySelectedModeToMenu();
    void persistSelectedMode(MainMode mode);
    void OnClose(wxCloseEvent &event);
    void OnStart(wxEvent &event);
    void OnEditUser(wxEvent &event);
    void OnNewUser(wxEvent &event);
    void OnExport(wxEvent &event);
    void OnImport(wxEvent &event);
    void OnScoreboard(wxEvent &event);
    void OnGithub(wxEvent &event);
    void OnAbout(wxEvent &event);
    void OnContact(wxEvent &event);
    void OnGoToWebsite(wxEvent &event);
    void OnUpdate(wxEvent &event);
    void OnSettings(wxEvent &event);
    void OnSliderValueChanged(wxEvent &event);
    void OnModeChanged(wxEvent &event);
    void OnExit(wxEvent& event);
};
#endif
