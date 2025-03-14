set(PROJECT_NAME CSC8508)

file(GLOB_RECURSE Header_Files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB_RECURSE Source_Files "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB Legacy_Files CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Legacy/*")
file(GLOB PS5_Files CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/PS5/*")

list(REMOVE_ITEM Source_Files ${Legacy_Files})
list(REMOVE_ITEM Header_Files ${Legacy_Files})
list(REMOVE_ITEM Source_Files ${PS5_Files})
list(REMOVE_ITEM Header_Files ${PS5_Files})

source_group("Header Files" FILES ${Header_Files})
source_group("Source Files" FILES ${Source_Files})
source_group("Legacy" FILES ${Legacy_Files})

if(USE_VULKAN)
	file(GLOB SHADER_FILES ${ASSET_ROOT}/Shaders/VK/*.*)
	list(REMOVE_ITEM Header_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechRenderer.h)
	list(REMOVE_ITEM Source_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechRenderer.cpp)
else()
	list(REMOVE_ITEM Header_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechVulkanRenderer.h)
	list(REMOVE_ITEM Source_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechVulkanRenderer.cpp)
endif()

if(PS5)
	source_group("PS5 Files" FILES ${PS5_Files})
	
	file(GLOB_RECURSE ALL_SHADERS CONFIGURE_DEPENDS 
	${COMPILE_ASSET_ROOT}Shaders/PSSL/*.pssl
	)
	list(REMOVE_ITEM Header_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechRenderer.h)
	list(REMOVE_ITEM Source_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechRenderer.cpp)
	
	source_group("Shader Files" FILES ${ALL_SHADERS})
	
	file(GLOB_RECURSE ALL_TEXTURES CONFIGURE_DEPENDS 
		${COMPILE_ASSET_ROOT}Textures/*.jpg 
		${COMPILE_ASSET_ROOT}Textures/*.png 
		${COMPILE_ASSET_ROOT}Textures/*.dds 
	)
	source_group("Texture Files" FILES ${ALL_TEXTURES})
	
	set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
	${PS5_Files}
	${Legacy_Files}
	${ALL_SHADERS}
	${ALL_TEXTURES}
	)
else()
	list(REMOVE_ITEM Header_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechAGCRenderer.h)
	list(REMOVE_ITEM Source_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechAGCRenderer.cpp)
	
	set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
	${Legacy_Files}
	)
endif()

if(USE_VULKAN)
    set(ALL_FILES 
        ${Header_Files}
        ${Source_Files}
        ${SHADER_FILES}
    )

foreach (file ${SHADER_FILES})
    get_filename_component(file_name ${file} NAME)
	get_filename_component(file_ext ${file} EXT)
	
	if(file_ext STREQUAL ".h" OR file_ext STREQUAL ".cpp")
		continue()
	endif()
		
	if( file_ext STREQUAL  ".vert" OR
		file_ext STREQUAL  ".frag" OR
		file_ext STREQUAL  ".comp" OR
		file_ext STREQUAL  ".geom" OR
		file_ext STREQUAL  ".tesc" OR
		file_ext STREQUAL  ".tese" OR
		file_ext STREQUAL  ".rgen" OR
		file_ext STREQUAL  ".rint" OR
		file_ext STREQUAL  ".rahit" OR
		file_ext STREQUAL  ".rchit" OR
		file_ext STREQUAL  ".rmiss" OR
		file_ext STREQUAL  ".rcall" OR
		file_ext STREQUAL  ".task" OR
		file_ext STREQUAL  ".mesh"
	)
		message("Adding custom command to ${file}")
		get_filename_component(file_dir ${file} ABSOLUTE)
        set(SPIRV_OUTPUT ${file_name}.spv)
		set(SPIRV_ABS_INPUT ${file_dir})
		set(SPIRV_ABS_OUTPUT ${file_dir}.spv)
		
		add_custom_command(
			OUTPUT ${SPIRV_ABS_OUTPUT}
			
			COMMENT "Compiling GLSL shader:"
			COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V  ${SPIRV_ABS_INPUT} -o ${SPIRV_ABS_OUTPUT}
            DEPENDS ${file}
			VERBATIM
		)
        list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUTPUT})   
	endif()
endforeach()

endif()

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})
set(ROOT_NAMESPACE CSC8508)

if(PS5)
	target_precompile_headers(${PROJECT_NAME} PRIVATE
		<memory>
		<unordered_set>
		<fstream>
		<iosfwd>
		
		<Matrix.h>
		<Vector.h>
		<Quaternion.h>
		<Plane.h>
		<Frustum.h>
		
		<Camera.h>
		<GameTimer.h>
		<TextureLoader.h>
	
		<vector>
		<map>
		<stack>
		<list>   
		<set>   
		<string>
		<thread>
		<atomic>
		<functional>
		<iostream>
		<chrono>
		<sstream>
		"../NCLCoreClasses/Vector.h"
		"../NCLCoreClasses/Quaternion.h"
		"../NCLCoreClasses/Plane.h"
		"../NCLCoreClasses/Matrix.h"
		"../NCLCoreClasses/GameTimer.h"
	)
else()
	target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vector>
    <map>
    <stack>
    <list>   
	<set>   
	<string>
    <thread>
    <atomic>
    <functional>
    <iostream>
	<chrono>
	<sstream>
	
	"../NCLCoreClasses/Vector.h"
    "../NCLCoreClasses/Quaternion.h"
    "../NCLCoreClasses/Plane.h"
    "../NCLCoreClasses/Matrix.h"
    "../NCLCoreClasses/GameTimer.h"
	)
endif()

if(PS5)
	set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
	VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)"
	)
else()
	use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")

	set_target_properties(${PROJECT_NAME} PROPERTIES
		VS_GLOBAL_KEYWORD "Win32Proj"
	)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
	)
endif()
################################################################################
# Compile definitions
################################################################################
if(MSVC)
	if(PS5)
		target_compile_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
		)
	else()
		target_compile_definitions(${PROJECT_NAME} PRIVATE
        "UNICODE;"
        "_UNICODE" 
        "WIN32_LEAN_AND_MEAN"
        "_WINSOCKAPI_"   
        "_WINSOCK2API_"
        "_WINSOCK_DEPRECATED_NO_WARNINGS"
		)
	endif()
endif()

if(USE_VULKAN)
target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vulkan/vulkan.hpp>
    <SmartTypes.h>
    <VulkanRenderer.h>
    <VulkanShader.h>
    <VulkanShaderBuilder.h>
    <VulkanTexture.h>
    <VulkanMesh.h>
    <VulkanPipelineBuilder.h>
    <VulkanDynamicRenderBuilder.h>

    <VulkanTextureBuilder.h>

    <VulkanDescriptorSetLayoutBuilder.h>
    <VulkanRenderPassBuilder.h>
    <VulkanCompute.h>
    <VulkanComputePipelineBuilder.h>
	<VulkanBufferBuilder.h>
)
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
	if(PS5)
		target_compile_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
		)
	else()
		target_compile_options(${PROJECT_NAME} PRIVATE
			$<$<CONFIG:Release>:
				/Oi;
				/Gy
			>
			/permissive-;
			/std:c++latest;
			/sdl;
			/W3;
			${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
			${DEFAULT_CXX_EXCEPTION_HANDLING};
			/Y-
		)
		target_link_options(${PROJECT_NAME} PRIVATE
			$<$<CONFIG:Release>:
				/OPT:REF;
				/OPT:ICF
			>
		)
	endif()
endif()

################################################################################
# Dependencies
################################################################################
if(MSVC AND NOT PS5)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "Winmm.lib")
endif()
if(PS5)
	target_include_directories (${PROJECT_NAME} 
	PUBLIC ${CMAKE_SOURCE_DIR}/PS5Core
    PUBLIC ${CMAKE_SOURCE_DIR}/GLTFLoader
    PUBLIC ${CMAKE_SOURCE_DIR}/NCLCoreClasses
	PUBLIC ${CMAKE_SOURCE_DIR}/CSC8508CoreClasses
	PUBLIC ${CMAKE_SOURCE_DIR}/EOSFramework
	PUBLIC ${CMAKE_SOURCE_DIR}/OpenGLRendering
	PUBLIC ${CMAKE_SOURCE_DIR}/Event
    PUBLIC ${CMAKE_SOURCE_DIR}/ImGui
	PUBLIC ${CMAKE_SOURCE_DIR}/UISystem
	PUBLIC ${CMAKE_SOURCE_DIR}/SerializedSave
	)
else()
	target_include_directories (${PROJECT_NAME} 
	PUBLIC ${CMAKE_SOURCE_DIR}/OpenGLRendering
    PUBLIC ${CMAKE_SOURCE_DIR}/NCLCoreClasses
	PUBLIC ${CMAKE_SOURCE_DIR}/CSC8508CoreClasses
	PUBLIC ${CMAKE_SOURCE_DIR}/EOSFramework
	PUBLIC ${CMAKE_SOURCE_DIR}/Event
    PUBLIC ${CMAKE_SOURCE_DIR}/ImGui
	PUBLIC ${CMAKE_SOURCE_DIR}/UISystem
	PUBLIC ${CMAKE_SOURCE_DIR}/SerializedSave
	)
endif()

if(PS5)
	# Set the tool and GenerateHeader on PSSL files in ${PSSL_SHADERS}
	set_property(SOURCE ${ALL_SHADERS} PROPERTY VS_TOOL_OVERRIDE WavePsslc)
	set_property(SOURCE ${ALL_SHADERS} PROPERTY VS_SETTINGS "GenerateHeader=false")
	set_property(SOURCE ${ALL_SHADERS} PROPERTY VS_SETTINGS "OutputFileName=${COMPILE_ASSET_ROOT}Shaders/PSSL/%(FileName).ags")
	set_property(SOURCE ${ALL_SHADERS} PROPERTY OUTPUT_NAME "$(Test)%(FileName).ags")

	# Set FileType to Document for all files using the WavePsslc tool
	set_property(TARGET CSC8508 PROPERTY VS_SOURCE_SETTINGS_WavePsslc "FileType=Document")	

	add_custom_target(
		Shaders
	)

	foreach (file ${ALL_TEXTURES})
		get_filename_component(file_name ${file} NAME)
		get_filename_component(file_ext ${file} EXT)
		
		message("Adding custom command to ${file}")
		get_filename_component(file_dir ${file} ABSOLUTE)
		#set(SPIRV_OUTPUT ${file_name}.spv)
		set(TEX_INPUT ${file_dir})
		set(TEX_OUTPUT ${file_dir}.gnf)
		
		message("Reading from ${TEX_INPUT}")
		message("Writing to ${TEX_OUTPUT}")
		
		add_custom_command(
			OUTPUT ${TEX_OUTPUT}
			
			COMMENT "Generating GNF for texture ${file}"
			COMMAND image2gnf.exe -g 1 -i "${file}" -o ${TEX_OUTPUT} -f Bc1UNorm
			DEPENDS ${file}
			VERBATIM
		)
		list(APPEND TEX_GNF_FILES ${TEX_OUTPUT})   
	endforeach()

	add_custom_target(
		Textures
		DEPENDS ON ${TEX_GNF_FILES}
	)
	set(PROJECT_DEPENDENCIES
		CSC8508CoreClasses
		NCLCoreClasses
		PS5Core
		GLTFLoader
		Event
		AudioEngine
		EOSFramework
		ImGui
		UISystem
		enet
		SerializedSave
	)

	add_dependencies(${PROJECT_NAME}
		${PROJECT_DEPENDENCIES}
		Shaders
		Textures
	)
	target_link_libraries(${PROJECT_NAME} 
    PUBLIC ${PROJECT_DEPENDENCIES}
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
	include_directories(${COMPILE_ASSET_ROOT}) 
else() 
	set(PROJECT_DEPENDENCIES
		NCLCoreClasses
		OpenGLRendering
		CSC8508CoreClasses
		EOSFramework
		Event
		AudioEngine
		ImGui
		UISystem
		enet
		SerializedSave
	)
	target_link_libraries(${PROJECT_NAME} 
		PUBLIC ${PROJECT_DEPENDENCIES}
	)
endif()

# Handle `fmod.dll` copying here instead of `AudioEngine`
add_custom_command(TARGET CSC8508 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FMOD_DIR}/lib/x64/fmod.dll"
    "$<TARGET_FILE_DIR:CSC8508>"
)

if(USE_VULKAN)
    include_directories("../VulkanRendering/")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC VulkanRendering)

    add_compile_definitions(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
    add_compile_definitions(VK_NO_PROTOTYPES)

    add_custom_target(
        SHADER_FILES
        DEPENDS ON ${SPIRV_BINARY_FILES}
    )

    add_dependencies(${PROJECT_NAME} SHADER_FILES)
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CSC8508)