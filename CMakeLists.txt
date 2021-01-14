cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build Type")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Project variables
set(project_name EmulIX-SMS)
set(EXECUTABLE_OUTPUT_PATH bin/)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_SUPPRESS_REGENERATION True)

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -march=native")

project(${project_name} VERSION 0.1)

# Project files
set(source_files
	src/Audio.cpp
	src/Breakpoint.cpp
	src/Cartridge.cpp
	src/CPU.cpp
	src/Debugger.cpp
	src/definitions.cpp
	src/EmulatorException.cpp
        src/Frame.cpp
	src/Graphics.cpp
	src/GraphicsThread.cpp
	src/Inputs.cpp
	src/Log.cpp
	src/main.cpp
	src/Memory.cpp
	src/MemoryBank.cpp
	src/Stats.cpp
        src/System.cpp
        src/ui/GameWindow.cpp
        src/ui/MainWindow.cpp
        src/ui/SettingsInputsWindow.cpp
	src/Watcher.cpp
)
set(header_files
	include/Audio.h
	include/Breakpoint.h
	include/Cartridge.h
	include/CPU.h
	include/Debugger.h
	include/definitions.h
	include/EmulatorException.h
        include/Frame.h
	include/Graphics.h
	include/GraphicsThread.h
	include/Inputs.h
	include/Log.h
	include/Memory.h
	include/MemoryBank.h
	include/Singleton.h
	include/Stats.h
        include/System.h
        include/SystemComponent.h
        include/types.h
        include/ui/GameWindow.h
        include/ui/MainWindow.h
        include/ui/SettingsInputsWindow.h
	include/Watcher.h
)

set(ui_files
    src/ui/MainWindow.ui
    src/ui/SettingsInputsWindow.ui
)

include_directories(include)

find_package(SFML 2.5.1 REQUIRED COMPONENTS system audio)
find_package(OpenGL COMPONENTS OpenGL)

find_package(Qt5 COMPONENTS Core Widgets Gamepad REQUIRED)

set(qrc_files)
qt5_add_resources(qrc_files
    resources.qrc
)

add_executable(${project_name} ${source_files} ${header_files} ${ui_files} ${qrc_files})
target_include_directories(${project_name} PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${project_name} PRIVATE ${PROJECT_SOURCE_DIR})

target_link_libraries(${project_name} OpenGL::GL Qt5::Core Qt5::Gamepad Qt5::Widgets pthread sfml-system sfml-audio)

# Visual Studio purpose
set_target_properties(${project_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
