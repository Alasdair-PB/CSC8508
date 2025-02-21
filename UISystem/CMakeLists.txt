file(GLOB Headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB Sources "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(UISystem
${Headers}
${Sources})

include_directories(../NCLCoreClasses/)

target_link_libraries(Event NCLCoreClasses)