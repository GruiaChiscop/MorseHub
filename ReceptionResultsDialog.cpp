#include "ReceptionResultsDialog.h"

#include <algorithm>

#include <wx/button.h>
#include <wx/colour.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

namespace
{
std::string separatorForDisplay(const std::string& separator)
{
    return separator.empty() ? ";" : separator;
}

std::string joinGroups(const std::vector<std::string>& groups, char separator)
{
    std::string result;
    for (std::size_t i = 0; i < groups.size(); ++i)
    {
        if (i != 0)
            result += separator;
        result += groups[i];
    }
    return result;
}
}

ReceptionResultsDialog::ReceptionResultsDialog(wxWindow* parent, const User& value, const std::vector<ReceptionSessionRow>& sessionRows)
    : wxDialog(parent, wxID_ANY, "Reception results"), user(value), rows(sessionRows), countdownTimer(this), secondsRemaining(value.moduleSettings.reception.resultEntryMinutes * 60)
{
    for (const ReceptionSessionRow& row : rows)
        maxGroupCount = std::max(maxGroupCount, static_cast<int>(row.sentGroups.size()));

    const std::string configuredSeparator = separatorForDisplay(user.moduleSettings.reception.groupSeparator);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(
        new wxStaticText(
            this,
            wxID_ANY,
            wxString::Format(
                "Review and correct the copied groups. Separate groups with '%s'. Rows with incomplete groups are shown in red and underlined.",
                configuredSeparator.c_str())),
        0,
        wxALL,
        10);

    countdownLabel = new wxStaticText(this, wxID_ANY, "");
    sizer->Add(countdownLabel, 0, wxALL, 10);

    grid = new wxGrid(this, wxID_ANY);
    grid->CreateGrid(static_cast<int>(rows.size()), 1 + maxGroupCount + 2);
    grid->SetColLabelValue(0, "Speed");
    for (int index = 0; index < maxGroupCount; ++index)
        grid->SetColLabelValue(firstGroupColumn() + index, wxString::Format("G%d", index + 1));
    grid->SetColLabelValue(errorsColumn(), "Errors");
    grid->SetColLabelValue(countedColumn(), "Counted");
    grid->EnableEditing(true);
    sizer->Add(grid, 1, wxALL | wxEXPAND, 10);

    wxBoxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    auto* submitButton = new wxButton(this, wxID_OK, "Submit results");
    auto* cancelButton = new wxButton(this, wxID_CANCEL, "Close");
    buttons->Add(submitButton, 0, wxALL, 5);
    buttons->Add(cancelButton, 0, wxALL, 5);
    sizer->Add(buttons, 0, wxALIGN_CENTER);

    submitButton->Bind(wxEVT_BUTTON, &ReceptionResultsDialog::OnSubmit, this);
    cancelButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { EndModal(wxCANCEL); });
    grid->Bind(wxEVT_GRID_CELL_CHANGED, &ReceptionResultsDialog::OnCellChanged, this);
    Bind(wxEVT_TIMER, &ReceptionResultsDialog::OnTimer, this);

    SetSizerAndFit(sizer);
    SetSize(wxSize(980, 520));

    populateGrid();
    refreshCountdownLabel();
    countdownTimer.Start(1000);
}

void ReceptionResultsDialog::populateGrid()
{
    for (int row = 0; row < static_cast<int>(rows.size()); ++row)
    {
        grid->SetReadOnly(row, 0, true);
        grid->SetReadOnly(row, errorsColumn(), true);
        grid->SetReadOnly(row, countedColumn(), true);

        grid->SetCellValue(row, 0, wxString::Format("%d", rows[row].speedCpm));
        const ReceptionParsedEntry parsed = ReceptionParser::parseGroups(rows[row].enteredGroups, separator());
        for (int column = 0; column < maxGroupCount; ++column)
        {
            const int gridColumn = firstGroupColumn() + column;
            if (column < static_cast<int>(rows[row].sentGroups.size()))
            {
                grid->SetReadOnly(row, gridColumn, false);
                if (column < static_cast<int>(parsed.groups.size()))
                    grid->SetCellValue(row, gridColumn, wxString::FromUTF8(parsed.groups[static_cast<std::size_t>(column)]));
            }
            else
            {
                grid->SetReadOnly(row, gridColumn, true);
                grid->SetCellBackgroundColour(row, gridColumn, wxColour(235, 235, 235));
            }
        }
        refreshRow(row);
    }
}

