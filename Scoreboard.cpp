#include "Result.h"
#include "ScoreBoard.h"
#include <sqlite3/sqlite3.h>
#include "Poco/String.h"
#include "Poco/NumberParser.h"
#include <stdexcept>
using namespace std;
Scoreboard::Scoreboard()
{

}

void Scoreboard::openSCB(const string& file)
{
    if(isOpen) throw runtime_error("A database is already opened");
    if(sqlite3_open(file.c_str(), &database)!=SQLITE_OK) throw runtime_error(sqlite3_errmsg(database));

const char* insertSQL = "INSERT INTO Results (text, typed_text, points, miliseconds, speed, pitch) VALUES (?, ?, ?, ?, ?, ?);";
const char* querySQL = "SELECT * FROM Results;";
const char* removeSQL = "DELETE FROM Results ";
sqlite3_prepare_v2(database, insertSQL, -1, &insertSTMT, NULL);
sqlite3_prepare_v2(database, querySQL, -1, &querySTMT, NULL);
    isOpen=true;
}