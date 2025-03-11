set(PROJECT_NAME UISystem)

file(GLOB Headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB Sources "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(UISystem
${Headers}
${Sources})

if(PS5)
	set(PROJECT_DEPENDENCIES
		${CMAKE_SOURCE_DIR}/ImGui/PS5
		${CMAKE_SOURCE_DIR}/NCLCoreClasses/
	)
else()
	set(PROJECT_DEPENDENCIES
		${CMAKE_SOURCE_DIR}/ImGui/WIN
		${CMAKE_SOURCE_DIR}/NCLCoreClasses/
	)
endif()

target_include_directories(${PROJECT_NAME} 
PUBLIC ${PROJECT_DEPENDENCIES}
)

add_dependencies(
	${PROJECT_NAME}
	ImGui
)
target_link_libraries(
${PROJECT_NAME}
PUBLIC ImGui) 

