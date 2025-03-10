set(PROJECT_NAME PS5Starter)

################################################################################
# Source groups
################################################################################

file(GLOB_RECURSE Header_Files CONFIGURE_DEPENDS *.h)
source_group("Header Files" FILES ${Header_Files})

file(GLOB_RECURSE Source_Files CONFIGURE_DEPENDS *.cpp)
source_group("Source Files" FILES ${Source_Files})

file(GLOB_RECURSE ALL_SHADERS CONFIGURE_DEPENDS 
	${COMPILE_ASSET_ROOT}Shaders/PSSL/*.pssl 
)
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
	${ALL_SHADERS}
	${ALL_TEXTURES}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <memory>
    <unordered_set>
    <vector>
    <map>
    <set>

    <string>
    <fstream>
    <iostream>
    <iosfwd>

    <Matrix.h>
    <Vector.h>
    <Quaternion.h>
	
	<Plane.h>
	<Frustum.h>

    <Camera.h>
    <GameTimer.h>
    <TextureLoader.h>
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
	VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)"
)
################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
    )
endif()

################################################################################
# Dependencies
################################################################################
target_include_directories (${PROJECT_NAME} 
    PUBLIC ${CMAKE_SOURCE_DIR}/NCLCoreClasses
    PUBLIC ${CMAKE_SOURCE_DIR}/PS5Core
    PUBLIC ${CMAKE_SOURCE_DIR}/GLTFLoader
	
	PUBLIC ${CMAKE_SOURCE_DIR}/CSC8508CoreClasses
	PUBLIC ${CMAKE_SOURCE_DIR}/CSC8508
)

# Set the tool and GenerateHeader on PSSL files in ${PSSL_SHADERS}
set_property(SOURCE ${ALL_SHADERS} PROPERTY VS_TOOL_OVERRIDE WavePsslc)
set_property(SOURCE ${ALL_SHADERS} PROPERTY VS_SETTINGS "GenerateHeader=false")
set_property(SOURCE ${ALL_SHADERS} PROPERTY VS_SETTINGS "OutputFileName=${COMPILE_ASSET_ROOT}Shaders/PSSL/%(FileName).ags")
set_property(SOURCE ${ALL_SHADERS} PROPERTY OUTPUT_NAME "$(Test)%(FileName).ags")


# Set FileType to Document for all files using the WavePsslc tool
set_property(TARGET PS5Starter PROPERTY VS_SOURCE_SETTINGS_WavePsslc "FileType=Document")	

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
    NCLCoreClasses
    PS5Core
    GLTFLoader
    CSC8508
    CSC8508CoreClasses
)

add_dependencies(${PROJECT_NAME}
    ${PROJECT_DEPENDENCIES}
    Shaders
	Textures
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE ${PROJECT_DEPENDENCIES}
    PRIVATE SceAgcDriver_stub_weak
    PRIVATE SceAgc_stub_weak
    PRIVATE SceVideoOut_stub_weak
    PRIVATE SceAgc_debug_nosubmission
    PRIVATE SceAgcCore_debug_nosubmission
    PRIVATE SceAgcGpuAddress_debug_nosubmission
	
	PRIVATE ScePad_stub_weak
	PRIVATE SceUserService_stub_weak
)

include_directories(${COMPILE_ASSET_ROOT}) 