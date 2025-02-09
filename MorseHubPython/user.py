import pickle
import json
import cryptography.fernet
from datetime import datetime
import os
from morsePlayer import AudioSignal
class Settings:
    def __init__(self):
        self.speed = 20 # this is stored internally as wpm
        self.rufzmode = 0 #tracks the mode used, e.g. tainer, toplist or hst competitions
        self.mode = 0 # tracks the mode (rufz/morse runner/zeus), 0 means none
        self.rounds = 50 # in toplist this is the default value
        self.symbols = 5 # this matters only in trainer mode, this tracks the number of symbols played
        self.frequency = 440 # the ppitch
        self.signalsString = [name for name in AudioSignal.__dict__.keys() if not name.startswith('__') and not name.startswith('_') and not name.endswith('_')]
        self.audioSignal = AudioSignal.Sine
        self.audioSignalString = self.signalsString[self.audioSignal]
class Result:
    def __init__(self):
        self.errors = 0
        self.text=None
        self.userText = None
        self.ms = 0
        self.points = 0
        self.speed = 0
        self.pitch = 0
        self.time = datetime.now
        self.f6 = False
class Scoreboard:
    def __init__(self):
        self._results = []
        self.finalScore = 0
        
    def add(self, result: Result):
        if result not in self._results:
            self._results.append(result)
    def remove(self, value):
        self._results.remove(value)
    def getErrors(self) -> int:
        sum = 0
        for i in self._results:
            sum+=1 if i.errors!=0 else 0
        return sum
    def getPlayedAgainCount(self):
        sum=0
        for i in self._results:
            sum+=1 if i.f6 else 0
        return sum
class User:
    def __init__(self, name="", call=""):
        self.name=name
        self.call=call
        self.dateOfBirth = datetime.now()
        self.settings = Settings()
        self.scoreboard = Scoreboard()

user = User()
        

def loadSettings() -> Settings:
    if not os.path.exists("settings.json"):
        return None
    j = json.JSONDecoder()
    with open("settings.json") as f:
        settingsstr = f.read()
        #settingsstr = decryptData(settingsstr)
        try:
            settings_dict = j.decode(settingsstr)
            s = Settings()
            s.speed = settings_dict["speed"]
            s.rufzmode = settings_dict["rufzmode"]
            s.mode = settings_dict["mode"]
            s.rounds = settings_dict["rounds"]
            s.symbols = settings_dict["symbols"]
            s.frequency = settings_dict["frequency"]
            s.audioSignal = AudioSignal(settings_dict["audioSignal"])  # Convert back to enum
            s.audioSignalString = settings_dict["audioSignalString"]
            return s
        except json.JSONDecodeError:
            return None
        
def saveSettings(s: Settings):
    j = json.JSONEncoder()
    settings_dict = {
        "speed": s.speed,
        "rufzmode": s.rufzmode,
        "mode": s.mode,
        "rounds": s.rounds,
        "symbols": s.symbols,
        "frequency": s.frequency,
        "audioSignal": s.audioSignal.value,  # Assuming AudioSignal is an Enum
        "audioSignalString": s.audioSignalString,
    }
    settingsstr = j.encode(settings_dict)
    
    with open("settings.json", "w") as f:
        #settingsstr = encryptData(settingsstr)
        f.write(settingsstr)

def encryptData(what):
    c = cryptography.fernet.Fernet("ZeenyaeiyyENfzQKCd2J3VfdiCWCMxvZNTUtlxa3XjY=")
    encrypted = c.encrypt(what.encode())
    return encrypted.decode()

def decryptData(what):
    c = cryptography.fernet.Fernet("ZeenyaeiyyENfzQKCd2J3VfdiCWCMxvZNTUtlxa3XjY=")
    decrypted = c.decrypt(what.encode())
    return decrypted.decode()

def serializeUser(user):
    with open("user.dat", "wb") as f:
        pickle.dump(user, f)
def deserializeUser():
    if not os.path.exists("user.dat"):
        return
    with open("user.dat", "rb") as f:
        return pickle.load(f)