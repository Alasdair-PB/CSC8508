set(PROJECT_NAME CSC8508)

file(GLOB_RECURSE Header_Files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
source_group("Header Files" FILES ${Header_Files})
file(GLOB_RECURSE Source_Files "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
source_group("Source Files" FILES ${Source_Files})
if(USE_VULKAN)
	file(GLOB SHADER_FILES ${ASSET_ROOT}/Shaders/VK/*.*)
else()
	list(REMOVE_ITEM Header_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechVulkanRenderer.h)
	list(REMOVE_ITEM Source_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechVulkanRenderer.cpp)
endif()

if(PS5)
	list(REMOVE_ITEM Header_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechRenderer.h)
	list(REMOVE_ITEM Source_Files ${CMAKE_CURRENT_SOURCE_DIR}/GameTechRenderer.cpp)
endif()


set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

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
if(PS5)
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

set(ROOT_NAMESPACE CSC8508)

else()
add_executable(${PROJECT_NAME}  ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE CSC8508)

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
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "UNICODE;"
        "_UNICODE" 
        "WIN32_LEAN_AND_MEAN"
        "_WINSOCKAPI_"   
        "_WINSOCK2API_"
        "_WINSOCK_DEPRECATED_NO_WARNINGS"
    )
endif()

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

################################################################################
# Dependencies
################################################################################
if(MSVC)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "Winmm.lib")
endif()


include_directories("../NCLCoreClasses/")
include_directories("../CSC8508CoreClasses/")
include_directories("../OpenGLRendering/")

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC NCLCoreClasses)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC CSC8508CoreClasses)

if(PS5)
	#include_directories("../PS5Core/")
	#target_link_libraries(${PROJECT_NAME} LINK_PUBLIC PS5Core)
	#include_directories("../PS5Starter/")
	#target_link_libraries(${PROJECT_NAME} LINK_PUBLIC PS5Starter)
else() 
include_directories("../OpenGLRendering/")
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC OpenGLRendering)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC enet)
endif()

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
