import threading
from pydub import AudioSegment, playback
from pydub.generators import Sine, Square, Triangle, Sawtooth
from enum import IntEnum

class AudioSignal(IntEnum):
    Sine = 0
    Square = 1
    Triangle = 2
    Sawtooth = 3

class MorseGenerator:
    def __init__(self, audioSignal=AudioSignal.Sine):
        self.speed = 20
        self.frequency = 440.0
        self.signal = audioSignal
        self._buffer = None
        self.thread = None
        self.codes = {
            "0": "-----", "1": ".----", "2": "..---", "3": "...--", "4": "....-",
            "5": ".....", "6": "-....", "7": "--...", "8": "---..", "9": "----.",
            "a": ".-", "b": "-...", "c": "-.-.", "d": "-..", "e": ".", "f": "..-.",
            "g": "--.", "h": "....", "i": "..", "j": ".---", "k": "-.-", "l": ".-..",
            "m": "--", "n": "-.", "o": "---", "p": ".--.", "q": "--.-", "r": ".-.",
            "s": "...", "t": "-", "u": "..-", "v": "...-", "w": ".--", "x": "-..-",
            "y": "-.--", "z": "--..", ".": ".-.-.-", ",": "--..--", "?": "..--..",
            "!": "-.-.--", "-": "-....-", "/": "-..-.", "@": ".--.-.", "(": "-.--.",
            ")": "-.--.-", " ": " "
        }

        self._set_generator()

    def _set_generator(self):
        if self.signal == AudioSignal.Sine:
            self.generator = Sine(self.frequency)
        elif self.signal == AudioSignal.Square:
            self.generator = Square(self.frequency)
        elif self.signal == AudioSignal.Triangle:
            self.generator = Triangle(self.frequency)
        else:
            self.generator = Sawtooth(self.frequency)

    @property
    def dot(self):
        return 1.2 / self.speed * 1000  # Convert to milliseconds

    @property
    def line(self):
        return 3.6 / self.speed * 1000  # Convert to milliseconds
    @property
    def audioSignal(self): return self.signal
    @audioSignal.setter
    def audioSignal(self, value):
        self.signal = value
        self._set_generator()
    @property
    def freq(self): return self.frequency
    @freq.setter
    def freq(self, value):
        self.frequency=value
        self._set_generator()
    def _char_add(self, c: str):
        try:
            text = self.codes[c.lower()]  # Convert to lowercase
        except KeyError:
            return

        if self._buffer is None:
            self._buffer = AudioSegment.silent(0)

        for i in text:
            if i == '.':
                self._buffer += self.generator.to_audio_segment(duration=self.dot)
            elif i == '-':
                self._buffer += self.generator.to_audio_segment(duration=self.line)

            self._buffer += AudioSegment.silent(self.dot)

        self._buffer += AudioSegment.silent(self.line)

    def _word_add(self, word: str):
        for i in word:
            self._char_add(i)
        self._buffer += AudioSegment.silent(7 * self.dot)

    def _transmit(self, text: str):
        words = text.split()
        if self._buffer is None:
            self._buffer = AudioSegment.silent(0)
        for w in words:
            self._word_add(w)
        playback._play_with_pyaudio(self._buffer) #Using this because SimpleAudio has some problems and makes the app exit when it finishes playback
        self._buffer = None

    def transmit(self, text: str):
        if self.isPlaying(): return
        self.thread = threading.Thread(target=self._transmit, args=(text,), daemon=True)
        self.thread.start()
    def isPlaying(self):
        if self.thread is None: return None
        return self.thread.is_alive()
    def output(self, text: str):
        from morse3 import Morse
        m = Morse(text)
        m.codes["="] = "-...-" #the equals sign does not exist
        m.codes["+"] = "-.-.-" #neither this sign does exist
        return m.stringToMorse()
    def toFile(self, filename: str, text: str):
        words = text.split()
        if self._buffer is None:
            self._buffer = AudioSegment.silent(0)

        for word in words: self._word_add(word)
        self._buffer.export(filename[:filename.rfind('.')], format = filename[filename.rfind('.'):])