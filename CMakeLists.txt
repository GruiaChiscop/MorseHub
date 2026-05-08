cmake_minimum_required(VERSION 3.21)

project(MorseHub VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MORSEHUB_BUILD_APP "Build the MorseHub desktop application." ON)
option(MORSEHUB_BUILD_TESTS "Build MorseHub tests." ON)

include(CTest)

if(MSVC)
    add_compile_options(/permissive- /FS)
endif()

set(MORSEHUB_WX_COMPONENTS base)
if(MORSEHUB_BUILD_APP)
    list(APPEND MORSEHUB_WX_COMPONENTS core adv)
endif()
list(REMOVE_DUPLICATES MORSEHUB_WX_COMPONENTS)

if(MORSEHUB_BUILD_APP OR MORSEHUB_BUILD_TESTS)
    add_library(morsehub_wx INTERFACE)

    find_package(wxWidgets CONFIG QUIET COMPONENTS ${MORSEHUB_WX_COMPONENTS})
    if(TARGET wxWidgets::wxWidgets)
        target_link_libraries(morsehub_wx INTERFACE wxWidgets::wxWidgets)
    elseif(TARGET wx::base)
        target_link_libraries(morsehub_wx INTERFACE wx::base)
        if("core" IN_LIST MORSEHUB_WX_COMPONENTS AND TARGET wx::core)
            target_link_libraries(morsehub_wx INTERFACE wx::core)
        endif()
        if("adv" IN_LIST MORSEHUB_WX_COMPONENTS AND TARGET wx::adv)
            target_link_libraries(morsehub_wx INTERFACE wx::adv)
        endif()
    else()
        find_package(wxWidgets REQUIRED COMPONENTS ${MORSEHUB_WX_COMPONENTS})
        include(${wxWidgets_USE_FILE})
        target_include_directories(morsehub_wx INTERFACE ${wxWidgets_INCLUDE_DIRS})
        target_link_libraries(morsehub_wx INTERFACE ${wxWidgets_LIBRARIES})
    endif()
endif()

add_library(morsehub_rufz
    RufzScoring.cpp
)

target_include_directories(morsehub_rufz
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(morsehub_support
    AudioOutputDevice.cpp
    ReceptionParser.cpp
    RunnerAudioEffects.cpp
    RunnerSupport.cpp
    StringGen.cpp
    dep/miniaudio.c
)

target_include_directories(morsehub_support
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/dep
)

if(MORSEHUB_BUILD_APP)
    add_executable(MorseHub WIN32
        mainWindow.cpp
        MorseHubSettingsDialog.cpp
        MorseGenerator.cpp
        MorseHub.cpp
        ReceptionCompetitionFrame.cpp
        ReceptionResultsDialog.cpp
        ReceptionSettingsDialog.cpp
        RunnerCompetitionFrame.cpp
        RunnerStartDialog.cpp
        RUFZCompetitionFrame.cpp
        RUFZStartDialog.cpp
        SessionDetailsDialog.cpp
        SessionHistory.cpp
        SignalGenerator.cpp
        TrainerCompetitionFrame.cpp
        TrainerSettingsDialog.cpp
        User.cpp
        utilities.cpp
        WelcomeWindow.cpp
    )

    target_include_directories(MorseHub
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/dep
    )

    target_link_libraries(MorseHub
        PRIVATE
            morsehub_support
            morsehub_rufz
            morsehub_wx
    )

    set_target_properties(MorseHub PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/objects
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/objects
    )
endif()

if(MORSEHUB_BUILD_TESTS)
    include(FetchContent)

    find_package(Catch2 3 CONFIG QUIET)
    if(NOT Catch2_FOUND)
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.8.1
        )
        FetchContent_MakeAvailable(Catch2)
    endif()

    add_executable(morsehub_rufz_tests
        SignalGenerator.cpp
        tests/ReceptionParserTests.cpp
        tests/RunnerAudioEffectsTests.cpp
        tests/RunnerSupportTests.cpp
        tests/RufzScoringTests.cpp
        tests/SignalGeneratorTests.cpp
        tests/TrainerSupportTests.cpp
    )

    target_link_libraries(morsehub_rufz_tests
        PRIVATE
            morsehub_support
            morsehub_rufz
            morsehub_wx
            Catch2::Catch2WithMain
    )

    include(Catch)
    catch_discover_tests(morsehub_rufz_tests)
endif()
