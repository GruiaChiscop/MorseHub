import os
env = Environment()
if env["TARGET_ARCH"]=="arm64": env["TARGET_ARCH"] = "amd64" # MorseHub can't be compiled for arm64 devices yet
env.Decider('content-timestamp')

if env["PLATFORM"] == "win32":
    env.Append(CCFLAGS = ["/EHsc", "/J", "/MD", "/Z7", "/std:c++20", "/GF", "/Zc:inline", "/O2", "/bigobj", "/permissive-", "/D_CRT_SECURE_NO_WARNINGS"])
    env.Append(LIBS = ["kernel32", "user32", "gdi32", "comdlg32", "winspool", "shell32", "shlwapi", "ole32", "oleaut32", "uuid", "advapi32", "version", "comctl32", "rpcrt4", "ws2_32", "wininet", "winmm", "wxbase32u", "wxmsw32u_core"])
    env.Append(LIBPATH = ["winlib"])
    env.Append(CPPPATH = ["winlib/mswu", "winlib/mswu/wx", "dep/wxInclude/msvc", "dep/wxInclude/msvc/wx"])
    env.Append(CPPDEFINES =["WXUSINGDLL"])

env.Append(CPPPATH = ["dep", "dep/wxInclude"])

sources = Glob("*.cpp")
cfiles = Glob("dep/*.c")
objects = []
for source in sources + cfiles:
    obj = env.Object(target="build/objects/" + os.path.basename(str(source)).replace('.cpp', '.obj').replace('.c', '.obj'), source=source)
    objects.append(obj)

env.Program(target="bin/MorseHub", source=objects)