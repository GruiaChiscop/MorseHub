#pragma once

#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/stattext.h>
#include <wx/timer.h>

#include <set>
#include <vector>

#include "ReceptionParser.h"
#include "ReceptionTypes.h"
#include "SessionHistory.h"
#include "User.h"

class ReceptionResultsDialog : public wxDialog
{
public:
    ReceptionResultsDialog(wxWindow* parent, const User& user, const std::vector<ReceptionSessionRow>& rows);

private:
    const User& user;
    std::vector<ReceptionSessionRow> rows;
    wxGrid* grid{};
    wxStaticText* countdownLabel{};
    wxTimer countdownTimer;
    int secondsRemaining{};
    int maxGroupCount{};
    std::set<int> announcedThresholds;

    void populateGrid();
    void refreshRow(int row);
    void refreshCountdownLabel();
    void submit(bool dueToTimeout);
    char separator() const;
    std::vector<std::string> rowGroupsFromGrid(int row) const;
    int firstGroupColumn() const;
    int errorsColumn() const;
    int countedColumn() const;

    void OnCellChanged(wxGridEvent& event);
    void OnSubmit(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
};
