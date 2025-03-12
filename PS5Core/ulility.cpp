ImGuiStyle g_guiStyle;
ImGuiStyle g_guiStyleWithGamma;
bool g_GUIMode = true;

void initializeImGui() {
    // JP: ImGui初期化。
    // EN: Initilizize ImGui.
    ImGui::CreateContext();
    const ImGui_PS::AllocateFunc imguiAllocateFunc = [](void*, size_t size, size_t alignment) {
        return (void*)allocDmem(size, uint32_t(alignment));
        };
    const ImGui_PS::ReleaseFunc imguiReleaseFunc = [](void*, void*) {
        return;
        };
    ImGui_PS::initialize(nullptr, imguiAllocateFunc, imguiReleaseFunc, NumCommandBuffers, g_UIScale);

    // JP: ガンマ補正ありのレンダーターゲットを想定してデフォルトテーマにデガンマを適用する。
    // EN: Apply degamma to the default theme for gamma-correction enabled render target.
    ImGui::StyleColorsDark(&g_guiStyle);
    g_guiStyleWithGamma = g_guiStyle;
    const auto degamma = [](const ImVec4& color) {
        const auto sRGB_degamma_s = [](float value) {
            if (value <= 0.04045f)
                return value / 12.92f;
            return std::pow((value + 0.055f) / 1.055f, 2.4f);
            };
        return ImVec4(sRGB_degamma_s(color.x),
            sRGB_degamma_s(color.y),
            sRGB_degamma_s(color.z),
            color.w);
        };
    for (int i = 0; i < ImGuiCol_COUNT; ++i) {
        g_guiStyleWithGamma.Colors[i] = degamma(g_guiStyleWithGamma.Colors[i]);
    }
    ImGui::GetStyle() = g_guiStyleWithGamma;



    int32_t ret;

    ret = sceUserServiceInitialize(nullptr);
    SCE_AGC_ASSERT(ret == SCE_OK || ret == SCE_USER_SERVICE_ERROR_ALREADY_INITIALIZED);

    ret = sceUserServiceGetInitialUser(&g_initialUser);
    SCE_AGC_ASSERT(ret == SCE_OK);

    ret = scePadInit();
    SCE_AGC_ASSERT(ret == SCE_OK);

    g_padHandle = scePadOpen(g_initialUser, SCE_PAD_PORT_TYPE_STANDARD, 0, nullptr);
    if (g_padHandle == SCE_PAD_ERROR_ALREADY_OPENED)
        g_padHandle = scePadGetHandle(g_initialUser, SCE_PAD_PORT_TYPE_STANDARD, 0);
    SCE_AGC_ASSERT(g_padHandle >= 0);

    ret = sceSysmoduleLoadModule(SCE_SYSMODULE_MOUSE);
    SCE_AGC_ASSERT(ret == SCE_OK);

    ret = sceMouseInit();
    SCE_AGC_ASSERT(ret == SCE_OK);

    g_mouseHandle = sceMouseOpen(g_initialUser, SCE_MOUSE_PORT_TYPE_STANDARD, 0, nullptr);
    SCE_AGC_ASSERT(g_mouseHandle >= 0);


    g_controller.initialize(g_padHandle, g_mouseHandle);

    g_controller.addDigitalButtonReleasedEvent([](const Controller&, const InputState&, ScePadButtonDataOffset senderButton) {
        if (senderButton == SCE_PAD_BUTTON_TOUCH_PAD)
            g_GUIMode = !g_GUIMode;
        });
}

void updateImGui() {
    static ImVec2 g_lastMousePosition = ImVec2(
        float(g_screen_width / 2) / g_UIScale,
        float(g_screen_height / 2) / g_UIScale);

    g_controller.update();
    InputState inputState;
    g_controller.getState(&inputState);

    ImGui_PS::ControlData controlData;
    ImGui_PS::translate(g_GUIMode ? &inputState.rawData : nullptr,
        g_GUIMode && g_controller.isMouseAvailable() ? ImGui_PS::PadUsage_Navigation : ImGui_PS::PadUsage_MouseEmulation,
        g_GUIMode && g_controller.isMouseAvailable() ? &inputState.mouseData : nullptr,
        g_lastMousePosition, &controlData);

    ImGui_PS::newFrame(
        uint32_t(g_screen_width),
        uint32_t(g_screen_height),
        controlData);
    ImGui::NewFrame();

    g_lastMousePosition = ImGui::GetIO().MousePos;
}

/////////////////////////////////////////////////////////////////////////
// Imgui.
/////////////////////////////////////////////////////////////////////////
void setViewportForImGui(
    sce::Agc::DrawCommandBuffer& dcb,
    const sce::Agc::CxRenderTarget* renderTarget)
{
    struct AdditionalCxRegisters {
        sce::Agc::CxRenderTarget renderTarget;
        sce::Agc::CxRenderTargetMask renderTargetMask;
        sce::Agc::Core::CxScreenViewport screenViewport;
        sce::Agc::CxScreenScissor screenScissor;
    };
    struct CxRegistersToDisableDepthStencil {
        sce::Agc::CxDepthStencilControl depthStencilControl;
        sce::Agc::CxDbRenderControl dbRenderControl;
    };

    sce::Agc::TwoSidedAllocator& mem = dcb;

    AdditionalCxRegisters* const addCxRegs =
        (AdditionalCxRegisters*)mem.allocateTopDown(
            sizeof(AdditionalCxRegisters),
            alignof(sce::Agc::CxRegister));
    const uint32_t width = renderTarget->getWidth();
    const uint32_t height = renderTarget->getHeight();

    addCxRegs->renderTarget = *renderTarget;
    addCxRegs->renderTarget.setSlot(0);
    addCxRegs->renderTargetMask.init().setMask(0, 0xF);
    sce::Agc::Core::setupScreenViewport(&addCxRegs->screenViewport, 0, 0, width, height, 0.5f, 0.5f);
    addCxRegs->screenScissor.init().setLeft(0).setTop(0).setRight(int32_t(width)).setBottom(int32_t(height));

    dcb.setCxRegistersIndirect(
        (sce::Agc::CxRegister*)addCxRegs,
        sizeof(AdditionalCxRegisters) / sizeof(sce::Agc::CxRegister));

    CxRegistersToDisableDepthStencil* const cxRegsDisableDS =
        (CxRegistersToDisableDepthStencil*)mem.allocateTopDown(
            sizeof(CxRegistersToDisableDepthStencil),
            alignof(sce::Agc::CxRegister));
    cxRegsDisableDS->depthStencilControl.init();
    cxRegsDisableDS->dbRenderControl.init();
    dcb.setCxRegistersIndirect(
        (sce::Agc::CxRegister*)cxRegsDisableDS,
        sizeof(CxRegistersToDisableDepthStencil) / sizeof(sce::Agc::CxRegister));
}