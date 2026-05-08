#pragma once
#include <wx/datetime.h>
#include <wx/string.h>
#include <string>
#include <vector>
#include "SignalGenerator.h"

enum class SpeedDisplayMode
{
	Cpm,
	Wpm
};

enum class RunnerMode
{
	Pileup,
	SingleCalls,
	WpxCompetition
};

enum class MainMode
{
	None,
	Rufz,
	Trainer,
	Runner,
	Reception
};

struct RufzTrainerSettings
{
	bool fixedSpeed{true};
	int speed{20};
	int adaptiveStep{5};
	int minSpeed{10};
	int groupLength{5};
	std::string allowedSymbols{"abcdefghijklmnopqrstuvwxyz1234567890.,/?=+"};
};

using TrainerSettings = RufzTrainerSettings;

struct RufzSettings
{
	int maxRounds{50};
	bool allowRepeat{true};
	int speedStepCpm{10};
	int minSpeedCpm{20};
	RufzTrainerSettings trainer;
};

struct ReceptionSettings
{
	bool fixedSpeed{true};
	int speed{20};
	int groupLength{5};
	std::string allowedSymbols{"abcdefghijklmnopqrstuvwxyz1234567890.,/?=+"};
	std::vector<int> speeds{60, 80, 100};
	bool useInternalInput{true};
	int resultEntryMinutes{30};
	int transmissionSeconds{60};
	int pauseSeconds{60};
	std::string groupSeparator{";"};
	int pitch{440};
	SType signalType{Sine};
};

struct RunnerSettings
{
	int initialSpeed{120};
	RunnerMode mode{RunnerMode::Pileup};
	int competitionMinutes{15};
	bool enableQrm{true};
	bool enableQrn{true};
	bool enableQsb{true};
	bool enableFlutter{true};
	bool enableLids{true};
};

struct ModuleSettings
{
	RufzSettings rufz;
	ReceptionSettings reception;
	RunnerSettings runner;
};

struct User
{
	std::string name;
	std::string callsign;
	wxDateTime dateOfBirth;
	int defaultSpeed;
	int defaultPitch;
	SType signalType;
	SpeedDisplayMode speedDisplayMode;
	MainMode lastSelectedMode;
	int preferredOutputDeviceIndex;
	int outputVolumePercent;
	ModuleSettings moduleSettings;
	User()
	{
		name = "";
		callsign = "none";
		dateOfBirth = wxDateTime::Now();
		defaultSpeed = 20;
		defaultPitch = 440;
		signalType = Sine;
		speedDisplayMode = SpeedDisplayMode::Cpm;
		lastSelectedMode = MainMode::None;
		preferredOutputDeviceIndex = -1;
		outputVolumePercent = 50;
	}
	User(const std::string& name, const std::string& callsign)
	{
		this->name = name;
		this->callsign = callsign;
		dateOfBirth = wxDateTime::Now();
		defaultSpeed = 20;
		defaultPitch = 440;
		signalType = Sine;
		speedDisplayMode = SpeedDisplayMode::Cpm;
		lastSelectedMode = MainMode::None;
		preferredOutputDeviceIndex = -1;
		outputVolumePercent = 50;
	}
};

void serialize(const User& user);
void deserialize(User& user);
