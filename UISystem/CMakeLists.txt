file(GLOB Headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB Sources "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(UISystem
${Headers}
${Sources})

include_directories(../NCLCoreClasses/)
include_directories(../OpenGLRendering/)
include_directories(../ImGui/)
