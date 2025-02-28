cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
project(PS5CSC8508 CXX)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-std=c++20)

if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
	set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

if(CMAKE_GENERATOR_PLATFORM STREQUAL "Prospero")
    set(PS5 TRUE)
	add_compile_definitions("USE_PS5")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti") #For dynamic cast
	add_compile_definitions("USEAGC")
endif()

################################################################################
# Nuget packages function stub.
################################################################################
function(use_package TARGET PACKAGE VERSION)
    message(WARNING "No implementation of use_package. Create yours. "
                    "Package \"${PACKAGE}\" with version \"${VERSION}\" "
                    "for target \"${TARGET}\" is ignored!")
endfunction()

################################################################################
# Common utils
################################################################################
include(CMake/Utils.cmake)

################################################################################
# Additional Global Settings(add specific info there)
################################################################################
include(CMake/GlobalSettingsInclude.cmake OPTIONAL)

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(COMPILE_ASSET_ROOT "${CMAKE_SOURCE_DIR}/Assets/" CACHE STRING "")

set(DEBUG_ASSET_ROOT "${CMAKE_SOURCE_DIR}/Assets/" CACHE STRING "")
set(RELEASE_ASSET_ROOT "${CMAKE_SOURCE_DIR}/Assets/" CACHE STRING "")

message(STATUS "Compile Asset Root: ${COMPILE_ASSET_ROOT}")
message(STATUS "Debug Asset Root: ${DEBUG_ASSET_ROOT}")
message(STATUS "Release Asset Root: ${RELEASE_ASSET_ROOT}")

add_compile_definitions(ASSETROOTLOCATION="${DEBUG_ASSET_ROOT}") 

if(MSVC) 
    add_compile_definitions("NOMINMAX")
    add_compile_definitions("WIN32_LEAN_AND_MEAN") 
endif()

 
################################################################################
# Sub-projects
################################################################################
add_subdirectory(NCLCoreClasses)
add_subdirectory(CSC8508)
add_subdirectory(CSC8508CoreClasses)
if(PS5)
	add_subdirectory(PS5Core)
	add_subdirectory(GLTFLoader)
	add_compile_options(-Wno-pragma-once-outside-header)
else()
	add_subdirectory(OpenGLRendering)
endif()

if(MSVC)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CSC8508)
endif()