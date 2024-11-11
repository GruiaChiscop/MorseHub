#ifndef CALLSIGN
#define CALLSIGN
#include "Utilities.h"
#include <fstream>
using namespace std;
class Callsign
{
    vector<string> items;
    string call;
    ifstream filestream;
    public:
    Callsign()
    {
        call = "";
    }
    ~Callsign()
    {
        filestream.close();
    }
    string currentCall() const { return call; }
    /* void load(const string& dataBase)
    * description: Loads a callsign database in memory
    * Note: This function will load the entire file if the file is in a linear format, e.g.: If the file is just a bunch of lines.
    * if the file is a .dta file, it will try just to get the database object for it.
    *  We need to this because we must randomly pick a callsign, and it's easier for us to choose froma  vector than walking through a file
    * it returns the number of calls found in the file
    */
   int load(const string& database)
   {
    filestream.open(database);
    string c;
    while(!filestream.eof())
    {
filestream>>c; //because the file contains only calls and new lines, we can use the operator>> to read
filestream.clear();
items.push_back(c);
    }
    items.shrink_to_fit(); //in case the capacity is larger than the size
return static_cast<int>(items.size());
   }
   string pickRandom()
   {
    size_t index = randomInteger(0, items.size());
    call = items.at(index);
    return call;
   }
   void optimise()
   {
    //if the filestream is still open, we close it
    if(filestream.is_open()) filestream.close();
    //if the capacity of our vector ist still larger, we shrink it
    items.shrink_to_fit();
   }
};

#endif