/* SIE CONFIDENTIAL
PlayStation(R)5 Programmer Tool Runtime Library Release 10.00.00.40-00.00.00.0.1
* Copyright (C) 2019 Sony Interactive Entertainment Inc.
* 
*/

#include "imgui_impl_ps.h"
#include "sampleutil\ui_framework\imgui\imgui_libfont.h"

#include <sdk_version.h>

#include <algorithm>
#include <chrono>

#include "imgui_impl_ps_shader_common.h"

#define devPrintf(fmt, ...) printf(fmt, ##__VA_ARGS__);

#ifdef _DEBUG
#   define IMGUI_PS_ENABLE_ASSERT
#endif

#ifdef IMGUI_PS_ENABLE_ASSERT
#   define Assert(expr, fmt, ...) \
    do { \
        if (!(expr)) { \
            devPrintf("%s @%s: %u:\n", #expr, __FILE__, __LINE__); \
            devPrintf(fmt"\n", ##__VA_ARGS__); \
            abort(); \
        } \
    } while (0)
#else
#   define Assert(expr, fmt, ...) (void)(expr)
#endif


static_assert(sizeof(ImGui_PS::EmbeddedShader::Attribute) == sizeof(ImDrawVert),
              "Vertex Attribute must match ImDrawVert");


namespace ImGui_PS {
    namespace EmbeddedShader {
#   define IMGUI_AGC_DECLARE_SHADER(basename)\
        extern char basename ## _header[];\
        extern const char basename ## _text[];\
        sce::Agc::Shader* basename

        IMGUI_AGC_DECLARE_SHADER(shader_vv);
        IMGUI_AGC_DECLARE_SHADER(shader_p);
#   undef IMGUI_AGC_DECLARE_SHADER
    }

    namespace MemType {
        enum Value {
            Cached = SCE_KERNEL_MTYPE_C,
            Cached_Shared = SCE_KERNEL_MTYPE_C_SHARED,
            // Uncached = SCE_KERNEL_MTYPE_UC,
            // Cached_RO = SCE_KERNEL_MTYPE_C_RO,
            // Cached_RO_Shared = SCE_KERNEL_MTYPE_C_RO_SHARED,
            SystemDefault
        };
    }

    class StackAllocator {
        MemType::Value m_type;
        bool m_isInitialized;
        bool m_regionIsProvided;
        enum { kMaximumAllocations = 8192 };
        uint8_t* m_heads[kMaximumAllocations];
        uint8_t* m_base;
        size_t m_alignment;
        off_t m_offset;
        size_t m_size;

        uint32_t m_numAllocations;
        off_t m_top;
        uint32_t m_refPoints[64];
        uint32_t m_refDepth;
    public:
        StackAllocator() : m_isInitialized(false) {}
        ~StackAllocator() {
            if (m_isInitialized)
                finalize();
        }

        void initialize(MemType::Value type, bool allowCPUWrite, size_t size, size_t align = 2 * 1024 * 1024);
        void initialize(void* region, size_t size, size_t align);
        void finalize() {
            Assert(m_isInitialized == true, "Cannot terminate a non-initialized instance.");
            if (!m_regionIsProvided) {
                int ret = sceKernelReleaseDirectMemory(m_offset, m_size);
                Assert(ret == 0, "Failed to release direct memory. : 0x%08x", ret);
            }
            m_isInitialized = false;
        }

        void *allocate(size_t size, size_t alignment);
        void release(void* pointer);

        template <typename T>
        T* allocate(uint32_t numElems = 1, size_t alignment = __alignof(T)) {
            return (T*)allocate(sizeof(T) * numElems, alignment);
        }

        template <typename T>
        void destructAndRelease(T* ptr) {
            (ptr)->~T();
            release(ptr);
        }

        void push();
        void pop();

        void releaseAll();

        void getCurrentUsage(size_t* size, uint32_t* numAllocs) const {
            *size = m_top;
            *numAllocs = m_numAllocations;
        }
    };

    void StackAllocator::initialize(MemType::Value type, bool allowCPUWrite, size_t size, size_t align) {
        Assert(m_isInitialized == false, "This instance has already initialized.");
        Assert(type != MemType::SystemDefault, "\"type\" must not be the system default allocator.");

        m_regionIsProvided = false;

        m_type = type;
        m_size = size;
        m_alignment = align;
        m_base = 0;

        int32_t retSys = sceKernelAllocateDirectMemory(0, SCE_KERNEL_MAIN_DMEM_SIZE, m_size, m_alignment, m_type, &m_offset);
        Assert(retSys == 0, "Failed to allocate direct memory. : 0x%08x", retSys);

        int32_t protectionFlags = SCE_KERNEL_PROT_CPU_READ | SCE_KERNEL_PROT_GPU_RW;
        if (allowCPUWrite)
            protectionFlags |= SCE_KERNEL_PROT_CPU_WRITE;
        retSys = sceKernelMapDirectMemory(&reinterpret_cast<void*&>(m_base), m_size, protectionFlags, 0, m_offset, m_alignment);
        Assert(retSys == 0, "Failed to map direct memory. : 0x%08x", retSys);

        m_numAllocations = 0;
        m_top = 0;
        m_refDepth = 0;

        m_isInitialized = true;
    }

    void StackAllocator::initialize(void* region, size_t size, size_t align) {
        Assert(((uintptr_t)region & (align - 1)) == 0, "Provided memory region doesn't satisfy the alignment.");

        m_regionIsProvided = true;

        m_size = size;
        m_alignment = align;
        m_base = (uint8_t*)region;

        m_offset = 0;

        m_numAllocations = 0;
        m_top = 0;
        m_refDepth = 0;

        m_isInitialized = true;
    }

    void* StackAllocator::allocate(size_t size, size_t alignment) {
        Assert(m_numAllocations < kMaximumAllocations, "The number of allocations reached the limit.");
        const size_t mask = alignment - 1;
        m_top = (m_top + mask) & ~mask;
        void* result = m_heads[m_numAllocations++] = m_base + m_top;
        m_top += size;
        Assert(m_top <= static_cast<off_t>(m_size), "The allocation spilled out.");
        return result;
    }

    void StackAllocator::release(void* pointer) {
        Assert(m_numAllocations > 0, "No allocation to release anymore.");
        uint8_t* lastPointer = m_heads[--m_numAllocations];
        Assert(lastPointer == pointer, "\"pointer\": %p is not the top.", pointer);
        m_top = lastPointer - m_base; // this may not rewind far enough if subsequent allocation has looser alignment than previous one
    }

    void StackAllocator::push() {
        m_refPoints[m_refDepth++] = m_numAllocations;
    }

    void StackAllocator::pop() {
        uint32_t refPoint = m_refPoints[--m_refDepth];
        while (m_numAllocations > refPoint) {
            release(m_heads[m_numAllocations - 1]);
        }
    }

    void StackAllocator::releaseAll() {
        m_numAllocations = 0;
        m_top = 0;
        m_refDepth = 0;
    }



    enum Key {
        Key_Tab = 0,
        Key_LeftArrow,
        Key_RightArrow,
        Key_UpArrow,
        Key_DownArrow,
        Key_PageUp,
        Key_PageDown,
        Key_Home,
        Key_End,
        Key_Insert,
        Key_Delete,
        Key_Backspace,
        Key_Space,
        Key_Enter,
        Key_Escape,
        NumKeys
    };



    struct StaticRenderStates {
        struct CxRegisters {
            sce::Agc::CxBlendControl blendControl;
            sce::Agc::CxPrimitiveSetup primitiveSetup;
            sce::Agc::CxDepthStencilControl depthStencilControl;
            sce::Agc::CxScanModeControl scanModeControl;

            sce::Agc::CxShaderLinkage shaderLinkage;
        };
        struct ShRegisters {
        };
        struct UcRegisters {
            sce::Agc::UcPrimitiveState shaderLinkage;
        };
        // registers that seem better to be reset
        struct CleanCxRegisters {
            sce::Agc::CxBlendControl blendControl;
            sce::Agc::CxScanModeControl scanModeControl;
        };

        sce::Agc::CxRegister* cxRegs;
        sce::Agc::ShRegister* shRegs;
        sce::Agc::UcRegister* ucRegs;
        sce::Agc::CxRegister* cleanCxRegs;
        uint32_t numCxRegs;
        uint32_t numShRegs;
        uint32_t numUcRegs;
        uint32_t numCleanCxRegs;

        void setRegisters(sce::Agc::DrawCommandBuffer &dcb) {
            dcb.setCxRegistersIndirect(cxRegs, numCxRegs);
            dcb.setShRegistersIndirect(shRegs, numShRegs);
            dcb.setUcRegistersIndirect(ucRegs, numUcRegs);
        }

        void setCleanRegisters(sce::Agc::DrawCommandBuffer &dcb) {
            dcb.setCxRegistersIndirect(cleanCxRegs, numCleanCxRegs);
        }
    };



    static uint32_t s_numBuffers;
    static void** s_frameMemRegions;
    static StackAllocator* s_frameMems;
    static uint32_t s_bufferIndex;

    static StaticRenderStates s_renderStates;

    static Image s_fontsImage;

    static sce::Agc::Core::VertexAttribute* s_vertexAttributeTable;
    static sce::Agc::IndexSize s_indexSize;

    using clock = std::chrono::system_clock;
    static clock::time_point s_lastTimePoint;

    static SceLibcMspace s_cpuMemory = nullptr;
    static void *s_cpuMemoryMappedAddr = nullptr;


    static void* s_allocator;
    static AllocateFunc s_allocateFunc;
    static ReleaseFunc s_releaseFunc;

    template <typename T>
    T* allocateCPU(size_t num = 1) {
        return (T*)s_allocateFunc(s_allocator, sizeof(T) * num, alignof(T));
    }
    static void* allocateCPU(size_t size, size_t alignment) {
        return s_allocateFunc(s_allocator, size, alignment);
    }
    static void releaseCPU(void* ptr) {
        s_releaseFunc(s_allocator, ptr);
    }

    template <typename T>
    T* allocateGPU(size_t num = 1) {
        return (T*)s_allocateFunc(s_allocator, sizeof(T) * num, alignof(T));
    }
    static void* allocateGPU(size_t size, size_t alignment) {
        return s_allocateFunc(s_allocator, size, alignment);
    }
    static void releaseGPU(void* ptr) {
        s_releaseFunc(s_allocator, ptr);
    }

    void initialize(void* allocatorInstance, AllocateFunc allocateFunc, ReleaseFunc releaseFunc, uint32_t numBuffers,
                    float uiScale, const char* iniFilePath, const char* logFilePath) {
        using namespace sce::Agc;
        using namespace EmbeddedShader;

        s_allocator = allocatorInstance;
        s_allocateFunc = allocateFunc;
        s_releaseFunc = releaseFunc;

        s_numBuffers = numBuffers;
        s_frameMemRegions = new void*[s_numBuffers];
        s_frameMems = new StackAllocator[s_numBuffers];
        const size_t FrameMemSize = 4 * 1024 * 1024;
        const size_t FrameMemAlignment = 16;
        for (int i = 0; i < s_numBuffers; ++i) {
            s_frameMemRegions[i] = allocateGPU(FrameMemSize, FrameMemAlignment);
            s_frameMems[i].initialize(s_frameMemRegions[i], FrameMemSize, FrameMemAlignment);
        }
        s_bufferIndex = -1;

        SceError err;

        // create static register states.
        {
            err = createShader(&shader_vv, shader_vv_header, shader_vv_text);
            Assert(err == SCE_OK, "failed to create a shader. %d", err);
            err = createShader(&shader_p, shader_p_header, shader_p_text);
            Assert(err == SCE_OK, "failed to create a shader. %d", err);

            Shader** shaders[] = {
                &shader_vv,
                &shader_p
            };
            const uint32_t numShaders = sizeof(shaders) / sizeof(shaders[0]);

            s_renderStates.numCxRegs = sizeof(StaticRenderStates::CxRegisters) / sizeof(CxRegister);
            s_renderStates.numShRegs = sizeof(StaticRenderStates::ShRegisters) / sizeof(ShRegister);
            s_renderStates.numUcRegs = sizeof(StaticRenderStates::UcRegisters) / sizeof(UcRegister);
            s_renderStates.numCleanCxRegs = sizeof(StaticRenderStates::CleanCxRegisters) / sizeof(CxRegister);
            for (int i = 0; i < numShaders; ++i) {
                Shader* shader = *shaders[i];
                s_renderStates.numCxRegs += shader->m_numCxRegisters;
                s_renderStates.numShRegs += shader->m_numShRegisters;
            }

            s_renderStates.cxRegs = allocateGPU<CxRegister>(s_renderStates.numCxRegs);
            s_renderStates.shRegs = allocateGPU<ShRegister>(s_renderStates.numShRegs);
            s_renderStates.ucRegs = allocateGPU<UcRegister>(s_renderStates.numUcRegs);
            s_renderStates.cleanCxRegs = allocateGPU<CxRegister>(s_renderStates.numCleanCxRegs);

            auto cxRegs = (StaticRenderStates::CxRegisters*)s_renderStates.cxRegs;

            cxRegs->blendControl.init()
                .setBlend(CxBlendControl::Blend::kEnable)
                .setColorBlendFunc(CxBlendControl::ColorBlendFunc::kAdd)
                .setColorSourceMultiplier(CxBlendControl::ColorSourceMultiplier::kSrcAlpha)
                .setColorDestMultiplier(CxBlendControl::ColorDestMultiplier::kOneMinusSrcAlpha)
                .setAlphaBlendFunc(CxBlendControl::AlphaBlendFunc::kAdd)
                .setAlphaSourceMultiplier(CxBlendControl::AlphaSourceMultiplier::kSrcAlpha)
                .setAlphaDestMultiplier(CxBlendControl::AlphaDestMultiplier::kOneMinusSrcAlpha)
                .setSlot(0);
            cxRegs->primitiveSetup.init()
                .setCullFace(CxPrimitiveSetup::CullFace::kNone)
                .setPolygonMode(CxPrimitiveSetup::PolygonMode::kEnable)
                .setFrontPolygonMode(CxPrimitiveSetup::FrontPolygonMode::kFill)
                .setBackPolygonMode(CxPrimitiveSetup::BackPolygonMode::kFill);
            cxRegs->depthStencilControl.init()
                .setDepth(CxDepthStencilControl::Depth::kDisable);
            cxRegs->scanModeControl.init().setViewportScissor(CxScanModeControl::ViewportScissor::kEnable);

            auto shaderCxRegs = (CxRegister*)(cxRegs + 1);
            for (int i = 0; i < numShaders; ++i) {
                Shader* shader = *shaders[i];
                std::copy_n(shader->m_cxRegisters, shader->m_numCxRegisters, shaderCxRegs);
                shaderCxRegs += shader->m_numCxRegisters;
            }

            auto shaderShRegs = s_renderStates.shRegs;
            for (int i = 0; i < numShaders; ++i) {
                Shader* shader = *shaders[i];
                std::copy_n(shader->m_shRegisters, shader->m_numShRegisters, shaderShRegs);
                shaderShRegs += shader->m_numShRegisters;
            }

            auto ucRegs = (StaticRenderStates::UcRegisters*)s_renderStates.ucRegs;
            err = sce::Agc::Core::linkShaders(&cxRegs->shaderLinkage, &ucRegs->shaderLinkage,
                                              nullptr, shader_vv, shader_p,
                                              sce::Agc::UcPrimitiveType::Type::kTriList);

            auto cleanCxRegs = (StaticRenderStates::CleanCxRegisters*)s_renderStates.cleanCxRegs;
            cleanCxRegs->blendControl.init();
            cleanCxRegs->scanModeControl.init();
        }

        // create fonts atlas.
        {
          size_t size = 128 * 1024 * 1024;
          s_cpuMemoryMappedAddr = allocateCPU(size, 64);
          Assert(s_cpuMemoryMappedAddr != nullptr, "Failed to allocate font memory");

          s_cpuMemory = sceLibcMspaceCreate("cpu memory", s_cpuMemoryMappedAddr, size, 0);
          Assert(s_cpuMemory != nullptr, "Failed to allocate font memory");

          ImGuiIO& io = ImGui::GetIO();

          float defaultFontSize = 12.0f * uiScale;
          ImFont *defaultFont = ImGuiLibFont::AddSystemFont(io.Fonts, defaultFontSize);
          Assert(defaultFont != nullptr, "failed to add default system font.");

          int ret = ImGuiLibFont::Initialize();
          Assert(ret == SCE_OK, "failed to initialize a imgui font atlas.");

          ImGuiLibFont::BuildFontAtlas(io.Fonts, s_cpuMemory);
        }

        // create fonts texture.
        {
            // Build texture atlas
            ImGuiIO& io = ImGui::GetIO();
            unsigned char* pixels;
            int width, height;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

            Core::TextureSpec texSpec;
            texSpec.init();
            texSpec.m_type = Core::Texture::Type::k2d;
            texSpec.m_width = width;
            texSpec.m_height = height;
            texSpec.m_depth = 1;
            texSpec.m_numMips = 1;
            texSpec.m_format = Core::DataFormat{ Core::TypedFormat::k8_8_8_8UNorm, Core::Swizzle::kRGBA_R4S4 };
            texSpec.m_tileMode = sce::AgcGpuAddress::TileMode::kLinear;

            auto sizeAlign = Core::getSize(&texSpec);
            texSpec.m_dataAddress = allocateGPU(sizeAlign.m_size, sizeAlign.m_align);
            std::copy_n(pixels, 4 * width * height, (unsigned char*)texSpec.m_dataAddress);

            err = Core::initialize(&s_fontsImage.texture, &texSpec);
            Assert(err == SCE_OK, "failed to initialize a texture.");

            s_fontsImage.sampler.init()
                .setXyFilterMode(Core::Sampler::FilterMode::kBilinear);

            // Store our identifier
            io.Fonts->TexID = s_fontsImage.getTextureID();
        }

        // create vertex-related static contents.
        {
            s_vertexAttributeTable = allocateGPU<sce::Agc::Core::VertexAttribute>(3);
            s_vertexAttributeTable[0] = sce::Agc::Core::VertexAttribute{ 0, sce::Agc::Core::VertexAttribute::Format::k32_32Float, offsetof(Attribute, posInModel), sce::Agc::Core::VertexAttribute::Index::kVertexId };
            s_vertexAttributeTable[1] = sce::Agc::Core::VertexAttribute{ 0, sce::Agc::Core::VertexAttribute::Format::k32_32Float, offsetof(Attribute, texCoord), sce::Agc::Core::VertexAttribute::Index::kVertexId };
            s_vertexAttributeTable[2] = sce::Agc::Core::VertexAttribute{ 0, sce::Agc::Core::VertexAttribute::Format::k8_8_8_8UNorm, offsetof(Attribute, color), sce::Agc::Core::VertexAttribute::Index::kVertexId };

            if (sizeof(ImDrawIdx) == sizeof(uint8_t))
                s_indexSize = sce::Agc::IndexSize::k8;
            else if (sizeof(ImDrawIdx) == sizeof(uint16_t))
                s_indexSize = sce::Agc::IndexSize::k16;
            else
                s_indexSize = sce::Agc::IndexSize::k32;
        }

        s_lastTimePoint = clock::time_point();

        ImGuiIO& io = ImGui::GetIO();

        io.BackendPlatformName = "imgui_impl_prospero";
        io.MouseDrawCursor = false;

        io.KeyMap[ImGuiKey_Tab] = Key_Tab;
        io.KeyMap[ImGuiKey_LeftArrow] = Key_LeftArrow;
        io.KeyMap[ImGuiKey_RightArrow] = Key_RightArrow;
        io.KeyMap[ImGuiKey_UpArrow] = Key_UpArrow;
        io.KeyMap[ImGuiKey_DownArrow] = Key_DownArrow;
        io.KeyMap[ImGuiKey_PageUp] = Key_PageUp;
        io.KeyMap[ImGuiKey_PageDown] = Key_PageDown;
        io.KeyMap[ImGuiKey_Home] = Key_Home;
        io.KeyMap[ImGuiKey_End] = Key_End;
        io.KeyMap[ImGuiKey_Insert] = Key_Insert;
        io.KeyMap[ImGuiKey_Delete] = Key_Delete;
        io.KeyMap[ImGuiKey_Backspace] = Key_Backspace;
        io.KeyMap[ImGuiKey_Space] = Key_Space;
        io.KeyMap[ImGuiKey_Enter] = Key_Enter;
        io.KeyMap[ImGuiKey_Escape] = Key_Escape;

        io.IniFilename = iniFilePath;
        io.LogFilename = logFilePath;

        io.BackendRendererName = "imgui_impl_ps";
    }

    void shutdown() {
        // delete vertex-related static contents
        {
            releaseGPU(s_vertexAttributeTable);
        }

        // delete fonts texture.
        {
            ImGuiIO &io = ImGui::GetIO();
            io.Fonts->TexID = 0;

            releaseGPU(s_fontsImage.texture.getDataAddress());
        }

        // delete fonts atlas.
        {
          ImGuiIO &io = ImGui::GetIO();
          ImGuiLibFont::ClearSystemFont(io.Fonts);
          ImGuiLibFont::Finalize();

          sceLibcMspaceDestroy(s_cpuMemory);
          releaseCPU(s_cpuMemoryMappedAddr);
        }

        // delete static register states.
        {
            releaseGPU(s_renderStates.cleanCxRegs);
            releaseGPU(s_renderStates.ucRegs);
            releaseGPU(s_renderStates.shRegs);
            releaseGPU(s_renderStates.cxRegs);
        }

        for (int i = s_numBuffers - 1; i >= 0; --i) {
            s_frameMems[i].finalize();
            releaseGPU(s_frameMemRegions[i]);
        }
        delete[] s_frameMems;
        delete[] s_frameMemRegions;

        s_releaseFunc = nullptr;
        s_allocateFunc = nullptr;
        s_allocator = nullptr;
    }

    void newFrame(uint32_t frameBufferWidth, uint32_t frameBufferHeight, const ControlData &controlData) {
        s_bufferIndex = (s_bufferIndex + 1) % s_numBuffers;
        //size_t usedSize;
        //uint32_t numAllocs;
        //s_frameMems[s_bufferIndex].getCurrentUsage(&usedSize, &numAllocs);
        //printf("%u, %lu[B], %u allocs\n", s_bufferIndex, usedSize, numAllocs);
        s_frameMems[s_bufferIndex].releaseAll();

        ImGuiIO& io = ImGui::GetIO();
        IM_ASSERT(io.Fonts->IsBuilt());

        // setup display size.
        io.DisplaySize = ImVec2((float)frameBufferWidth, (float)frameBufferHeight);

        // setup time step.
        auto curTimePoint = clock::now();
        float deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(curTimePoint - s_lastTimePoint).count() * 1e-3f;
        io.DeltaTime = s_lastTimePoint > clock::time_point(clock::duration::zero()) ? deltaTime : (1.0f / 60.0f);
        s_lastTimePoint = curTimePoint;

        if (controlData.enableNavigation)
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        else
            io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;

        // handle pad inputs.
        {
            if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) && controlData.hasGamePad) {
                std::copy_n(controlData.navInputs, ImGuiNavInput_COUNT, io.NavInputs);

                io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
            }

            if (!controlData.hasGamePad)
                io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
        }

        // handle mouse inputs.
        {
            io.MouseDrawCursor = controlData.drawCursor;
            io.MousePos = controlData.mousePosition;
            io.MousePos.x = std::min(io.DisplaySize.x, std::max(0.0f, io.MousePos.x));
            io.MousePos.y = std::min(io.DisplaySize.y, std::max(0.0f, io.MousePos.y));
            std::copy_n(controlData.mouseDown, IM_ARRAYSIZE(controlData.mouseDown), io.MouseDown);
            io.MouseWheel = controlData.mouseWheel;
        }

        // handle keyboard inputs.
        {
            std::fill_n(io.KeysDown, IM_ARRAYSIZE(io.KeysDown), false);
            // ...
        }
    }

    template <typename CxRegisterType>
    inline void setCxRegistersDirect(sce::Agc::DrawCommandBuffer &dcb, const CxRegisterType &reg) {
        for (int i = 0; i < sizeof(CxRegisterType) / sizeof(sce::Agc::CxRegister); ++i)
            dcb.setCxRegisterDirect(reg.m_regs[i]);
    }

    template <typename UcRegisterType>
    inline void setUcRegistersDirect(sce::Agc::DrawCommandBuffer &dcb, const UcRegisterType &reg) {
        for (int i = 0; i < sizeof(UcRegisterType) / sizeof(sce::Agc::UcRegister); ++i)
            dcb.setUcRegisterDirect(reg.m_regs[i]);
    }

    void renderDrawData(ImGuiDrawCommandBuffer &dcb, ImDrawData* draw_data) {
        using namespace sce::Agc;
        SceError err;

        StackAllocator &frameMem = s_frameMems[s_bufferIndex];

        // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
        ImGuiIO& io = ImGui::GetIO();
        int fb_width = (int)(draw_data->DisplaySize.x * io.DisplayFramebufferScale.x);
        int fb_height = (int)(draw_data->DisplaySize.y * io.DisplayFramebufferScale.y);
        if (fb_width <= 0 || fb_height <= 0)
            return;
        draw_data->ScaleClipRects(io.DisplayFramebufferScale);

        s_renderStates.setRegisters(dcb);

        using namespace EmbeddedShader;

        // setup vertex and index buffer.
        auto vertexBuffer = frameMem.allocate<sce::Agc::Core::Buffer>();
        auto vertexData = frameMem.allocate<ImDrawVert>(draw_data->TotalVtxCount);
        auto indexData = frameMem.allocate<ImDrawIdx>(draw_data->TotalIdxCount);
        {
            ImDrawVert* vertHead = vertexData;
            ImDrawIdx* idxHead = indexData;
            for (int i = 0; i < draw_data->CmdListsCount; ++i) {
                const ImDrawList* cmdList = draw_data->CmdLists[i];
                std::copy_n(cmdList->VtxBuffer.begin(), cmdList->VtxBuffer.size(), vertHead);
                std::copy_n(cmdList->IdxBuffer.begin(), cmdList->IdxBuffer.size(), idxHead);
                vertHead += cmdList->VtxBuffer.size();
                idxHead += cmdList->IdxBuffer.size();
            }

            sce::Agc::Core::BufferSpec vertexBufferSpec;
            vertexBufferSpec.initAsRegularBuffer(vertexData, sizeof(Attribute), draw_data->TotalVtxCount);
            err = sce::Agc::Core::initialize(vertexBuffer, &vertexBufferSpec);
            Assert(err == SCE_OK, "failed to initialize a vertex buffer.");
        }

        float L = draw_data->DisplayPos.x;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T = draw_data->DisplayPos.y;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        const float orthoProjMatrix[4][4] = {
            { 2.0f / (R - L),    0.0f,               0.0f, 0.0f },
            { 0.0f,              2.0f / (T - B),     0.0f, 0.0f },
            { 0.0f,              0.0f,              -1.0f, 0.0f },
            { (R + L) / (L - R), (T + B) / (B - T),  0.0f, 1.0f },
        };

        SRT srt;
        srt.cbPerFrame = frameMem.allocate<CBPerFrame>();
        srt.cbPerFrame->matProj = float4x4(&orthoProjMatrix[0][0]);

        sce::Agc::Core::Binder binder;
        binder.init(&dcb, &dcb).setShaders(nullptr, shader_vv, shader_p);
        binder.setStageActive(sce::Agc::ShaderType::kGs, true);
        binder.setStageActive(sce::Agc::ShaderType::kPs, true);

        binder.getStage(sce::Agc::ShaderType::kGs)
            .setVertexAttributeTable(s_vertexAttributeTable)
            .setVertexBuffers(0, 1, vertexBuffer);
        dcb.setIndexSize(s_indexSize, sce::Agc::GeCachePolicy::kBypass);
        dcb.setIndexBuffer(indexData);

        //ImVec2 dispPos = draw_data->DisplayPos;

        // draw
        uint32_t vertexBufferOffset = 0;
        uint32_t indexBufferOffset = 0;
        for (int i = 0; i < draw_data->CmdListsCount; ++i) {
            const ImDrawList* cmdList = draw_data->CmdLists[i];

            sce::Agc::UcIndexOffset indexOffset;
            indexOffset.init().setOffsetValue(vertexBufferOffset);
            setUcRegistersDirect(dcb, indexOffset);

            for (int j = 0; j < cmdList->CmdBuffer.size(); ++j) {
                const ImDrawCmd &cmd = cmdList->CmdBuffer[j];

                // clip rectangle
                float x1 = cmd.ClipRect.x;
                float y1 = cmd.ClipRect.y;
                float x2 = cmd.ClipRect.z;
                float y2 = cmd.ClipRect.w;

                if (x1 < fb_width && y1 < fb_height && x2 > 0 && y2 > 0) {
                    srt.cbPerObject = frameMem.allocate<CBPerObject>();

                    const Image &image = *(Image*)cmd.TextureId;
                    srt.cbPerObject->texture = image.texture;
                    srt.cbPerObject->sampler = image.sampler;

                    sce::Agc::CxViewport viewportScissor;
                    viewportScissor.init()
                        .setSlot(0)
                        .setScaleX(fb_width * 0.5f).setScaleY(-fb_height * 0.5f).setScaleZ(0.5f)
                        .setOffsetX(fb_width * 0.5f).setOffsetY(fb_height * 0.5f).setOffsetZ(0.5f)
                        .setMinZ(0.0f).setMaxZ(1.0f)
                        .setLeft((uint32_t)x1).setTop((uint32_t)y1).setRight((uint32_t)x2).setBottom((uint32_t)y2)
                        .setWindowOffset(sce::Agc::CxViewport::WindowOffset::kEnable);
                    setCxRegistersDirect(dcb, viewportScissor);

                    binder.getStage(sce::Agc::ShaderType::kGs).setUserSrtBuffer(&srt, sizeof(srt) / 4);
                    binder.getStage(sce::Agc::ShaderType::kPs).setUserSrtBuffer(&srt, sizeof(srt) / 4);

                    dcb.drawIndexOffset(indexBufferOffset, cmd.ElemCount, shader_vv->m_specials->m_drawModifier);
                    binder.postDraw();
                }

                indexBufferOffset += cmd.ElemCount;
            }
            vertexBufferOffset += cmdList->VtxBuffer.size();
        }

        sce::Agc::UcIndexOffset indexOffset;
        indexOffset.init();
        setUcRegistersDirect(dcb, indexOffset);

        s_renderStates.setCleanRegisters(dcb);
    }

    void translate(const ScePadData* padData, PadUsage padUsage, const SceMouseData* mouseData, const ImVec2 &lastMousePos, ControlData* controlData) {
        const float StickDeadZone = 0.0666667f;

        controlData->enableNavigation = false;
        if (padData && padUsage == PadUsage_Navigation) {
            controlData->navInputs[ImGuiNavInput_Activate] = (float)(padData->buttons & SCE_PAD_BUTTON_CROSS);
            controlData->navInputs[ImGuiNavInput_Cancel] = (float)(padData->buttons & SCE_PAD_BUTTON_CIRCLE);
            controlData->navInputs[ImGuiNavInput_Menu] = (float)(padData->buttons & SCE_PAD_BUTTON_SQUARE);
            controlData->navInputs[ImGuiNavInput_Input] = (float)(padData->buttons & SCE_PAD_BUTTON_TRIANGLE);

            controlData->navInputs[ImGuiNavInput_DpadLeft] = (float)(padData->buttons & SCE_PAD_BUTTON_LEFT) * 0.01f; // 0.01f is a scaling factor to slow down resizing speed.
            controlData->navInputs[ImGuiNavInput_DpadRight] = (float)(padData->buttons & SCE_PAD_BUTTON_RIGHT) * 0.01f;
            controlData->navInputs[ImGuiNavInput_DpadUp] = (float)(padData->buttons & SCE_PAD_BUTTON_UP) * 0.01f;
            controlData->navInputs[ImGuiNavInput_DpadDown] = (float)(padData->buttons & SCE_PAD_BUTTON_DOWN) * 0.01f;

            controlData->navInputs[ImGuiNavInput_FocusPrev] = (float)(padData->buttons & SCE_PAD_BUTTON_L1);
            controlData->navInputs[ImGuiNavInput_FocusNext] = (float)(padData->buttons & SCE_PAD_BUTTON_R1);

            controlData->navInputs[ImGuiNavInput_TweakSlow] = (float)(padData->buttons & SCE_PAD_BUTTON_L1);
            controlData->navInputs[ImGuiNavInput_TweakFast] = (float)(padData->buttons & SCE_PAD_BUTTON_R1);

            float leftStickX = (2 * padData->leftStick.x - UINT8_MAX) / 255.0f;
            float leftStickY = (2 * padData->leftStick.y - UINT8_MAX) / 255.0f;
            leftStickX = std::fabs(leftStickX) < StickDeadZone ? 0.0f : leftStickX;
            leftStickY = std::fabs(leftStickY) < StickDeadZone ? 0.0f : leftStickY;
            controlData->navInputs[ImGuiNavInput_LStickLeft] = std::fmax(0.0f, -leftStickX);
            controlData->navInputs[ImGuiNavInput_LStickRight] = std::fmax(0.0f, leftStickX);
            controlData->navInputs[ImGuiNavInput_LStickUp] = std::fmax(0.0f, -leftStickY);
            controlData->navInputs[ImGuiNavInput_LStickDown] = std::fmax(0.0f, leftStickY);

            controlData->enableNavigation = true;
        }
        controlData->hasGamePad = (padData != nullptr) && padData->connected;



        // Treat the left/right stick as mouse cursor/wheel when mouse data is not given.

        std::fill_n(controlData->mouseDown, IM_ARRAYSIZE(controlData->mouseDown), false);
        controlData->mouseWheel = 0;

        controlData->drawCursor = mouseData != nullptr || padData != nullptr;
        controlData->mousePosition = lastMousePos;

        if (mouseData) {
            float length = std::sqrt((float)mouseData->xAxis * mouseData->xAxis + (float)mouseData->yAxis * mouseData->yAxis);
            float delta = 1000.0f * std::pow(length / 100, 1.75f);
            float deltaX = 0.0f;
            float deltaY = 0.0f;
            if (mouseData->xAxis != 0)
                deltaX = (mouseData->xAxis / length) * delta;
            if (mouseData->yAxis != 0)
                deltaY = (mouseData->yAxis / length) * delta;

            controlData->mousePosition = ImVec2(controlData->mousePosition.x + deltaX, controlData->mousePosition.y + deltaY);
            controlData->mouseDown[0] = mouseData->buttons & SCE_MOUSE_BUTTON_PRIMARY;
            controlData->mouseDown[1] = mouseData->buttons & SCE_MOUSE_BUTTON_SECONDARY;
            controlData->mouseDown[2] = mouseData->buttons & SCE_MOUSE_BUTTON_OPTIONAL;
            controlData->mouseDown[3] = mouseData->buttons & SCE_MOUSE_BUTTON_OPTIONAL2;
            controlData->mouseDown[4] = mouseData->buttons & SCE_MOUSE_BUTTON_OPTIONAL3;
            controlData->mouseWheel = mouseData->wheel;
        }
        else if (controlData->hasGamePad) {
            if (padUsage == PadUsage_MouseEmulation) {
                //float rightStickX = (2 * padData->rightStick.x - UINT8_MAX) / 255.0f;
                float rightStickY = (2 * padData->rightStick.y - UINT8_MAX) / 255.0f;
                //rightStickX = std::fabs(rightStickX) < StickDeadZone ? 0.0f : rightStickX;
                rightStickY = std::fabs(rightStickY) < StickDeadZone ? 0.0f : rightStickY;

                float delta = 0.5f * std::copysign(std::pow(std::fabs(rightStickY), 2), -rightStickY);

                controlData->mouseDown[0] = padData->buttons & SCE_PAD_BUTTON_CROSS;
                controlData->mouseDown[1] = padData->buttons & SCE_PAD_BUTTON_CIRCLE;
                controlData->mouseDown[2] = padData->buttons & SCE_PAD_BUTTON_TRIANGLE;
                controlData->mouseDown[3] = false;
                controlData->mouseDown[4] = false;
                controlData->mouseWheel = delta;
            }

            float leftStickX = (2 * padData->leftStick.x - UINT8_MAX) / 255.0f;
            float leftStickY = (2 * padData->leftStick.y - UINT8_MAX) / 255.0f;
            leftStickX = std::fabs(leftStickX) < StickDeadZone ? 0.0f : leftStickX;
            leftStickY = std::fabs(leftStickY) < StickDeadZone ? 0.0f : leftStickY;

            float length = std::sqrt(leftStickX * leftStickX + leftStickY * leftStickY);
            float delta = 15.0f * std::pow(length, 2);
            float deltaX = 0.0f;
            float deltaY = 0.0f;
            if (leftStickX != 0)
                deltaX = (leftStickX / length) * delta;
            if (leftStickY != 0)
                deltaY = (leftStickY / length) * delta;

            // override the delta if a DPad (L/R) is pressed.
            if (padUsage == PadUsage_MouseEmulation) {
                if ((padData->buttons & SCE_PAD_BUTTON_RIGHT) || (padData->buttons & SCE_PAD_BUTTON_LEFT) ||
                    (padData->buttons & SCE_PAD_BUTTON_DOWN) || (padData->buttons & SCE_PAD_BUTTON_UP)) {
                    deltaX = (bool)(padData->buttons & SCE_PAD_BUTTON_RIGHT) - (bool)(padData->buttons & SCE_PAD_BUTTON_LEFT);
                    deltaY = (bool)(padData->buttons & SCE_PAD_BUTTON_DOWN) - (bool)(padData->buttons & SCE_PAD_BUTTON_UP);
                }
            }

            controlData->mousePosition = ImVec2(controlData->mousePosition.x + deltaX, controlData->mousePosition.y + deltaY);
        }
    }
}
