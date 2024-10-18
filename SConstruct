import os

env = Environment()
Decider('content-timestamp')

if env["PLATFORM"] == "win32":
    env.Append(CCFLAGS = ["/EHsc", "/J", "/MD", "/Z7", "/std:c++20", "/GF", "/Zc:inline", "/O2", "/bigobj", "/permissive-", "/D_CRT_SECURE_NO_WARNINGS"])
    env.Append(LIBS = ["kernel32", "user32", "gdi32", "comdlg32", "winspool", "shell32", "shlwapi", "ole32", "oleaut32", "uuid", "advapi32", "version", "comctl32", "rpcrt4", "ws2_32", "wininet", "winmm", "wxbase32u", "wxmsw32u_core"])
    env.Append(LIBPATH = ["winlib"])
    env.Append(CPPPATH = ["winlib/mswu", "winlib/mswu/wx", "dep/wxInclude/msvc", "dep/wxInclude/msvc/wx"])
    env.Append(CPPDEFINES =["WXUSINGDLL"])

env.Append(CPPPATH = ["dep/Tonic/src", "dep/json/single_include", "dep/wxInclude"])

sources = Glob("*.cpp")
env.Program("MorseHub", sources)
