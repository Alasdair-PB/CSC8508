cmake_minimum_required(VERSION 3.16.0)
project(AudioEngine CXX)

# Set FMOD base path
set(FMOD_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/FMOD/api/core")

################################################################################
# Set target arch type if empty. Visual studio solution generator provides it.
################################################################################
if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()

if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"))
    #message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
endif()

# Define AudioEngine library
add_library(AudioEngine STATIC
    AudioEngine.cpp
    AudioEngine.h
    AudioObject.h
    AudioListenerComponent.h
    AudioListenerComponent.cpp
    AudioSourceComponent.h
    NetworkedListenerComponent.h
)

# Include directories
target_include_directories(AudioEngine PRIVATE
    "../NCLCoreClasses"
    "../CSC8508CoreClasses"
    "${FMOD_DIR}/inc"
)

# include Maths headers from NCLCore
target_precompile_headers(AudioEngine PRIVATE
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
    "../CSC8508CoreClasses/Transform.h"
    "../CSC8508CoreClasses/Debug.h"
)

################################################################################
# Dependencies
################################################################################

target_link_libraries(AudioEngine LINK_PUBLIC NCLCoreClasses)
target_link_libraries(AudioEngine LINK_PUBLIC CSC8508CoreClasses)

# Ensure FMOD_LIB_DIR exists
message(STATUS "FMOD Include Dir: ${FMOD_DIR}/inc")
target_include_directories(AudioEngine PRIVATE "${FMOD_DIR}/inc")

message(STATUS "FMOD Library Dir: ${FMOD_DIR}/lib/${CMAKE_VS_PLATFORM_NAME}")
target_link_directories(AudioEngine PRIVATE "${FMOD_DIR}/lib/${CMAKE_VS_PLATFORM_NAME}")

message(STATUS "FMOD Library File: ${FMOD_DIR}/lib/${CMAKE_VS_PLATFORM_NAME}/fmod_vc.lib")
target_link_libraries(AudioEngine PRIVATE ${FMOD_DIR}/lib/x64/fmod_vc.lib)

# Set Opus Install Path
set(OPUS_SOURCE_PATH "${CMAKE_SOURCE_DIR}/ThirdParty/opus_source")
set(OPUS_INSTALL_PATH "${CMAKE_SOURCE_DIR}/ThirdParty/opus_install")


# Include Opus headers from the install folder.
target_include_directories(AudioEngine PRIVATE "${OPUS_INSTALL_PATH}/include")


# Bypass any configuration-time file check (using SKIP_OPUS_CHECK)
# and link directly against the imported target (OpusLib) defined in the top-level file.
if(NOT SKIP_OPUS_CHECK)
    set(SKIP_OPUS_CHECK TRUE)
endif()

target_link_libraries(AudioEngine PRIVATE OpusLib)


