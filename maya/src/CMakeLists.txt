file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)



############### Adding source files to target ${PROJECT_NAME} ###############

add_executable(${PROJECT_NAME} ${SOURCES})



############### Adding source files to target ${PROJECT_NAME}_lib ###############

add_library(${PROJECT_NAME}_lib STATIC ${SOURCES}) #The _lib target is needed for testing!



############### Linking libraries for target ${PROJECT_NAME} ###############

target_link_libraries(${PROJECT_NAME} PUBLIC cpr::cpr)
#target_link_libraries(${PROJECT_NAME} PRIVATE Td::TdStatic)

if(LINUX)
	target_link_libraries(${PROJECT_NAME} PUBLIC pthread )
endif()



############### Linking libraries for target ${PROJECT_NAME}_lib ###############

target_link_libraries(${PROJECT_NAME}_lib PUBLIC cpr::cpr)
#target_link_libraries(${PROJECT_NAME}_lib PRIVATE Td::TdStatic)
