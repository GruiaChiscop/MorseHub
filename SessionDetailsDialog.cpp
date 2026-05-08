#include "SessionDetailsDialog.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "Utilities.h"

SessionDetailsDialog::SessionDetailsDialog(wxWindow* parent, const User& value, const SessionRecord& record)
    : wxDialog(parent, wxID_ANY, "Session details", wxDefaultPosition, wxSize(980, 560)), user(value), session(record)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    const wxString replayHint = session.mode == "runner" || session.mode == "runner_wpx"
        ? "Double-click the call, correct text, sent text, response, or retransmitted text to hear that step in Morse with the saved round settings."
        : "Double-click Entered text or Correct text to hear it in Morse with the saved round settings.";
    sizer->Add(
        new wxStaticText(
            this,
            wxID_ANY,
            wxString::Format(
                "Mode: %s | Rounds: %d | Points: %d | Errors: %d | Max speed: %s\n%s",
                wxString::FromUTF8(session.mode),
                session.rounds,
                session.totalPoints,
                session.totalErrors,
                Utils::formatSpeed(session.maxSpeed, user.speedDisplayMode),
                replayHint)),
        0,
        wxALL,
        10);

    const bool hasRepeatColumn = supportsRepeatColumn();
    const bool hasRunnerColumns = supportsRunnerColumns();
    const int columnCount = 7 + (hasRepeatColumn ? 1 : 0) + (hasRunnerColumns ? 7 : 0);
    grid = new wxGrid(this, wxID_ANY);
    grid->CreateGrid(static_cast<int>(session.results.size()), columnCount);
    grid->SetColLabelValue(0, "Entered text");
    grid->SetColLabelValue(1, "Correct text");
    grid->SetColLabelValue(2, "Speed");
    grid->SetColLabelValue(3, "Possible score");
    grid->SetColLabelValue(4, "Awarded score");
    grid->SetColLabelValue(5, "Elapsed ms");
    grid->SetColLabelValue(6, "Errors");
    int nextColumn = 7;
    if (hasRepeatColumn)
        grid->SetColLabelValue(nextColumn++, session.mode == "runner" ? "Repeat" : "F6");
    if (hasRunnerColumns)
    {
        grid->SetColLabelValue(nextColumn++, "Entered call");
        grid->SetColLabelValue(nextColumn++, "Entered RST");
        grid->SetColLabelValue(nextColumn++, "Entered NR");
        grid->SetColLabelValue(nextColumn++, "Sent");
        grid->SetColLabelValue(nextColumn++, "Response");
        grid->SetColLabelValue(nextColumn++, "Retransmitted");
        grid->SetColLabelValue(nextColumn, "QSO state");
    }
    sizer->Add(grid, 1, wxALL | wxEXPAND, 10);

    sizer->Add(new wxButton(this, wxID_OK, "Close"), 0, wxALL | wxALIGN_CENTER, 10);
    SetSizer(sizer);

    grid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &SessionDetailsDialog::OnCellDoubleClick, this);
    populateGrid();
}

void SessionDetailsDialog::populateGrid()
{
    for (int row = 0; row < static_cast<int>(session.results.size()); ++row)
    {
        const Result& result = session.results[static_cast<std::size_t>(row)];
        grid->SetCellValue(row, 0, wxString::FromUTF8(result.typedText));
        grid->SetCellValue(row, 1, wxString::FromUTF8(result.text));
        grid->SetCellValue(row, 2, Utils::formatSpeed(result.speed, user.speedDisplayMode));
        grid->SetCellValue(row, 3, result.possiblePoints > 0 ? wxString::Format("%d", result.possiblePoints) : wxString("-"));
        grid->SetCellValue(row, 4, wxString::Format("%d", result.points));
        grid->SetCellValue(row, 5, wxString::Format("%d", result.elapsedMilliseconds));
        grid->SetCellValue(row, 6, wxString::Format("%d", result.errorCount));
        int nextColumn = 7;
        if (supportsRepeatColumn())
            grid->SetCellValue(row, nextColumn++, result.usedRepeat ? "Yes" : "No");
        if (supportsRunnerColumns())
        {
            grid->SetCellValue(row, nextColumn++, wxString::FromUTF8(result.runnerEnteredCall));
            grid->SetCellValue(row, nextColumn++, wxString::FromUTF8(result.runnerEnteredRst));
            grid->SetCellValue(row, nextColumn++, wxString::FromUTF8(result.runnerEnteredNr));
            grid->SetCellValue(row, nextColumn++, wxString::FromUTF8(result.runnerSentText));
            grid->SetCellValue(row, nextColumn++, wxString::FromUTF8(result.runnerResponseText));
            grid->SetCellValue(row, nextColumn++, wxString::FromUTF8(result.runnerRetransmittedText));
            grid->SetCellValue(row, nextColumn, wxString::FromUTF8(result.runnerStateText));
        }

        for (int col = 0; col < grid->GetNumberCols(); ++col)
            grid->SetReadOnly(row, col, true);
    }

    grid->AutoSizeColumns();
}

void SessionDetailsDialog::playTextFromCell(int row, int col)
{
    if (row < 0 || row >= static_cast<int>(session.results.size()))
        return;

    const Result& result = session.results[static_cast<std::size_t>(row)];
    std::string text;
    if (col == 0)
        text = result.typedText;
    else if (col == 1)
        text = result.text;
    else if (supportsRunnerColumns())
    {
        const int runnerStartColumn = supportsRepeatColumn() ? 8 : 7;
        const int enteredCallColumn = runnerStartColumn;
        const int sentColumn = runnerStartColumn + 3;
        const int responseColumn = runnerStartColumn + 4;
        const int retransmitColumn = runnerStartColumn + 5;
        if (col == enteredCallColumn)
            text = result.runnerEnteredCall;
        else if (col == sentColumn)
            text = result.runnerSentText;
        else if (col == responseColumn)
            text = result.runnerResponseText;
        else if (col == retransmitColumn)
            text = result.runnerRetransmittedText;
    }
    if (text.empty())
        return;

    const auto outputDevice = user.preferredOutputDeviceIndex >= 0
        ? std::optional<AudioOutputDevice>(AudioOutputDeviceService::resolvePlaybackDevice(user.preferredOutputDeviceIndex))
        : std::nullopt;
    playbackGenerator = std::make_unique<MorseGenerator>(
        result.speed > 0 ? result.speed : user.defaultSpeed,
        result.frequency > 0 ? result.frequency : user.defaultPitch,
        result.signalType,
        outputDevice);
    playbackGenerator->transmitAsync(text);
}

bool SessionDetailsDialog::supportsRepeatColumn() const
{
    return session.mode == "rufz" || session.mode == "runner" || session.mode == "runner_wpx";
}

bool SessionDetailsDialog::supportsRunnerColumns() const
{
    return session.mode == "runner" || session.mode == "runner_wpx";
}

void SessionDetailsDialog::OnCellDoubleClick(wxGridEvent& event)
{
    playTextFromCell(event.GetRow(), event.GetCol());
    event.Skip();
}
