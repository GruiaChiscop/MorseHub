#pragma once
#include <string>
#include <vector>
#include "Result.h"
#include "sqlite3/sqlite3.h"
using namespace std;
using SortCondition= bool(*)(const Result& a, const Result& b);
using SQLiteCallback = int(*)(void*, int, char**, char**);
class Scoreboard
{
    public:
    Scoreboard();
    void openSCB(const string& file);
    void close();
    int errors() const { return m_errors; } 
    vector<Result> results() { return m_results; }
    size_t addResult(const Result& r);
    size_t removeLast();
    size_t removeFirst();
    size_t removeAt(size_t index);
    size_t removeWhat(const Result& r);
    Result getHighest() const;
    Result getLowest() const;
    double average(); //makes the average between scores
    void sortByErrors(bool ascending=true);
    void sortByScore(bool ascending=true);
void sortBy(SortCondition condition);
    private:
    vector<Result> m_results;
    sqlite3* database;
    sqlite3_stmt* insertSTMT, *removeSTMT, *querySTMT, *extraSTMT;
    int m_errors = 0;
    SQLiteCallback queryCallback;
    SQLiteCallback insertCallback;
    SQLiteCallback removeCallback;
bool isOpen;
};