void ReceptionResultsDialog::refreshRow(int row)
{
    const std::vector<std::string> enteredGroups = rowGroupsFromGrid(row);
    rows[static_cast<std::size_t>(row)].enteredGroups = joinGroups(enteredGroups, separator());
    const ReceptionRowEvaluation evaluation = ReceptionParser::evaluateGroups(rows[static_cast<std::size_t>(row)].sentGroups, enteredGroups);

    grid->SetCellValue(row, errorsColumn(), wxString::Format("%d", evaluation.errorCount));
    grid->SetCellValue(row, countedColumn(), "Pending");

    for (int index = 0; index < maxGroupCount; ++index)
    {
        const int column = firstGroupColumn() + index;
        if (index >= static_cast<int>(rows[static_cast<std::size_t>(row)].sentGroups.size()))
            continue;

        const std::string normalized = index < static_cast<int>(evaluation.parsed.groups.size())
            ? evaluation.parsed.groups[static_cast<std::size_t>(index)]
            : std::string();
        if (grid->GetCellValue(row, column).ToStdString() != normalized)
            grid->SetCellValue(row, column, wxString::FromUTF8(normalized));

        const bool incomplete = normalized.size() != 5;
        wxFont font = grid->GetCellFont(row, column);
        font.SetUnderlined(incomplete);
        grid->SetCellFont(row, column, font);
        grid->SetCellTextColour(row, column, incomplete ? *wxRED : *wxBLACK);
    }
    grid->ForceRefresh();
}

void ReceptionResultsDialog::refreshCountdownLabel()
{
    countdownLabel->SetLabel(wxString::Format("Result entry time remaining: %d:%02d", secondsRemaining / 60, secondsRemaining % 60));
}

