/*pack.h - the pack files header*/
//this header is suposed to pack sounds and is inspired NVGT's pack system, but it can pack any other files if necessary

#pragma once

#include <string>
#include <vector>

using namespace std;

enum PackFileType { sound, file };
class Pack {
FILE* stream;
vector<string> files;
PackFileType pftype;
public:
string initialName;
string finalName;
Pack(const string& name="", PackFileType type);
bool open(const string& packName);
bool close();
bool add(const string& name, const string& inPackname);
bool exists(const string& inPackName);
vector<string> listAllFiles();
int getFilesCount();
bool extract(const string& inPackName, const string& finalName="");
string getInMemory(const string& inPackName);

};