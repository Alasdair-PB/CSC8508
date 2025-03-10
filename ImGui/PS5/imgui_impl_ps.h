/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 10.00.00.40-00.00.00.0.1
* Copyright (C) 2020 Sony Interactive Entertainment Inc.
* 
*/
#pragma once

#if !defined(__PROSPERO__)
#error "Only PlayStation(R)5 targets supported"
#endif

#include "imgui.h"

#include <agc.h>
#include <mouse.h>
#include <pad.h>

namespace ImGui_PS {
    using AllocateFunc = void* (*)(void* allocator, size_t size, size_t align);
    using ReleaseFunc = void (*)(void* allocator, void* ptr);

    struct Image {
        sce::Agc::Core::Texture texture;
        sce::Agc::Core::Sampler sampler;

        // JP: renderDrawData()を呼び出すまでこのオブジェクトを保持する必要がある。
        //     (これとは別に、テクスチャーデータはGPUが描画するまで保持する必要がある。)
        // EN: You should keep the lifetime of this object until the call to renderDrawData().
        //     (Apart from this, texture data should be kept until GPU use it.)
        ImTextureID getTextureID() { return (ImTextureID*)this; }
    };

    struct ControlData {
        bool hasGamePad;
        bool enableNavigation;
        float navInputs[ImGuiNavInput_COUNT];
        bool drawCursor;
        ImVec2 mousePosition;
        bool mouseDown[5];
        float mouseWheel;
    };

    IMGUI_IMPL_API void initialize(void* allocatorInstance, AllocateFunc allocateFunc, ReleaseFunc releaseFunc, uint32_t numBuffers,
                                   float uiScale = 1.0f, const char* iniFilePath = "/app0/imgui.ini", const char* logFilePath = "/app0/imgui_log.txt");
    IMGUI_IMPL_API void shutdown();
    IMGUI_IMPL_API void newFrame(uint32_t frameBufferWidth, uint32_t frameBufferHeight, const ControlData &controlData);

    using ImGuiDrawCommandBuffer = sce::Agc::DrawCommandBuffer;
    IMGUI_IMPL_API void renderDrawData(ImGuiDrawCommandBuffer &dcb, ImDrawData* draw_data);

    enum PadUsage {
        PadUsage_Navigation = 0,
        PadUsage_MouseEmulation
    };

    IMGUI_IMPL_API void translate(const ScePadData* padData, PadUsage padUsage, const SceMouseData* mouseData, const ImVec2 &lastMousePos, ControlData* controlData);
}
