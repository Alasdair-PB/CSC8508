file(GLOB Headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB Sources "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(EOSFramework
${Headers}
${Sources})

include_directories(../NCLCoreClasses/)
include_directories(../CSC8508CoreClasses/)

target_link_libraries(Event NCLCoreClasses)
target_link_libraries(Event CSC8508CoreClasses)