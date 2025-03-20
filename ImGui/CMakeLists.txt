set(PROJECT_NAME ImGui)

if(PS5)
	file(GLOB Headers "${CMAKE_CURRENT_SOURCE_DIR}/PS5/*.h")
	file(GLOB Sources "${CMAKE_CURRENT_SOURCE_DIR}/PS5/*.cpp")
else()
	file(GLOB_RECURSE Headers "${CMAKE_CURRENT_SOURCE_DIR}/WIN/*.h")
	file(GLOB_RECURSE Sources "${CMAKE_CURRENT_SOURCE_DIR}/WIN/*.cpp")
endif()

add_library(ImGui
${Headers}
${Sources})

if(PS5)
	set(PROJECT_DEPENDENCIES
		${CMAKE_SOURCE_DIR}/ImGui/PS5
		${CMAKE_SOURCE_DIR}/NCLCoreClasses/
	)
#PUBLIC ${CMAKE_SOURCE_DIR}/ImGui/PS5
else()
	set(PROJECT_DEPENDENCIES
		${CMAKE_SOURCE_DIR}/ImGui/WIN
		${CMAKE_SOURCE_DIR}/NCLCoreClasses/
	)
endif()

target_include_directories(${PROJECT_NAME} 
PUBLIC ${DEPENDENCIES}
#PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/MyLib
)