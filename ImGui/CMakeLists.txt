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
	set(DEPENDENCIES
		${CMAKE_SOURCE_DIR}/ImGui/PS5
		${CMAKE_SOURCE_DIR}/NCLCoreClasses/
		PRIVATE SceAgcDriver_stub_weak
		PRIVATE SceAgc_stub_weak
		PRIVATE SceVideoOut_stub_weak
		PRIVATE SceAgc_debug_nosubmission
		PRIVATE SceAgcCore_debug_nosubmission
		PRIVATE SceAgcGpuAddress_debug_nosubmission
		
		PRIVATE ScePad_stub_weak
		PRIVATE SceUserService_stub_weak
		
		PRIVATE SceNpUtility_stub_weak
		PRIVATE SceCommonDialog_stub_weak
		PRIVATE SceNetCtlApDialog_stub_weak
		PRIVATE SceNetCtlAp_stub_weak
		PRIVATE SceNetCtl_stub_weak
		PRIVATE SceNet_stub_weak
		PRIVATE ScePosix_stub_weak
	)
#PUBLIC ${CMAKE_SOURCE_DIR}/ImGui/PS5
else()
	set(DEPENDENCIES
		${CMAKE_SOURCE_DIR}/ImGui/WIN
		${CMAKE_SOURCE_DIR}/NCLCoreClasses/
	)
endif()

target_include_directories(${PROJECT_NAME} 
PUBLIC ${DEPENDENCIES}
#PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/MyLib
)
target_link_libraries(${PROJECT_NAME} 
    PUBLIC ${DEPENDENCIES}
	)