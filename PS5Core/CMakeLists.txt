set(PROJECT_NAME PS5Core)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "AGCBuffer.h"
    "AGCMesh.h"
	"AGCShader.h"
	"AGCTexture.h"
	"AGCRenderer.h"
	"PS5Window.h"
	"PS5memoryAllocator.h"
	"PS5Controller.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "AGCBuffer.cpp"
    "AGCMesh.cpp"
	"AGCShader.cpp"
	"AGCTexture.cpp"
	"AGCRenderer.cpp"
	"PS5Window.cpp"
	"PS5memoryAllocator.cpp"
	"PS5Controller.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <memory>
    <unordered_set>
    <map>
    <vector>
    <set>
    <string>
    <fstream>
    <iostream>
    <string>
    <iosfwd>
    
    <Matrix.h>
    <Vector.h>
    <Quaternion.h>

    <Quaternion.h>
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)

################################################################################
# Compile and link options
################################################################################
target_include_directories (${PROJECT_NAME} 
    PUBLIC ${CMAKE_SOURCE_DIR}/NCLCoreClasses
)	

################################################################################
# Dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")