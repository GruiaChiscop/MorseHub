#include "MainWindow.h"
#include "Utilities.h"
#include "User.h"
void MainFrame::OnClose(wxEvent &event)
{
    serialize(user);        // in case it hasn't already serialized and there's unsaved data
    Utils::say("73", true); // We must wait untill the messsage finishes, otherwise nothing would be heared
    event.Skip();
}
void MainFrame::OnModeChanged(wxEvent &event)
{
    wxMenuBar *menu = GetMenuBar();
    if (event.GetId() == ID_RUFZ)
    {
        // we uncheck the others in case if they're already checked
        menu->FindItem(ID_RUNNER)->Check(false);
        menu->FindItem(ID_RECEPTION)->Check(false);
    }
    else if (event.GetId() == ID_RUNNER)
    {
        menu->FindItem(ID_RUFZ)->Check(false);
        menu->FindItem(ID_RECEPTION)->Check(false);
    }
    else if (event.GetId() == ID_RECEPTION)
    {
        menu->FindItem(ID_RUFZ)->Check(false);
        menu->FindItem(ID_RUNNER)->Check(false);
    }
    else /*ID_JOINSERVER*/
    {
        //we uncheck everything if already checked, the client can become a server, or, if connected, it is instructed to do various things.
        menu->FindItem(ID_RUNNER)->Check(false);
        menu->FindItem(ID_RECEPTION)->Check(false);
        menu->FindItem(ID_RUFZ)->Check(false);
//show the server conection dialog
    }
}
void MainFrame::OnStart(wxEvent &event)
{
    // RUFZStartDialog r(user);
    // morseRunnerStartDialog m(user);
    // ReceptionStartDialog rx(user);
    wxMenuBar* menu = GetMenuBar();
    if(menu->FindItem(ID_RUFZ)->IsChecked())
    {
        //r.ShowModal();
    }
    else if(menu->FindItem(ID_RUNNER)->IsChecked())
    {
        //m.ShowModal();
    }
    else if(menu->FindItem(ID_PSRESTARTWINDOWS)->IsChecked())
    {
        //rx.ShowModal();
    }
    else
    {
        wxMessageBox("You must select the mode first", "No mode selected", wxOK | wxICON_ERROR);
    }
}
void MainFrame::OnEditUser(wxEvent &event)
{
}
void MainFrame::OnNewUser(wxEvent &event)
{
}
void MainFrame::OnExport(wxEvent &event)
{
}
void MainFrame::OnImport(wxEvent &event)
{
}
void MainFrame::OnGithub(wxEvent &event)
{
}
void MainFrame::OnAbout(wxEvent &event)
{
}
void MainFrame::OnContact(wxEvent &event)
{
}
void MainFrame::OnGoToWebsite(wxEvent &event)
{
}
void MainFrame::OnUpdate(wxEvent &event)
{
}
void MainFrame::OnSettings(wxEvent &event)
{
}
void MainFrame::OnSliderValueChanged(wxEvent &event)
{
}
void MainFrame::OnExit(wxEvent &event)
{
    Close();
}