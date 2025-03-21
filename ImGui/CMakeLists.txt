set(PROJECT_NAME ImGui)

if(PS5)
	file(GLOB Headers "${CMAKE_CURRENT_SOURCE_DIR}/PS5/*.h")
	file(GLOB Sources "${CMAKE_CURRENT_SOURCE_DIR}/PS5/*.cpp")
	
	set(IMGUI_SOURCES
    ${SCE_PROSPERO_SDK_DIR}/target/samples/sample_code/common/source/imgui/imgui.cpp
    ${SCE_PROSPERO_SDK_DIR}/target/samples/sample_code/common/source/imgui/imgui_draw.cpp
    ${SCE_PROSPERO_SDK_DIR}/target/samples/sample_code/common/source/imgui/imgui_widgets.cpp
    ${SCE_PROSPERO_SDK_DIR}/target/samples/sample_code/common/source/imgui/prospero/imgui_libfont_prospero.cpp
	)
	set(IMGUI_HEADERS
		${SCE_PROSPERO_SDK_DIR}/target/samples/sample_code/common/include/sampleutil/ui_framework/imgui/imgui_libfont.h
		${SCE_PROSPERO_SDK_DIR}/target/samples/sample_code/common/source/imgui/imconfig.h
		${SCE_PROSPERO_SDK_DIR}/target/samples/sample_code/common/source/imgui/imgui.h
		${SCE_PROSPERO_SDK_DIR}/target/samples/sample_code/common/source/imgui/imgui_internal.h
		${SCE_PROSPERO_SDK_DIR}/target/samples/sample_code/common/source/imgui/imstb_rectpack.h
		${SCE_PROSPERO_SDK_DIR}/target/samples/sample_code/common/source/imgui/imstb_textedit.h
		${SCE_PROSPERO_SDK_DIR}/target/samples/sample_code/common/source/imgui/imstb_truetype.h
	)
	
	source_group("ImGUI IMPL" FILES ${Headers} ${Sources})
	source_group("ImGUI UTIL" FILES ${IMGUI_SOURCES} ${IMGUI_HEADERS})

	add_library(ImGui
	${Headers}
	${Sources}
	${IMGUI_SOURCES}
	${IMGUI_HEADERS})
else()
	file(GLOB_RECURSE Headers "${CMAKE_CURRENT_SOURCE_DIR}/WIN/*.h")
	file(GLOB_RECURSE Sources "${CMAKE_CURRENT_SOURCE_DIR}/WIN/*.cpp")
	add_library(ImGui
	${Headers}
	${Sources})
endif()



if(PS5)
	set(DEPENDENCIES
		${CMAKE_SOURCE_DIR}/ImGui/PS5
		${CMAKE_SOURCE_DIR}/NCLCoreClasses/
		${SCE_PROSPERO_SDK_DIR}/target/samples/sample_code/common/include
		${SCE_PROSPERO_SDK_DIR}/target/samples/sample_code/common/source/imgui
		libSceSysmodule_stub_weak
		SceFont_stub_weak
		SceFontFt_stub_weak
		SceEnC_nosubmission
		SceMat_nosubmission_stub_weak
		ScePosix_stub_weak
		SceSysmodule_stub_weak
	)
#PUBLIC ${CMAKE_SOURCE_DIR}/ImGui/PS5
else()
	set(DEPENDENCIES
		${CMAKE_SOURCE_DIR}/ImGui/WIN
		${CMAKE_SOURCE_DIR}/NCLCoreClasses/
	)
endif()

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        SCE_AGC_CHECK_ERROR_CODES
        _SCE_SAMPLE_UTIL_IMGUI_EXTERNAL_BUILD
)

target_include_directories(${PROJECT_NAME} 
PUBLIC ${DEPENDENCIES}
PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}"
#PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/MyLib
)