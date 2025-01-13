cmake_minimum_required(VERSION 3.15)
project("MorseHub")
if(NOT CMAKE_GENERATOR_PLATFORM)
set(CMAKE_GENERATOR_PLATFORM "x64")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#set compiler specific options
if(WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:inline /EHsc /MT /Z7")
set(LIBS kernel32 user32 gdi32 comdlg32 winspool shell32 shlwapi ole32 oleaut32 uuid advapi32 version comctl32 rpcrt4 ws2_32 wininet winmm wxmsw32u UniversalSpeechStatic PocoFoundationmt PocoEncodingsmt PocoJSONmt PocoNetmt PocoUtilmt PocoXMLmt CppUnitmt libcurl)
link_directories(${CMAKE_SOURCE_DIR}/winlib)
set(WININCLUDE_DIRS "${CMAKE_SOURCE_DIR}/winlib/mswu" "${CMAKE_SOURCE_DIR}/winlib/mswu/wx" "${CMAKE_SOURCE_DIR}/dep/wxInclude/msvc" "${CMAKE_SOURCE_DIR}/dep/wxInclude/msvc/wx") #for wx setup headers (windows only)
include_directories(${WININCLUDE_DIRS})
#add required macros
add_definitions(-DWXUSINGDLL)
endif()
#now add the rest of include directories
include_directories(${CMAKE_SOURCE_DIR}/dep ${CMAKE_SOURCE_DIR}/dep/wxInclude)
#now get all cpp and c files to be compiled
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/*.cpp")
file(GLOB CFILES "${CMAKE_SOURCE_DIR}/dep/*.c")
add_executable(MorseHub ${SOURCES} ${CFILES})
if(WIN32)
link_libraries(${LIBS})
endif()
set_target_properties(MorseHub PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/objects LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/objects)
