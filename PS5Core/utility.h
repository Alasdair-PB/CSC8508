/////////////////////////////////////////////////////////////////////////
// Imgui.
/////////////////////////////////////////////////////////////////////////

// Setup viewport for imgui.
void setViewportForImGui(
	sce::Agc::DrawCommandBuffer& dcb,
	const sce::Agc::CxRenderTarget* renderTarget);

extern ImGuiStyle g_guiStyle;
extern ImGuiStyle g_guiStyleWithGamma;
extern SceUserServiceUserId g_initialUser;
extern int32_t g_padHandle, g_mouseHandle;
extern Controller g_controller;
extern bool g_GUIMode;

void initController();
void initializeImGui();
void updateImGui();

extern ImGuiStyle g_guiStyle;
extern ImGuiStyle g_guiStyleWithGamma;
extern SceUserServiceUserId g_initialUser;
extern int32_t g_padHandle, g_mouseHandle;
extern Controller g_controller;
extern bool g_GUIMode;

void initController();
void initializeImGui();
void updateImGui();