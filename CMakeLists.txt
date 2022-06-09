cmake_minimum_required(VERSION 3.13.4)


############### Names ###############
set(PROJECT_NAME ___template___project___name___)


############### Preprocessor Macros ###############
add_compile_definitions(GIT_REPOSITORY_URL="___template___project___url___")


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


############### Language and Flags ###############
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)
set(CMAKE_CXX_FLAGS_RELEASE "/MT /O2 /Ob2 /DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Zi /Ob0 /Od /RTC1")


############### Libraries ###############
include_directories(SYSTEM extern/spdlog/include)
add_subdirectory(extern/googletest)


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
