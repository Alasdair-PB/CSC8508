cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

project(PS5CSC8508 CXX)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-std=c++17)
add_compile_options(-Wno-pragma-once-outside-header)

if(NOT CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x64")
endif()
message("${CMAKE_VS_PLATFORM_NAME} architecture in use")

################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#[[
if(PS5)
	find_package(Vulkan)
endif() 
]]

set(COMPILE_ASSET_ROOT "${CMAKE_SOURCE_DIR}/Assets/" CACHE STRING "")

set(DEBUG_ASSET_ROOT "${CMAKE_SOURCE_DIR}/Assets/" CACHE STRING "")
set(RELEASE_ASSET_ROOT "${CMAKE_SOURCE_DIR}/Assets/" CACHE STRING "")

add_compile_definitions(ASSETROOTLOCATION="${DEBUG_ASSET_ROOT}") 

set(PS5 CACHE BOOL FORCE)
if(PS5)
    #add_compile_definitions("USEVULKAN") 
	#add_compile_definitions("VK_USE_PLATFORM_WIN32_KHR") 
endif() 

if(MSVC) 
    add_compile_definitions("NOMINMAX")
    add_compile_definitions("WIN32_LEAN_AND_MEAN") 
endif()


add_compile_definitions("USEAGC") 


 
################################################################################
# Sub-projects
################################################################################
add_subdirectory(NCLCoreClasses)
add_subdirectory(GLTFLoader)
add_subdirectory(PS5Core)
add_subdirectory(PS5Starter)
add_subdirectory(CSC8508)
add_subdirectory(CSC8508CoreClasses)

if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PS5Starter)
endif()