void ReceptionResultsDialog::submit(bool dueToTimeout)
{
    int incompleteCells = 0;
    std::vector<ReceptionRowEvaluation> evaluations;
    evaluations.reserve(rows.size());
    for (int row = 0; row < static_cast<int>(rows.size()); ++row)
    {
        refreshRow(row);
        ReceptionRowEvaluation evaluation = ReceptionParser::evaluateGroups(rows[static_cast<std::size_t>(row)].sentGroups, rowGroupsFromGrid(row));
        incompleteCells += evaluation.incompleteGroupCount;
        evaluations.push_back(evaluation);
    }

    if (!dueToTimeout)
    {
        wxString warning = "Are you sure you want to submit the reception results? This cannot be undone.";
        if (incompleteCells > 0)
            warning += wxString::Format("\nThere are %d incomplete group cells.", incompleteCells);
        if (wxMessageBox(warning, "Submit results", wxYES_NO | wxICON_WARNING, this) != wxYES)
            return;
    }

    std::vector<int> order(rows.size());
    for (int i = 0; i < static_cast<int>(rows.size()); ++i)
        order[static_cast<std::size_t>(i)] = i;

    std::sort(order.begin(), order.end(), [&evaluations, this](int left, int right) {
        if (evaluations[static_cast<std::size_t>(left)].errorCount != evaluations[static_cast<std::size_t>(right)].errorCount)
            return evaluations[static_cast<std::size_t>(left)].errorCount < evaluations[static_cast<std::size_t>(right)].errorCount;
        return rows[static_cast<std::size_t>(left)].speedCpm > rows[static_cast<std::size_t>(right)].speedCpm;
    });

    std::set<int> countedRows;
    for (std::size_t i = 0; i < order.size() && i < 3; ++i)
        countedRows.insert(order[i]);

    SessionRecord session;
    session.mode = "reception";
    session.startedAt = wxDateTime::Now().FormatISOCombined(' ').ToStdString();
    session.rounds = static_cast<int>(rows.size());

    for (int row = 0; row < static_cast<int>(rows.size()); ++row)
    {
        const bool counted = countedRows.count(row) != 0;
        grid->SetCellValue(row, countedColumn(), counted ? "Yes" : "No");

        Result result;
        result.text = joinGroups(rows[static_cast<std::size_t>(row)].sentGroups, separator());
        result.typedText = rows[static_cast<std::size_t>(row)].enteredGroups;
        result.points = evaluations[static_cast<std::size_t>(row)].correctGroupCount;
        result.speed = rows[static_cast<std::size_t>(row)].speedCpm;
        result.errorCount = evaluations[static_cast<std::size_t>(row)].errorCount;
        session.results.push_back(result);

        if (counted)
        {
            session.totalPoints += result.points;
            session.totalErrors += result.errorCount;
            session.maxSpeed = std::max(session.maxSpeed, result.speed);
        }
    }

    try
    {
        SessionHistoryStore::append(session);
    }
    catch (const std::exception& e)
    {
        wxMessageBox(wxString::Format("Could not save session history:\n%s", e.what()), "Save failed", wxOK | wxICON_ERROR, this);
    }

    wxMessageBox(
        wxString::Format("Reception results processed.\nCounted speeds: %d\nTotal errors: %d\nCorrect groups: %d", static_cast<int>(countedRows.size()), session.totalErrors, session.totalPoints),
        "Reception summary",
        wxOK | wxICON_INFORMATION,
        this);
    EndModal(wxOK);
}

char ReceptionResultsDialog::separator() const
{
    const std::string& separatorValue = user.moduleSettings.reception.groupSeparator;
    return separatorValue.empty() ? ';' : separatorValue.front();
}

std::vector<std::string> ReceptionResultsDialog::rowGroupsFromGrid(int row) const
{
    std::vector<std::string> groups;
    groups.reserve(rows[static_cast<std::size_t>(row)].sentGroups.size());
    for (int index = 0; index < static_cast<int>(rows[static_cast<std::size_t>(row)].sentGroups.size()); ++index)
    {
        groups.push_back(ReceptionParser::normalizeForCell(grid->GetCellValue(row, firstGroupColumn() + index).ToStdString()));
    }
    return groups;
}

int ReceptionResultsDialog::firstGroupColumn() const
{
    return 1;
}

int ReceptionResultsDialog::errorsColumn() const
{
    return firstGroupColumn() + maxGroupCount;
}

int ReceptionResultsDialog::countedColumn() const
{
    return errorsColumn() + 1;
}

void ReceptionResultsDialog::OnCellChanged(wxGridEvent& event)
{
    if (event.GetCol() >= firstGroupColumn() && event.GetCol() < errorsColumn())
        refreshRow(event.GetRow());
    event.Skip();
}

void ReceptionResultsDialog::OnSubmit(wxCommandEvent& event)
{
    (void)event;
    submit(false);
}

void ReceptionResultsDialog::OnTimer(wxTimerEvent& event)
{
    (void)event;
    if (secondsRemaining <= 0)
    {
        countdownTimer.Stop();
        submit(true);
        return;
    }

    --secondsRemaining;
    refreshCountdownLabel();

    static const int thresholds[] = {600, 300, 60, 30};
    for (int threshold : thresholds)
    {
        if (secondsRemaining == threshold && announcedThresholds.insert(threshold).second)
        {
            wxMessageBox(
                wxString::Format("Reception result entry time remaining: %d minute(s) %d second(s).", threshold / 60, threshold % 60),
                "Time warning",
                wxOK | wxICON_WARNING,
                this);
            break;
        }
    }
}
