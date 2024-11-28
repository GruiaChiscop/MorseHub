#pragma once
#include <wx/datetime.h>
#include <wx/string.h>
#include <string>
#include "SignalGenerator.h"
struct User
{
	std::string name;
	std::string callsign;
	wxDateTime dateOfBirth;
	int defaultSpeed;
	int defaultPitch;
SType signalType;
	User()
	{
		name = "";
		callsign = "none";
		dateOfBirth = wxDateTime::Now();
		defaultSpeed = 20;
		defaultPitch = 440;
		signalType = Sine;
	}
	User(const std::string& name, const std::string& callsign)
	{
		this->name = name;
		this->callsign = callsign;
		dateOfBirth = wxDateTime::Now();
		defaultSpeed = 20;
		defaultPitch = 440;
		signalType = Sine;
	}
};

void serialize(const User& user);
void deserialize(User& user);