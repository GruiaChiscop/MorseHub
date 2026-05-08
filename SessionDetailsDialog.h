#pragma once

#include <wx/dialog.h>
#include <wx/grid.h>

#include <memory>
#include <optional>

#include "AudioOutputDevice.h"
#include "MorseGenerator.h"
#include "SessionHistory.h"
#include "User.h"

class SessionDetailsDialog : public wxDialog
{
public:
    SessionDetailsDialog(wxWindow* parent, const User& user, const SessionRecord& session);

private:
    const User& user;
    SessionRecord session;
    wxGrid* grid{};
    std::unique_ptr<MorseGenerator> playbackGenerator = nullptr;

    void populateGrid();
    void playTextFromCell(int row, int col);
    bool supportsRepeatColumn() const;
    bool supportsRunnerColumns() const;

    void OnCellDoubleClick(wxGridEvent& event);
};
