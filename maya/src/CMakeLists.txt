file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

add_library(${PROJECT_NAME}_lib STATIC ${SOURCES}) #for using in tests

############### Libraries for executable target ###############
if(LINUX)
	target_link_libraries(${PROJECT_NAME} PUBLIC
		pthread
	)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)

############### Libraries for lib target ###############

target_link_libraries(${PROJECT_NAME}_lib PRIVATE cpr::cpr)
#target_link_libraries(${PROJECT_NAME}_lib PRIVATE Td::TdStatic)
