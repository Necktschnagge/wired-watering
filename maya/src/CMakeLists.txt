file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})


############### Libraries ###############
target_link_libraries(${PROJECT_NAME} PUBLIC cpr::cpr)
#target_link_libraries(${PROJECT_NAME} PRIVATE Td::TdStatic)

if(LINUX)
	target_link_libraries(${PROJECT_NAME} PUBLIC
		pthread
	)
endif()

add_library(${PROJECT_NAME}_lib STATIC ${SOURCES}) #for using in tests
target_link_libraries(${PROJECT_NAME}_lib PUBLIC cpr::cpr)
#target_link_libraries(${PROJECT_NAME}_lib PRIVATE Td::TdStatic)
