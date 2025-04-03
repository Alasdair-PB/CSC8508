cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
if(PS5)
project(PS5CSC8508 CXX)
file(COPY ${CMAKE_SOURCE_DIR}/Assets/ 
     DESTINATION ${CMAKE_SOURCE_DIR}/build/Assets)
file(COPY ${CMAKE_SOURCE_DIR}/sce_module/ 
 DESTINATION ${CMAKE_SOURCE_DIR}/build/sce_module)
else ()
    if (Editor)
        project(EDITOR CXX)
        add_compile_definitions("EDITOR")
    else ()
        project(CSC8508 CXX)
    endif()
endif()

if(EOSBuild)
    add_compile_definitions("EOSBUILD")
endif()


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
# IP Address
################################################################################
if(NOT PS5)
execute_process(
    COMMAND powershell -Command "(Invoke-WebRequest -Uri 'https://api64.ipify.org' -UseBasicParsing).Content"
    OUTPUT_VARIABLE PUBLIC_IP
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "Public IP Address: ${PUBLIC_IP}")
add_definitions(-DPUBLIC_IP_ADDRESS=${PUBLIC_IP})
endif()

################################################################################
# Third-Party Integration
################################################################################

# Create Third Party Directory
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/ThirdParty)


######################################
# FMOD
######################################


# Set FMOD path
if(PS5)
set(FMOD_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/FMOD PS5/api/core")
else()
set(FMOD_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/FMOD/api/core")
endif()

# Check if FMOD is installed
if(NOT EXISTS "${FMOD_DIR}")
    message(FATAL_ERROR "FMOD not found. Please install FMOD in the ThirdParty directory.")
endif()

# Include FMOD headers globally
include_directories("${FMOD_DIR}/inc")

######################################
# OPUS
######################################


# Set Opus Paths and ensure the directory exists.
set(OPUS_SOURCE_PATH "${CMAKE_SOURCE_DIR}/ThirdParty/opus_source")
if(NOT PS5)
set(OPUS_INSTALL_PATH "${CMAKE_SOURCE_DIR}/ThirdParty/opus_install")
file(MAKE_DIRECTORY ${OPUS_INSTALL_PATH})

# Include ExternalProject module.
include(ExternalProject)

# Define an external project to build Opus.
ExternalProject_Add(opus_build
    GIT_REPOSITORY "https://github.com/xiph/opus.git"     # Use the Opus repository.
    GIT_TAG "v1.3.1"                                      # Use a specific version of Opus.
    SOURCE_DIR "${OPUS_SOURCE_PATH}"                      # Use the existing opus_source folder.
    BINARY_DIR "${OPUS_INSTALL_PATH}/build"               # Build output will be placed here.
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=${OPUS_INSTALL_PATH}       # Install files into opus_install.
        -DCMAKE_BUILD_TYPE=Release
        -G "Visual Studio 17 2022"
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target install
)

# Create an imported target for the built Opus library.
add_library(OpusLib STATIC IMPORTED GLOBAL)
set_target_properties(OpusLib PROPERTIES
    IMPORTED_LOCATION "${OPUS_INSTALL_PATH}/lib/opus.lib"
)
else()
set(OPUS_INSTALL_PATH "${CMAKE_SOURCE_DIR}/ThirdParty/opus_install_PS5")
file(MAKE_DIRECTORY ${OPUS_INSTALL_PATH})

# Include ExternalProject module.
include(ExternalProject)

# Define an external project to build Opus.
ExternalProject_Add(opus_build
    GIT_REPOSITORY "https://github.com/xiph/opus.git"     # Use the Opus repository.
    GIT_TAG "v1.3.1"                                      # Use a specific version of Opus.
    SOURCE_DIR "${OPUS_SOURCE_PATH}"                      # Use the existing opus_source folder.
    BINARY_DIR "${OPUS_INSTALL_PATH}/build"               # Build output will be placed here.
    CMAKE_ARGS 
		"-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
        -DCMAKE_INSTALL_PREFIX=${OPUS_INSTALL_PATH}       # Install files into opus_install.
        -DCMAKE_BUILD_TYPE=Release
        -G "Visual Studio 17 2022"
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target install
)
# Create an imported target for the built Opus library.
add_library(OpusLib STATIC IMPORTED GLOBAL)
set_target_properties(OpusLib PROPERTIES
    IMPORTED_LOCATION "${OPUS_INSTALL_PATH}/lib/libopus.a"
)
endif()
# After building, the Opus headers will be in ${OPUS_INSTALL_PATH}/include
include_directories("${OPUS_INSTALL_PATH}/include")
# Set flag so subprojects (like AudioEngine) skip configuration-time checks.
set(SKIP_OPUS_CHECK TRUE)

######################################
# EOS
######################################

# Set EOS path
set(EOS_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/EOS/SDK")

# Check if EOS is installed
if(NOT EXISTS "${EOS_DIR}")
    message(FATAL_ERROR "EOS not found. Please install EOS in the ThirdParty directory.")
endif()

# Include EOS headers globally
include_directories("${EOS_DIR}/Include")

################################################################################
# Sub-projects & Linking
################################################################################
add_subdirectory(NCLCoreClasses)
add_subdirectory(CSC8508)
add_subdirectory(CSC8508CoreClasses)
add_subdirectory(ImGui)
add_subdirectory(Event)
add_subdirectory(EOSFramework)
add_subdirectory(SerializedSave)
add_subdirectory(UISystem)
add_subdirectory(AudioEngine)

if(PS5)
	add_subdirectory(PS5Core)
	add_subdirectory(GLTFLoader)
	add_compile_options(-Wno-pragma-once-outside-header)
elseif(USE_VULKAN)
    add_subdirectory(VulkanRendering)
else()
	add_subdirectory(OpenGLRendering)
endif()

# Link Audio Engine
if(NOT PS5)
if(TARGET AudioEngine)
    add_dependencies(AudioEngine opus_build)
endif()

# Link FMOD
target_link_libraries(AudioEngine PRIVATE "${FMOD_DIR}/lib/${CMAKE_VS_PLATFORM_NAME}/fmod_vc.lib")
endif()
target_link_libraries(CSC8508 PRIVATE AudioEngine)
target_include_directories(CSC8508 PRIVATE "${FMOD_DIR}/inc")

# Link Opus
target_link_libraries(AudioEngine PRIVATE ${OPUS_LIBRARY})

# Link EOS Engine
if(EOSBuild)
target_link_libraries(CSC8508 PRIVATE "${EOS_DIR}/Lib/EOSSDK-Win64-Shipping.lib")
else()
target_link_libraries(CSC8508 PRIVATE "${EOS_DIR}/Lib/EOSSDK-Win64-Shipping.lib")
endif()
target_include_directories(CSC8508 PRIVATE "${EOS_DIR}/Include")

if(MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CSC8508)
endif()
