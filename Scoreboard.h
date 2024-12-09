#pragma once
#include <string>
#include <vector>
#include "Result.h"

using namespace std;

class Scoreboard
{
    public:
    int errors() const { return m_errors; }
    const std::vector<Result> results() { return m_results; }
    size_t addResult(const Result& r);
    size_t removeLast();
    size_t removeFirst();
    size_t removeAt(size_t index);
    size_t removeWhat(const Result& r);
    Result getHighest() const;
    Result getLowest() const;
    void sort(bool ascending=true);

    private:
    vector<Result> m_results;
    int m_errors = 0;
    void update();
};