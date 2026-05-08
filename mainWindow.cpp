#include "MainWindow.h"
#include "MorseHubSettingsDialog.h"
#include "SessionDetailsDialog.h"
#include <wx/listctrl.h>
#include "Utilities.h"
#include "User.h"
#include "RUFZStartDialog.h"
#include "RUFZCompetitionFrame.h"
#include "ReceptionCompetitionFrame.h"
#include "ReceptionSettingsDialog.h"
#include "RunnerCompetitionFrame.h"
#include "RunnerStartDialog.h"
#include "SessionHistory.h"
#include "TrainerCompetitionFrame.h"
#include "SignalGenerator.h"
#include "WelcomeWindow.h"

namespace
{
class SessionHistoryDialog : public wxDialog
{
public:
    SessionHistoryDialog(wxWindow* parent, const User& user)
        : wxDialog(parent, wxID_ANY, "Session history", wxDefaultPosition, wxSize(900, 520)), user(user), sessions(SessionHistoryStore::load())
    {
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
        list->AppendColumn("Started", wxLIST_FORMAT_LEFT, 170);
        list->AppendColumn("Mode", wxLIST_FORMAT_LEFT, 120);
        list->AppendColumn("Rounds", wxLIST_FORMAT_RIGHT, 80);
        list->AppendColumn("Points", wxLIST_FORMAT_RIGHT, 90);
        list->AppendColumn("Errors", wxLIST_FORMAT_RIGHT, 90);
        list->AppendColumn("Max speed", wxLIST_FORMAT_LEFT, 110);
        list->AppendColumn("Repeats", wxLIST_FORMAT_RIGHT, 80);
        sizer->Add(list, 1, wxALL | wxEXPAND, 10);

        emptyLabel = new wxStaticText(this, wxID_ANY, sessions.empty() ? "No session history has been recorded yet." : "");
        sizer->Add(emptyLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

        sizer->Add(new wxButton(this, wxID_OK, "Close"), 0, wxALL | wxALIGN_CENTER, 10);
        SetSizer(sizer);

        list->Bind(wxEVT_LIST_COL_CLICK, &SessionHistoryDialog::OnColumnClicked, this);
        list->Bind(wxEVT_LIST_ITEM_ACTIVATED, &SessionHistoryDialog::OnItemActivated, this);
        populateRows();
    }

private:
    const User& user;
    std::vector<SessionRecord> sessions;
    wxListCtrl* list{};
    wxStaticText* emptyLabel{};
    std::vector<std::size_t> visibleOrder;
    int sortColumn{0};
    bool sortAscending{false};

    wxString speedLabel(int cpm) const
    {
        return Utils::formatSpeed(cpm, user.speedDisplayMode);
    }

    void populateRows()
    {
        list->DeleteAllItems();
        emptyLabel->SetLabel(sessions.empty() ? "No session history has been recorded yet." : "");
        if (sessions.empty())
            return;

        std::vector<std::size_t> order(sessions.size());
        for (std::size_t i = 0; i < sessions.size(); ++i)
            order[i] = i;

        auto lessThan = [this](const SessionRecord& left, const SessionRecord& right) {
            switch (sortColumn)
            {
            case 0:
                return left.startedAt < right.startedAt;
            case 1:
                return left.mode < right.mode;
            case 2:
                return left.rounds < right.rounds;
            case 3:
                return left.totalPoints < right.totalPoints;
            case 4:
                return left.totalErrors < right.totalErrors;
            case 5:
                return left.maxSpeed < right.maxSpeed;
            case 6:
                return left.repeatCount < right.repeatCount;
            default:
                return left.startedAt < right.startedAt;
            }
        };

        std::sort(order.begin(), order.end(), [&](std::size_t leftIndex, std::size_t rightIndex) {
            const SessionRecord& left = sessions[leftIndex];
            const SessionRecord& right = sessions[rightIndex];
            if (lessThan(left, right))
                return sortAscending;
            if (lessThan(right, left))
                return !sortAscending;
            return left.startedAt > right.startedAt;
        });

        visibleOrder = order;
        long rowIndex = 0;
        for (std::size_t entryIndex : visibleOrder)
        {
            const SessionRecord& session = sessions[entryIndex];
            rowIndex = list->InsertItem(rowIndex, wxString::FromUTF8(session.startedAt));
            list->SetItem(rowIndex, 1, wxString::FromUTF8(session.mode));
            list->SetItem(rowIndex, 2, wxString::Format("%d", session.rounds));
            list->SetItem(rowIndex, 3, wxString::Format("%d", session.totalPoints));
            list->SetItem(rowIndex, 4, wxString::Format("%d", session.totalErrors));
            list->SetItem(rowIndex, 5, speedLabel(session.maxSpeed));
            list->SetItem(rowIndex, 6, wxString::Format("%d", session.repeatCount));
            ++rowIndex;
        }
    }

    void OnColumnClicked(wxListEvent& event)
    {
        const int clickedColumn = event.GetColumn();
        if (sortColumn == clickedColumn)
            sortAscending = !sortAscending;
        else
        {
            sortColumn = clickedColumn;
            sortAscending = clickedColumn != 0;
        }
        populateRows();
    }

    void OnItemActivated(wxListEvent& event)
    {
        const long row = event.GetIndex();
        if (row < 0 || row >= static_cast<long>(visibleOrder.size()))
            return;

        SessionDetailsDialog dialog(this, user, sessions[visibleOrder[static_cast<std::size_t>(row)]]);
        dialog.ShowModal();
    }
};

void openUrl(wxWindow* parent, const wxString& url)
{
    if (!wxLaunchDefaultBrowser(url))
        wxMessageBox("Could not open the link in your default browser.", "Open link failed", wxOK | wxICON_ERROR, parent);
}

void showSessionFrame(MainFrame* owner, wxFrame* frame)
{
    owner->Hide();
    frame->Bind(wxEVT_DESTROY, [owner](wxWindowDestroyEvent& event) {
        owner->Show();
        owner->Raise();
        event.Skip();
    });
    frame->Show();
}
}

void MainFrame::OnClose(wxCloseEvent &event)
{
    serialize(user);        // in case it hasn't already serialized and there's unsaved data
    Utils::say("73", true); // We must wait untill the messsage finishes, otherwise nothing would be heared
    event.Skip();
}

void MainFrame::applySelectedModeToMenu()
{
    wxMenuBar* menu = GetMenuBar();
    if (!menu)
        return;

    menu->FindItem(ID_RUFZ)->Check(user.lastSelectedMode == MainMode::Rufz);
    menu->FindItem(ID_TRAINER)->Check(user.lastSelectedMode == MainMode::Trainer);
    menu->FindItem(ID_RUNNER)->Check(user.lastSelectedMode == MainMode::Runner);
    menu->FindItem(ID_RECEPTION)->Check(user.lastSelectedMode == MainMode::Reception);
}

void MainFrame::persistSelectedMode(MainMode mode)
{
    user.lastSelectedMode = mode;
    applySelectedModeToMenu();
    serialize(user);
}

void MainFrame::OnModeChanged(wxEvent &event)
{
    wxMenuBar *menu = GetMenuBar();
    if (event.GetId() == ID_RUFZ)
    {
        persistSelectedMode(MainMode::Rufz);
    }
    else if (event.GetId() == ID_TRAINER)
    {
        persistSelectedMode(MainMode::Trainer);
    }
    else if (event.GetId() == ID_RUNNER)
    {
        persistSelectedMode(MainMode::Runner);
    }
    else if (event.GetId() == ID_RECEPTION)
    {
        persistSelectedMode(MainMode::Reception);
    }
    else if(event.GetId() == ID_JOINSERVER)
    {
        //we uncheck everything if already checked, the client can become a server, or, if connected, it is instructed to do various things.
        menu->FindItem(ID_RUNNER)->Check(false);
        menu->FindItem(ID_RECEPTION)->Check(false);
        menu->FindItem(ID_RUFZ)->Check(false);
        menu->FindItem(ID_TRAINER)->Check(false);
        user.lastSelectedMode = MainMode::None;
        serialize(user);
//show the server conection dialog
    }
}
void MainFrame::OnStart(wxEvent &event)
{
    RUFZStartDialog r(this, user);
    // morseRunnerStartDialog m(user);
    // ReceptionStartDialog rx(user);
    wxMenuBar* menu = GetMenuBar();
    if(menu->FindItem(ID_RUFZ)->IsChecked())
    {
        int res = r.ShowModal();
        if(res==wxOK)
        {
            serialize(user);
            Utils::configureGlobalMorseGenerator(user);
            auto* competition = new RUFZCompetitionFrame(this, user);
            showSessionFrame(this, competition);
        }
    }
    else if(menu->FindItem(ID_TRAINER)->IsChecked())
    {
        auto* trainer = new TrainerCompetitionFrame(this, user);
        showSessionFrame(this, trainer);
    }
    else if(menu->FindItem(ID_RUNNER)->IsChecked())
    {
        RunnerStartDialog runnerDialog(this, user);
        if (runnerDialog.ShowModal() == wxOK)
        {
            serialize(user);
            Utils::configureGlobalMorseGenerator(user);
            auto* runner = new RunnerCompetitionFrame(this, user);
            showSessionFrame(this, runner);
        }
    }
    else if(menu->FindItem(ID_RECEPTION)->IsChecked())
    {
        ReceptionSettingsDialog settingsDialog(this, user);
        if (settingsDialog.ShowModal() == wxOK)
        {
            serialize(user);
            auto* reception = new ReceptionCompetitionFrame(this, user);
            showSessionFrame(this, reception);
        }
    }
    else
    {
        wxMessageBox("You must select the mode first", "No mode selected", wxOK | wxICON_ERROR);
    }
}
void MainFrame::OnEditUser(wxEvent &event)
{
    WelcomeWindow dialog(this, user, false, "Edit profile", "Update your MorseHub profile and audio settings", "Save");
    if (dialog.ShowModal() == wxOK)
    {
        serialize(user);
        Utils::configureGlobalMorseGenerator(user);
    }
}
void MainFrame::OnNewUser(wxEvent &event)
{
    if (wxMessageBox("Create a new profile? Your current profile will be replaced.", "New profile", wxYES_NO | wxICON_QUESTION, this) != wxYES)
        return;

    User newUser;
    WelcomeWindow dialog(this, newUser, false, "Create profile", "Set up a new MorseHub profile", "Save");
    if (dialog.ShowModal() == wxOK)
    {
        user = newUser;
        serialize(user);
        Utils::configureGlobalMorseGenerator(user);
    }
}
void MainFrame::OnExport(wxEvent &event)
{
    wxFileDialog dialog(this, "Export session history", "", "MorseHubHistory.json", "JSON files (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dialog.ShowModal() != wxID_OK)
        return;

    try
    {
        SessionHistoryStore::exportTo(dialog.GetPath().ToStdString());
        wxMessageBox("Session history exported successfully.", "Export complete", wxOK | wxICON_INFORMATION, this);
    }
    catch (const std::exception& e)
    {
        wxMessageBox(wxString::Format("Could not export session history:\n%s", e.what()), "Export failed", wxOK | wxICON_ERROR, this);
    }
}
void MainFrame::OnImport(wxEvent &event)
{
    wxFileDialog dialog(this, "Import session history", "", "", "JSON files (*.json)|*.json", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() != wxID_OK)
        return;

    try
    {
        SessionHistoryStore::importFrom(dialog.GetPath().ToStdString());
        wxMessageBox("Session history imported successfully.", "Import complete", wxOK | wxICON_INFORMATION, this);
    }
    catch (const std::exception& e)
    {
        wxMessageBox(wxString::Format("Could not import session history:\n%s", e.what()), "Import failed", wxOK | wxICON_ERROR, this);
    }
}
void MainFrame::OnScoreboard(wxEvent &event)
{
    SessionHistoryDialog dialog(this, user);
    dialog.ShowModal();
}
void MainFrame::OnGithub(wxEvent &event)
{
    openUrl(this, "https://github.com/christosb/MorseHub");
}
void MainFrame::OnAbout(wxEvent &event)
{
    wxMessageBox(
        "MorseHub\n\nA desktop application for Morse/HST training.\nThe current milestone focuses on a solid RufzXP-style foundation before RX and Morse Runner.",
        "About MorseHub",
        wxOK | wxICON_INFORMATION,
        this);
}
void MainFrame::OnContact(wxEvent &event)
{
    openUrl(this, "mailto:christosb@users.noreply.github.com");
}
void MainFrame::OnGoToWebsite(wxEvent &event)
{
    openUrl(this, "https://rufzxp.net/");
}
void MainFrame::OnUpdate(wxEvent &event)
{
    wxMessageBox(
        "The updater is intentionally postponed until the main training modes are in place.",
        "Updates postponed",
        wxOK | wxICON_INFORMATION,
        this);
}
void MainFrame::OnSettings(wxEvent &event)
{
    if (event.GetId() == ID_TRAINER_SETTINGS)
    {
        MorseHubSettingsDialog dialog(this, user, 2);
        if (dialog.ShowModal() == wxOK)
        {
            serialize(user);
            Utils::configureGlobalMorseGenerator(user);
        }
        return;
    }

    MorseHubSettingsDialog dialog(this, user);
    if (dialog.ShowModal() == wxOK)
    {
        serialize(user);
        Utils::configureGlobalMorseGenerator(user);
    }
}
void MainFrame::OnSliderValueChanged(wxEvent &event)
{
    wxSlider* slider = dynamic_cast<wxSlider*>(event.GetEventObject());
    if (!slider)
        return;

    user.outputVolumePercent = slider->GetValue();
    serialize(user);
}
void MainFrame::OnExit(wxEvent &event)
{
    Close();
}
