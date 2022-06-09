file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})


############### Libraries ###############
if(LINUX)
	target_link_libraries(${PROJECT_NAME} PUBLIC
		pthread
	)
endif()

add_library(${PROJECT_NAME}_lib STATIC ${SOURCES}) #for using in tests
