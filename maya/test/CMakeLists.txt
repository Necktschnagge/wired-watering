set(TEST_PROJECT_NAME TEST-${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)

set(SOURCES ${TEST_SOURCES})

add_executable(${TEST_PROJECT_NAME} ${TEST_SOURCES})

add_test(NAME ${TEST_PROJECT_NAME} COMMAND ${TEST_PROJECT_NAME})

#copy gtestd.dll into directory where TEST-...exe is located
if (WIN32)
add_custom_command(TARGET ${TEST_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/build/bin/Debug"
        "${PROJECT_SOURCE_DIR}/build/Debug")
endif()

############### Libraries ###############
if(LINUX)
	target_link_libraries(${TEST_PROJECT_NAME} PUBLIC
		pthread
	)
endif()

target_link_libraries(${TEST_PROJECT_NAME} PUBLIC ${CMAKE_PROJECT_NAME}_lib gtest)
