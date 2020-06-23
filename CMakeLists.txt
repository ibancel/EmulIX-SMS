cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build Type")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)


# Project variables
set(project_name EmulIX-SMS)
set(EXECUTABLE_OUTPUT_PATH bin/)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_SUPPRESS_REGENERATION True)

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
	src/Graphics.cpp
	src/GraphicsThread.cpp
	src/Inputs.cpp
	src/Log.cpp
	src/main.cpp
	src/Memory.cpp
	src/MemoryBank.cpp
	src/Stats.cpp
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
	include/Graphics.h
	include/GraphicsThread.h
	include/Inputs.h
	include/Log.h
	include/Memory.h
	include/MemoryBank.h
	include/Singleton.h
	include/Stats.h
	include/Watcher.h
)

# include_directories(include)

find_package(SFML 2.5 REQUIRED COMPONENTS system window graphics network audio main)
find_package(OpenGL COMPONENTS OpenGL)


add_executable(${project_name} ${source_files} ${header_files})
target_include_directories(${project_name} PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${project_name} OpenGL::GL sfml-system sfml-window sfml-graphics sfml-network sfml-audio sfml-main)