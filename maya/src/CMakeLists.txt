#set(BINARY ${CMAKE_PROJECT_NAME})
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)
#set(SOURCES ${SOURCES})

add_executable(${PROJECT_NAME} ${SOURCES})


############### Libraries ###############
target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)
#target_link_libraries(${PROJECT_NAME} PRIVATE Td::TdStatic)

if(LINUX)
	target_link_libraries(${PROJECT_NAME} PUBLIC
		pthread
	)
endif()

add_library(${PROJECT_NAME}_lib STATIC ${SOURCES}) #for using in tests
target_link_libraries(${PROJECT_NAME}_lib PRIVATE cpr::cpr)
#target_link_libraries(${PROJECT_NAME}_lib PRIVATE Td::TdStatic)
