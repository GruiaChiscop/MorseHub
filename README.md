# MorseHub
 An application used in HST competitions
## About the app
MorseHub is an application designed to be used in future HST competitions, as a replacement for the well known [RufzXP](https://www.rufzxp.net/) and [Morse Runner](https://www.dxatlas.com/MorseRunner/) programs.
# Why they need to be replaced?
There are several very important reasons why they need to be replaced, or, updated. One reason is that their own developers [Morse Runner](https://github.com/VE3NEA/morserunner) (only this is an open source project) no longer maintain these programs and, as the Microsoft windows operating systems evolve, they will become incompatible. For example, to use RufzXP, you need to install .Net FrameWork 2.0., which is a iteration that is very old and has many security issues. 
Another reason is related to accessibility and bugs. These programs were initially written in Pascal with Delphi, which for blind or visually impaired users is not so accessible with screen readers. It is impossible for a blind person using Morse Runner to view the maximum speed after finishing a competition or in RufzXP without using object navigator or OCR.
A third reason is that these programs are not operating cross platform. I think it's important for an app to work on as many platforms as possible (OSX, Linux and Windows generally).
# Future plans:
As this app was originally built in C#, currently I'm trying to port it to C++ to make it cross platform. Firstly, I need to port the morse library I created in C# with [NAudio](https://github.com/naudio/NAudio) and then write a cross platform build system with [SCons](https://github.com/SCons/scons).
Then, to make this app a legit one, I need to contact the authors of RufzXP to get the algorithm they used to calculate the scores. After this is done, this app will be ready for HST competitions.
