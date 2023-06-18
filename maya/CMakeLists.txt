cmake_minimum_required(VERSION 3.13.4)


############### Names ###############
set(PROJECT_NAME Maya-Commander)


############### Preprocessor Macros ###############
add_compile_definitions(GIT_REPOSITORY_URL="https://github.com/Necktschnagge/wired-watering")


############### Project Decklaration ###############
project(${PROJECT_NAME} VERSION 1.0)


############### Check Platform ###############
message("CMake is checking for CMAKE_SYSTEM_PROCESSOR containing arm to detect if running on Raspberry Pi:")
message("CMAKE_SYSTEM_PROCESSOR is: " ${CMAKE_SYSTEM_PROCESSOR})
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
	message("Running on Raspberry Pi.")
	set(on_target_platform true)
else(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
	message("Running on Development or Test Platform.")
	set(on_target_platform false)
endif()
if(UNIX AND NOT APPLE)
	set(LINUX true)
else()
	set(LINUX false)
endif()
if (WIN32)
	message("Running on WIN32.")
endif()
if (LINUX)
	message("Running on LINUX.")
endif()
if (APPLE)
	message("Running on APPLE.")
endif()
if (UNIX)
	message("Running on UNIX.")
endif()

if(WIN32) # Install dlls in the same directory as the executable on Windows # at least some of them
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
endif()

############### Language and Flags ###############
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)
if (WIN32)
	#set(CMAKE_CXX_FLAGS_RELEASE "/MT /O2 /Ob2 /DNDEBUG")
	#set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Zi /Ob0 /Od /RTC1")
endif()

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG db351ffbbadc6c4e9239daaa26e9aefa9f0ec82d) # The commit hash for 1.8.0. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

############### Libraries ###############
include_directories(SYSTEM extern/spdlog/include)
#include_directories(SYSTEM extern/cpr/include)
#include_directories(SYSTEM build/_deps/curl-src/include)
add_subdirectory(extern/googletest)

add_subdirectory(extern/td)

#add_subdirectory(extern/cpr)

############### Sources ###############
add_subdirectory(src)


############### Testing ###############
add_subdirectory(test)
enable_testing()

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()


############### Visual Studio ###############
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})#Set Visualo Studio start-up project, so that one can directly run the debugger.
