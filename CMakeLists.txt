cmake_minimum_required(VERSION 3.21)
project(GravitySimulation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(InstallRequiredSystemLibraries)

if(NOT WIN32 AND NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type specified; defaulting to Release.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the build type (Debug, Release, RelWithDebInfo, MinSizeRel)." FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -march=native -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
endif()

FetchContent_Declare(
    sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        2.6.2 
)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(sfml)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/GravitySimulation/bin)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(GravitySimulation ${SOURCES})

target_link_libraries(GravitySimulation PRIVATE sfml-graphics sfml-window sfml-system)

set_target_properties(GravitySimulation PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/GravitySimulation/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/GravitySimulation/bin
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/GravitySimulation/bin
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/GravitySimulation/bin
)

if(WIN32)
    add_custom_command(TARGET GravitySimulation POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:GravitySimulation> $<TARGET_FILE_DIR:GravitySimulation>
        COMMAND_EXPAND_LISTS
    )
endif()

add_custom_command(
    TARGET GravitySimulation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            ${CMAKE_BINARY_DIR}/GravitySimulation/resources
)