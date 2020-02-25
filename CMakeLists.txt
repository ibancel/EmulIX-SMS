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
file(GLOB_RECURSE source_files src/*)
file(GLOB_RECURSE header_files include/*)

# include_directories(include)

find_package(SFML 2.5 REQUIRED COMPONENTS system window graphics network audio main)
find_package(OpenGL COMPONENTS OpenGL)


include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(${project_name} ${source_files} ${header_files})

target_link_libraries(${project_name} OpenGL::GL sfml-system sfml-window sfml-graphics sfml-network sfml-audio sfml-main)

# target_include_directories(${project_name} PRIVATE
#							"${PROJECT_SOURCE_DIR}/include"
#							)