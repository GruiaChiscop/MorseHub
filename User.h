#pragma once
#include <wx/datetime.h>
#include <wx/string.h>
#include <string>
#include "nlohmann/json.hpp"
class User
{
public:
	std::string name;
	std::string callsign;
	wxDateTime dateOfBirth;
	int defaultSpeed;
	int defaultPitch;

	User()
	{
		name = "";
		callsign = "";
		dateOfBirth = wxDateTime::Now();
		defaultSpeed = 20;
		defaultPitch = 440;
	}
	User(const std::string& name, const std::string& callsign)
	{
		this->name = name;
		this->callsign = callsign;
		dateOfBirth = wxDateTime::Now();
		defaultSpeed = 20;
		defaultPitch = 440;
	}
};

