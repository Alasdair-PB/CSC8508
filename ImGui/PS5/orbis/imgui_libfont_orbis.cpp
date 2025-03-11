/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 10.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc. 
 * 
 */

#include <scebase_common/scebase_target.h>
#if _SCE_TARGET_OS_ORBIS

#include "sampleutil/ui_framework/imgui/imgui_libfont.h"
#include "../imgui_internal.h"   // ImMin,ImMax,ImFontAtlasBuild*,
#include <stdint.h>

#include <mspace.h>

#include <unordered_map>

#include <libsysmodule.h>
#include <sce_font.h>
#include <sanitizer/asan_interface.h>
#include <mat.h>
#ifdef _DEBUG
#include "sampleutil/memory/memory_analyzer.h"
#include "sampleutil/debug/perf.h"
#endif

#define STBRP_ASSERT(x)    IM_ASSERT(x)
#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
struct stbrp_context;
__attribute__((unused)) static void stbrp_setup_heuristic(stbrp_context *context, int heuristic);
#include "../imstb_rectpack.h"

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#endif

#pragma comment(lib,"libSceSysmodule_stub_weak.a")
#pragma comment(lib,"SceFont_stub_weak")
#pragma comment(lib,"SceFontFt_stub_weak")

namespace 
{
    // Glyph metrics:
    // --------------
    //
    //                       xmin                     xmax
    //                        |                         |
    //                        |<-------- width -------->|
    //                        |                         |
    //              |         +-------------------------+----------------- ymax
    //              |         |    ggggggggg   ggggg    |     ^        ^
    //              |         |   g:::::::::ggg::::g    |     |        |
    //              |         |  g:::::::::::::::::g    |     |        |
    //              |         | g::::::ggggg::::::gg    |     |        |
    //              |         | g:::::g     g:::::g     |     |        |
    //    offsetX  -|-------->| g:::::g     g:::::g     |  offsetY     |
    //              |         | g:::::g     g:::::g     |     |        |
    //              |         | g::::::g    g:::::g     |     |        |
    //              |         | g:::::::ggggg:::::g     |     |        |
    //              |         |  g::::::::::::::::g     |     |      height
    //              |         |   gg::::::::::::::g     |     |        |
    //  baseline ---*---------|---- gggggggg::::::g-----*--------      |
    //            / |         |             g:::::g     |              |
    //     origin   |         | gggggg      g:::::g     |              |
    //              |         | g:::::gg   gg:::::g     |              |
    //              |         |  g::::::ggg:::::::g     |              |
    //              |         |   gg:::::::::::::g      |              |
    //              |         |     ggg::::::ggg        |              |
    //              |         |         gggggg          |              v
    //              |         +-------------------------+----------------- ymin
    //              |                                   |
    //              |------------- advanceX ----------->|

    /// A structure that describe a glyph.
    struct GlyphInfo 
    {
        float Width;		// Glyph's width in pixels.
        float Height;		// Glyph's height in pixels.
        float OffsetX;		// The distance from the origin ("pen position") to the left of the glyph.
        float OffsetY;		// The distance from the origin to the top of the glyph. This is usually a value < 0.
        float AdvanceX;		// The distance from the origin to the origin of the next glyph. This is usually a value > 0.
    };

    // Font parameters and metrics.
    struct FontInfo 
    {
        uint32_t    PixelHeight;        // Size this font was generated with.
        float       Ascender;           // The pixel extents above the baseline in pixels (typically positive).
        float       Descender;          // The extents below the baseline in pixels (typically negative).
        float       LineSpacing;        // The baseline-to-baseline distance. Note that it usually is larger than the sum of the ascender and descender taken as absolute values. There is also no guarantee that no glyphs extend above or below subsequent baselines when using this distance. Think of it as a value the designer of the font finds appropriate.
        float       LineGap;            // The spacing in pixels between one row's descent and the next row's ascent.
        float       MaxAdvanceWidth;    // This field gives the maximum horizontal cursor advance for all glyphs in the font.
    };

    // libfont glyph rasterizer.
    // NB: No ctor/dtor, explicitly call Init()/Shutdown()
    struct FreeTypeFont
    {
        void        Init(const ImFontConfig& cfg, unsigned int extra_user_flags, SceFontLibrary	&library, SceFontRenderer	&renderer);   // Initialize from an external data buffer. Doesn't copy data, and you must ensure it stays valid up to this object lifetime.
        void        Shutdown();
		FontInfo	EstimateFontInfo(float font_scale);
        void        SetPixelHeight(float pixel_height);                               // Change font pixel size. All following calls to RasterizeGlyph() will use this size
        bool        CalcGlyphInfo(uint32_t ucode, GlyphInfo& glyph_info, SceFontRenderSurface &bitmap);
        void        BlitGlyph(SceFontRenderSurface &bitmap, stbrp_rect rect, uint8_t* dst, uint32_t dst_pitch, unsigned char* multiply_table = nullptr);

        // [Internals]
        FontInfo        Info;               // Font descriptor of the current font.
        unsigned int    UserFlags;          // = ImFontConfig::RasterizerFlags
		SceFontHandle	LibFontFontHandle;
    };

	void FreeTypeFont::Init(const ImFontConfig& cfg, unsigned int extra_user_flags, SceFontLibrary	&library, SceFontRenderer	&renderer)
	{
		int ret = SCE_OK;

        // Convert to freetype flags (nb: Bold and Oblique are processed separately)
        UserFlags = cfg.RasterizerFlags | extra_user_flags;
		uint32_t fontset;
		if (UserFlags & ImGuiLibFont::Oblique)
		{
			if (UserFlags & ImGuiLibFont::Bold)
			{
				fontset = SCE_FONT_SET_SST_STD_EUROPEAN_JP_W1G_BOLD_ITALIC;
			}
			else {
				fontset = SCE_FONT_SET_SST_STD_EUROPEAN_JP_W1G_ITALIC;
			}
		}
		else {
			if (UserFlags & ImGuiLibFont::Bold)
			{
				fontset = SCE_FONT_SET_SST_STD_EUROPEAN_JP_W1G_BOLD;
			}
			else {
				fontset = SCE_FONT_SET_SST_STD_EUROPEAN_JP_W1G;
			}
		}
		LibFontFontHandle = 0;
		if (cfg.FontData == (void*)0x100)
		{
			ret = sceFontOpenFontSet(library, fontset, SCE_FONT_OPEN_MEMORY_STREAM, nullptr, &LibFontFontHandle);
		} else {
			ret = sceFontOpenFontMemory(library, cfg.FontData, cfg.FontDataSize, nullptr, &LibFontFontHandle);
		}
		IM_ASSERT(ret == SCE_FONT_OK);

		memset(&Info, 0, sizeof(Info));
		SetPixelHeight(cfg.SizePixels);

		ret = sceFontBindRenderer(LibFontFontHandle, renderer);
		IM_ASSERT(ret == SCE_FONT_OK);
	}

    void FreeTypeFont::Shutdown()
    {
		int ret = SCE_OK;

		ret = sceFontUnbindRenderer(LibFontFontHandle);
		IM_ASSERT(ret == SCE_FONT_OK);

		ret = sceFontCloseFont(LibFontFontHandle);
		IM_ASSERT(ret == SCE_FONT_OK);
    }

    FontInfo FreeTypeFont::EstimateFontInfo(float font_scale)
	{
		int ret = SCE_OK;

		FontInfo outInfo;

		outInfo.Ascender = 0.f;
		outInfo.Descender = 0.f;
		outInfo.MaxAdvanceWidth = 0.f;
		outInfo.LineGap = 0.f;

		ret = sceFontSetScalePixel(LibFontFontHandle, font_scale, font_scale);
		IM_ASSERT(ret == SCE_OK);

		// Estimate texture size
		char alphanumericCharacters[] = "abcdefghijklnmopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
		{
			for(char *code = &alphanumericCharacters[0]; *code != 0; code++)
			{
				uint16_t c = (uint16_t)(*code);

				SceFontGlyphMetrics gm;
				ret = sceFontGetCharGlyphMetrics(LibFontFontHandle, c, &gm);
				IM_ASSERT(ret == SCE_OK);

				outInfo.Ascender = fmaxf(outInfo.Ascender, gm.Horizontal.bearingY);
				outInfo.Descender = fminf(outInfo.Descender, gm.Horizontal.bearingY - gm.height);
				outInfo.MaxAdvanceWidth = fmaxf(outInfo.MaxAdvanceWidth, gm.Horizontal.advance);
			}
		}

		uint16_t ucs2aiueo[] = { 0x3042, // a
			0x3044, //i 
			0x3046, // u
			0x3048, //e
			0x304a, //o
			0x30D7, //pu
			0x30DD, //po

			0x4E00,
			0x4E01,
			0x4E02,
			0x4E03,
			0x4E04,
			0x4E05,
			0x4E06,
			0x4E07,

			0xAC00,
			0xAC01,
			0xAC02,
			0xAC03,
			0xAC04,
			0xAC05,
			0xAC06,
			0xAC07,

			0x0E38,
			0x0E4C,
			0x1ED3,

			0 };
		{
			for(uint16_t *code = &ucs2aiueo[0]; *code != 0; code++)
			{
				uint16_t c = *code;

				SceFontGlyphMetrics gm;
				ret = sceFontGetCharGlyphMetrics(LibFontFontHandle, c, &gm);
				if (ret != SCE_OK) {
					continue;
				}

				outInfo.Ascender = fmaxf(outInfo.Ascender, gm.Horizontal.bearingY);
				outInfo.Descender = fminf(outInfo.Descender, gm.Horizontal.bearingY - gm.height);
				outInfo.MaxAdvanceWidth = fmaxf(outInfo.MaxAdvanceWidth, gm.Horizontal.advance);
			}
		}

		outInfo.PixelHeight = outInfo.Ascender - outInfo.Descender;
		outInfo.LineSpacing = outInfo.PixelHeight + outInfo.LineGap;

		return outInfo;
	}

	void FreeTypeFont::SetPixelHeight(float pixel_height) 
	{
        // I'm not sure how to deal with font sizes properly.
        // As far as I understand, currently ImGui assumes that the 'pixel_height' is a maximum height of an any given glyph,
        // i.e. it's the sum of font's ascender and descender. Seems strange to me.

		Info = EstimateFontInfo(pixel_height);

		float font_scale = pixel_height * (pixel_height / Info.PixelHeight);

		Info = EstimateFontInfo(font_scale);
    }

    bool FreeTypeFont::CalcGlyphInfo(uint32_t ucode, GlyphInfo &glyph_info, SceFontRenderSurface &bitmap)
    {
		int ret = SCE_OK;

		SceFontGlyphMetrics gm;
		ret = sceFontGetCharGlyphMetrics(LibFontFontHandle, ucode, &gm);
		if (ret == SCE_FONT_ERROR_NO_SUPPORT_GLYPH)
		{
			return false;
		}
		IM_ASSERT(ret == SCE_FONT_OK);

		SceFontGlyphMetrics  metrics;
		SceFontRenderResult  result;
		ret = sceFontRenderCharGlyphImageHorizontal(LibFontFontHandle, ucode, &bitmap, -gm.Horizontal.bearingX, gm.Horizontal.bearingY, &metrics, &result);
		IM_ASSERT(ret == SCE_FONT_OK);

        glyph_info.AdvanceX = gm.Horizontal.advance;
		glyph_info.OffsetX = gm.Horizontal.bearingX;
        glyph_info.OffsetY = -gm.Horizontal.bearingY;
		glyph_info.Width = gm.width;
		glyph_info.Height = gm.height;

		return true;
    }

    void FreeTypeFont::BlitGlyph(SceFontRenderSurface &bitmap, stbrp_rect rect, uint8_t* dst, uint32_t dst_pitch, unsigned char* multiply_table)
    {
		const uint32_t w = rect.w;
		const uint32_t h = rect.h;
		const uint8_t* src = reinterpret_cast<const uint8_t*>(bitmap.buffer);
		const uint32_t src_pitch = bitmap.widthByte;

        if (multiply_table == nullptr)
        {
            for (uint32_t y = 0; y < h; y++, src += src_pitch, dst += dst_pitch)
                memcpy(dst, src, w);
        }
        else
        {
            for (uint32_t y = 0; y < h; y++, src += src_pitch, dst += dst_pitch)
                for (uint32_t x = 0; x < w; x++)
                    dst[x] = multiply_table[src[x]];
        }
    }
}

static bool s_needAtlasUpdate = false;

// Flag for adding new glyph and rebuilding font atlas
static bool s_addNewGlyph = false;

// Cached codes which is added to atlas
static ImVector<ImWchar> *s_additionalCodes = nullptr;

// Packed node data in atlas
static ImVector<stbrp_node> *s_packNodes = nullptr;

// Data used to pack new glyph into atlas
static stbrp_context s_context;

// Reserve to add new font glyph.
void reserveNewCode(ImWchar c)
{
    if (!s_additionalCodes->contains(c))
    {
        s_additionalCodes->push_back(c);
        s_addNewGlyph = true;
    }
}


ImFont *ImGuiLibFont::AddSystemFont(ImFontAtlas *atlas, float size_pixels, const ImFontConfig *font_cfg_template, const ImWchar *glyph_ranges)
{
    IM_ASSERT(!atlas->Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == nullptr);
    font_cfg.FontData = (void*)0x100;
    font_cfg.FontDataSize = 1;
    font_cfg.SizePixels = size_pixels;
    if (glyph_ranges)
        font_cfg.GlyphRanges = glyph_ranges;
    s_needAtlasUpdate = true;
    return atlas->AddFont(&font_cfg);
}

bool ImGuiLibFont::ClearSystemFont(ImFontAtlas *atlas)
{
	for (int i = 0; i < atlas->ConfigData.Size; i++)
	{
		if ((uintptr_t)atlas->ConfigData[i].FontData == 0x100)
		{
			atlas->ConfigData[i].FontData = nullptr;
		}
	}

	return true;
}

int ImGuiLibFont::Initialize()
{
    int ret;

    ret = sceSysmoduleLoadModule(SCE_SYSMODULE_FONT);
    IM_ASSERT(ret == SCE_OK);
    if (ret != SCE_OK) {
        return ret;
    }
    ret = sceSysmoduleLoadModule(SCE_SYSMODULE_FONT_FT);
    IM_ASSERT(ret == SCE_OK);
    if (ret != SCE_OK) {
        sceSysmoduleUnloadModule(SCE_SYSMODULE_FONT);
        return ret;
    }
    ret = sceSysmoduleLoadModule(SCE_SYSMODULE_FREETYPE_OT);
    IM_ASSERT(ret == SCE_OK);
    if (ret != SCE_OK) {
        sceSysmoduleUnloadModule(SCE_SYSMODULE_FONT);
        sceSysmoduleUnloadModule(SCE_SYSMODULE_FONT_FT);
        return ret;
    }

    s_additionalCodes = new ImVector<ImWchar>;
    s_packNodes = new ImVector<stbrp_node>;

    return SCE_OK;
}

int ImGuiLibFont::Finalize()
{
    int ret;

    delete s_additionalCodes; s_additionalCodes = nullptr;
    delete s_packNodes; s_packNodes = nullptr;

    ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_FONT);
    IM_ASSERT(ret == SCE_OK);
    if (ret != SCE_OK) {
        return ret;
    }
    ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_FONT_FT);
    IM_ASSERT(ret == SCE_OK);
    if (ret != SCE_OK) {
        return ret;
    }
    ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_FREETYPE_OT);
    IM_ASSERT(ret == SCE_OK);

    return ret;
}

bool ImGuiLibFont::BuildFontAtlas(ImFontAtlas *atlas, SceLibcMspace mspace, unsigned int extra_flags)
{
	int ret = SCE_OK;

	if (!(s_needAtlasUpdate || s_addNewGlyph)) return false;

    // Prioritize first build when both flag is on
    bool postpone = false;
    if (s_addNewGlyph && s_needAtlasUpdate)
    {
        postpone = true;
        s_addNewGlyph = false;
    }

    IM_ASSERT(atlas->ConfigData.Size > 0);
    IM_ASSERT(atlas->TexGlyphPadding == 1); // Not supported

	SceFontMemoryInterface libFontAllocator =
	{
		/*SceFontMallocCallback=*/[](void *mspaceobject, uint32_t size)
    	{
            void *pAllocatedMem = nullptr;
            if (mspaceobject != nullptr)
            {
                pAllocatedMem = sceLibcMspaceMalloc(*(SceLibcMspace*)mspaceobject, size);
#ifdef _DEBUG
                if (sce::SampleUtil::Memory::g_isMatInitialized)
                {
                    sceMatAlloc(pAllocatedMem, size, 0, SCEMAT_GROUP_AUTO);
                    sceMatTagAllocation(pAllocatedMem, "libfont for imgui");
                }
                sce::SampleUtil::Debug::Perf::tagBuffer("libfont for imgui", pAllocatedMem, size, 1);
                ASAN_UNPOISON_MEMORY_REGION(pAllocatedMem, size);
#endif
            } else {
                pAllocatedMem = malloc(size);
#ifdef _DEBUG
                if (sce::SampleUtil::Memory::g_isMatInitialized)
                {
                    sceMatAlloc(pAllocatedMem, size, 0, SCEMAT_GROUP_AUTO);
                    sceMatTagAllocation(pAllocatedMem, "libfont for imgui");
                }
                sce::SampleUtil::Debug::Perf::tagBuffer("libfont for imgui", pAllocatedMem, size, 1);
#endif
            }
            return pAllocatedMem;
    	},
		/*SceFontFreeCallback=*/[](void *mspaceobject, void *address)
        {
            if (mspaceobject != nullptr)
            {
#ifdef _DEBUG
                size_t allocSize = sceLibcMspaceMallocUsableSize(address);
                ASAN_POISON_MEMORY_REGION(address, allocSize);
                if (sce::SampleUtil::Memory::g_isMatInitialized)
                {
                    sceMatFree(address);
                }
                sce::SampleUtil::Debug::Perf::unTagBuffer(address);
#endif
                sceLibcMspaceFree(*(SceLibcMspace*)mspaceobject, address);
            } else {
                free(address);
#ifdef _DEBUG
                if (sce::SampleUtil::Memory::g_isMatInitialized)
                {
                    sceMatFree(address);
                }
                sce::SampleUtil::Debug::Perf::unTagBuffer(address);
#endif
            }
        },
		/*SceFontReallocCallback=*/[](void *mspaceobject, void *address, uint32_t size)
        {
            void *pAllocatedMem = nullptr;
            if (mspaceobject != nullptr)
            {
#ifdef _DEBUG
                size_t allocSize = sceLibcMspaceMallocUsableSize(address);
                ASAN_POISON_MEMORY_REGION(address, allocSize);
                if (sce::SampleUtil::Memory::g_isMatInitialized)
                {
                    sceMatReallocBegin(address, allocSize, SCEMAT_GROUP_AUTO);
                }
                sce::SampleUtil::Debug::Perf::unTagBuffer(address);
#endif
                pAllocatedMem = sceLibcMspaceRealloc(*(SceLibcMspace*)mspaceobject, address, size);
#ifdef _DEBUG
                ASAN_UNPOISON_MEMORY_REGION(pAllocatedMem, size);
                if (sce::SampleUtil::Memory::g_isMatInitialized)
                {
                    sceMatReallocEnd(pAllocatedMem, size, 0);
                }
                sce::SampleUtil::Debug::Perf::tagBuffer("libfont for imgui", pAllocatedMem, size, 1);
#endif
            } else {
#ifdef _DEBUG
                size_t allocSize = sceLibcMspaceMallocUsableSize(address);
                if (sce::SampleUtil::Memory::g_isMatInitialized)
                {
                    sceMatReallocBegin(address, allocSize, SCEMAT_GROUP_AUTO);
                }
                sce::SampleUtil::Debug::Perf::unTagBuffer(address);
#endif
                pAllocatedMem = realloc(address, size);
#ifdef _DEBUG
                if (sce::SampleUtil::Memory::g_isMatInitialized)
                {
                    sceMatReallocEnd(pAllocatedMem, size, 0);
                }
                sce::SampleUtil::Debug::Perf::tagBuffer("libfont for imgui", pAllocatedMem, size, 1);
#endif
            }
            return pAllocatedMem;
       },
		/*SceFontCallocCallback=*/[](void *mspaceobject, uint32_t count, uint32_t size)
        {
            void *pAllocatedMem = nullptr;
            if (mspaceobject != nullptr)
            {
                pAllocatedMem = sceLibcMspaceCalloc(*(SceLibcMspace*)mspaceobject, count, size);
#ifdef _DEBUG
                ASAN_UNPOISON_MEMORY_REGION(pAllocatedMem, count * size);
                if (sce::SampleUtil::Memory::g_isMatInitialized)
                {
                    sceMatAlloc(pAllocatedMem, count * size, 0, SCEMAT_GROUP_AUTO);
                }
                sce::SampleUtil::Debug::Perf::tagBuffer("libfont for imgui", pAllocatedMem, size, count);
#endif
            } else {
                pAllocatedMem = calloc(count, size);
#ifdef _DEBUG
                if (sce::SampleUtil::Memory::g_isMatInitialized)
                {
                    sceMatAlloc(pAllocatedMem, count * size, 0, SCEMAT_GROUP_AUTO);
                }
                sce::SampleUtil::Debug::Perf::tagBuffer("libfont for imgui", pAllocatedMem, size, count);
#endif
            }
            return pAllocatedMem;
        },
		nullptr, nullptr
	};

	SceFontMemory memory;
	SceFontLibrary library;
	SceFontRenderer renderer;

	ret = sceFontMemoryInit(&memory, nullptr, 0, &libFontAllocator, &mspace, nullptr, nullptr);
	IM_ASSERT(ret == SCE_FONT_OK);

	ret = sceFontCreateLibrary(&memory, sceFontSelectLibraryFt(0), &library);
	IM_ASSERT(ret == SCE_FONT_OK);

	ret = sceFontSupportSystemFonts(library);
	IM_ASSERT(ret == SCE_FONT_OK);
	ret = sceFontSupportExternalFonts(library, 4, SCE_FONT_FORMAT_OPENTYPE|SCE_FONT_FORMAT_OPENTYPE_TT);
	IM_ASSERT(ret == SCE_FONT_OK);

	ret = sceFontCreateRenderer(&memory, sceFontSelectRendererFt(0), &renderer);
	IM_ASSERT(ret == SCE_FONT_OK);

    // Check exist texture when add glyph
    if (s_addNewGlyph)
    {
        IM_ASSERT(atlas->TexID != nullptr);
    }
    // Need only for first build
    else
    {
        ImFontAtlasBuildRegisterDefaultCustomRects(atlas);

        atlas->TexID = nullptr;
        atlas->TexWidth = atlas->TexHeight = 0;
        atlas->TexUvScale = ImVec2(0.0f, 0.0f);
        atlas->TexUvWhitePixel = ImVec2(0.0f, 0.0f);
        atlas->ClearTexData();
    }

    ImVector<FreeTypeFont> fonts;
    fonts.resize(atlas->ConfigData.Size);

    ImVec2 max_glyph_size(1.0f, 1.0f);
    ImVec2 mean_glyph_size(0.0f, 0.0f);

    // Count glyphs/ranges, initialize font
    int total_glyphs_count = 0;
    int total_ranges_count = 0;
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++) 
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        FreeTypeFont& font_face = fonts[input_i];
        IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

		font_face.Init(cfg, extra_flags, library, renderer);

        max_glyph_size.x = ImMax(max_glyph_size.x, font_face.Info.MaxAdvanceWidth);
        max_glyph_size.y = ImMax(max_glyph_size.y, font_face.Info.Ascender - font_face.Info.Descender);

        if (!cfg.GlyphRanges)
            cfg.GlyphRanges = atlas->GetGlyphRangesDefault();
		int glyphs_count = 0;
        for (const ImWchar* in_range = cfg.GlyphRanges; in_range[0] && in_range[ 1 ]; in_range += 2, total_ranges_count++) 
            glyphs_count += (in_range[1] - in_range[0]) + 1;
		total_glyphs_count += glyphs_count;
		mean_glyph_size.x += (font_face.Info.MaxAdvanceWidth) * (float)glyphs_count;
		mean_glyph_size.y += (font_face.Info.Ascender - font_face.Info.Descender) * (float)glyphs_count;
    }
	mean_glyph_size.x /= (float)total_glyphs_count;
	mean_glyph_size.y /= (float)total_glyphs_count;

    // Already setted when add glyph
    if (s_addNewGlyph)
    {
        IM_ASSERT(atlas->TexWidth != 0);
    }
    // We need a width for the skyline algorithm. Using a dumb heuristic here to decide of width. User can override TexDesiredWidth and TexGlyphPadding if they wish.
    // Width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
    // Need only for first build
    else
    {
        atlas->TexWidth = (atlas->TexDesiredWidth > 0) ? atlas->TexDesiredWidth : (total_glyphs_count > 3000) ? 4096 : (total_glyphs_count > 2000) ? 2048 : (total_glyphs_count > 1000) ? 1024 : 512;
    }

    // We don't do the original first pass to determine texture height, but just rough estimate.
    // Looks ugly inaccurate and excessive, but AFAIK with FreeType we actually need to render glyphs to get exact sizes.
    // Alternatively, we could just render all glyphs into a big shadow buffer, get their sizes, do the rectangle packing and just copy back from the 
    // shadow buffer to the texture buffer. Will give us an accurate texture height, but eat a lot of temp memory. Probably no one will notice.)
    const int total_rects = total_glyphs_count + atlas->CustomRects.size();
//    float min_rects_per_row = ceilf((atlas->TexWidth / (max_glyph_size.x + 1.0f)));
    float min_rects_per_row = ceilf((atlas->TexWidth / (mean_glyph_size.x + 1.0f)));
    float min_rects_per_column = ceilf(total_rects / min_rects_per_row);
//    atlas->TexHeight = (int)(min_rects_per_column * (max_glyph_size.y + 1.0f));

    // Node data is exist and keep from first build
    if (s_addNewGlyph)
    {
        IM_ASSERT(s_packNodes->Size > 0);
    }
    // Need only for first build
    else
    {
        s_packNodes->resize(total_rects);

        atlas->TexHeight = (int)(min_rects_per_column * (mean_glyph_size.y + 1.0f));

        // Create texture
        atlas->TexHeight = (atlas->Flags & ImFontAtlasFlags_NoPowerOfTwoHeight) ? (atlas->TexHeight + 1) : ImUpperPowerOfTwo(atlas->TexHeight);
        atlas->TexUvScale = ImVec2(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
        atlas->TexPixelsAlpha8 = (unsigned char*)ImGui::MemAlloc(atlas->TexWidth * atlas->TexHeight);
        IM_ASSERT(atlas->TexPixelsAlpha8);
        memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);

        stbrp_init_target(&s_context, atlas->TexWidth, atlas->TexHeight, s_packNodes->Data, total_rects);

        // Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
        ImFontAtlasBuildPackCustomRects(atlas, &s_context);
    }

    // Render characters, setup ImFont and glyphs for runtime
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        FreeTypeFont& font_face = fonts[input_i];

        // create font surface to raster a character
        float dummyPixelSize = cfg.SizePixels * 1.5f;   // Cause Sanitizer HeapOverflow error, so adjust surface size
        void *rasterBuffer = malloc(dummyPixelSize * dummyPixelSize);
        SceFontRenderSurface surface;
        sceFontRenderSurfaceInit(&surface, rasterBuffer, dummyPixelSize, 1, dummyPixelSize, dummyPixelSize);
        sceFontRenderSurfaceSetScissor(&surface, 0, 0, dummyPixelSize, dummyPixelSize);

        ImFont* dst_font = cfg.DstFont;
        if (cfg.MergeMode)
            dst_font->BuildLookupTable();

        const float ascent = font_face.Info.Ascender;
        const float descent = font_face.Info.Descender;
        // Need only for first build
        if (!s_addNewGlyph)
        {
            ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
        }
        const float font_off_x = cfg.GlyphOffset.x;
        const float font_off_y = cfg.GlyphOffset.y + (float)(int)(dst_font->Ascent + 0.5f);

        bool multiply_enabled = (cfg.RasterizerMultiply != 1.0f);
        unsigned char multiply_table[256];
        if (multiply_enabled)
            ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);

        // First build
        if (!s_addNewGlyph)
        {
            for (const ImWchar* in_range = cfg.GlyphRanges; in_range[0] && in_range[1]; in_range += 2)
            {
                for (uint32_t codepoint = in_range[0]; codepoint <= in_range[1]; ++codepoint)
                {
                    if (cfg.MergeMode && dst_font->FindGlyphNoFallback((ImWchar)codepoint))
                        continue;

                    GlyphInfo glyph_info;
                    memset(rasterBuffer, 0, dummyPixelSize * dummyPixelSize);
                    if (!font_face.CalcGlyphInfo(codepoint, glyph_info, surface))
                    {
                        continue;
                    }

                    // Pack rectangle
                    stbrp_rect rect;
                    rect.w = (uint16_t)glyph_info.Width + 2; // Account for texture filtering
                    rect.h = (uint16_t)glyph_info.Height + 2;
                    stbrp_pack_rects(&s_context, &rect, 1);

                    // Copy rasterized pixels to main texture
                    uint8_t* blit_dst = atlas->TexPixelsAlpha8 + (rect.y + 1) * atlas->TexWidth + rect.x + 1;
                    stbrp_rect blit_rect = rect;
                    blit_rect.w -= 1;
                    blit_rect.h -= 1;
                    ++blit_rect.x;
                    ++blit_rect.y;
                    font_face.BlitGlyph(surface, blit_rect, blit_dst, atlas->TexWidth, multiply_enabled ? multiply_table : nullptr);

                    float char_advance_x_org = glyph_info.AdvanceX;
                    float char_advance_x_mod = ImClamp(char_advance_x_org, cfg.GlyphMinAdvanceX, cfg.GlyphMaxAdvanceX);
                    float char_off_x = font_off_x;
                    if (char_advance_x_org != char_advance_x_mod)
                        char_off_x += cfg.PixelSnapH ? (float)(int)((char_advance_x_mod - char_advance_x_org) * 0.5f) : (char_advance_x_mod - char_advance_x_org) * 0.5f;

                    // Register glyph
                    dst_font->AddGlyph((ImWchar)codepoint,
                        glyph_info.OffsetX + char_off_x,
                        glyph_info.OffsetY + font_off_y,
                        glyph_info.OffsetX + char_off_x + blit_rect.w,
                        glyph_info.OffsetY + font_off_y + blit_rect.h,
                        blit_rect.x / (float)atlas->TexWidth,
                        blit_rect.y / (float)atlas->TexHeight,
                        (blit_rect.x + blit_rect.w) / (float)atlas->TexWidth,
                        (blit_rect.y + blit_rect.h) / (float)atlas->TexHeight,
                        char_advance_x_mod);
                }
            }
        }
        // Add new glyph to existing atlas
        else
        {
            for (ImWchar code : *s_additionalCodes)
            {
                GlyphInfo glyph_info;
                memset(rasterBuffer, 0, dummyPixelSize * dummyPixelSize);
                if (!font_face.CalcGlyphInfo(code, glyph_info, surface))
                {
                    continue;
                }

                // Pack rectangle
                stbrp_rect rect;
                rect.w = (uint16_t)glyph_info.Width + 1; // Account for texture filtering
                rect.h = (uint16_t)glyph_info.Height + 1;
                stbrp_pack_rects(&s_context, &rect, 1);

                // Copy rasterized pixels to main texture
                uint8_t* blit_dst = atlas->TexPixelsAlpha8 + rect.y * atlas->TexWidth + rect.x;
                    stbrp_rect blit_rect = rect;
                    blit_rect.w -= 1;
                    blit_rect.h -= 1;
                    ++blit_rect.x;
                    ++blit_rect.y;
                font_face.BlitGlyph(surface, blit_rect, blit_dst, atlas->TexWidth, multiply_enabled ? multiply_table : nullptr);

                float char_advance_x_org = glyph_info.AdvanceX;
                float char_advance_x_mod = ImClamp(char_advance_x_org, cfg.GlyphMinAdvanceX, cfg.GlyphMaxAdvanceX);
                float char_off_x = font_off_x;
                if (char_advance_x_org != char_advance_x_mod)
                    char_off_x += cfg.PixelSnapH ? (float)(int)((char_advance_x_mod - char_advance_x_org) * 0.5f) : (char_advance_x_mod - char_advance_x_org) * 0.5f;

                // Register glyph
                dst_font->AddGlyph(code,
                    glyph_info.OffsetX + char_off_x, 
                    glyph_info.OffsetY + font_off_y, 
                    glyph_info.OffsetX + char_off_x + blit_rect.w, 
                    glyph_info.OffsetY + font_off_y + blit_rect.h,
                    blit_rect.x / (float)atlas->TexWidth, 
                    blit_rect.y / (float)atlas->TexHeight, 
                    (blit_rect.x + blit_rect.w) / (float)atlas->TexWidth, 
                    (blit_rect.y + blit_rect.h) / (float)atlas->TexHeight,
                    char_advance_x_mod);
            }
        }

		free(rasterBuffer);
	}

    // Cleanup
    for (int n = 0; n < fonts.Size; n++)
        fonts[n].Shutdown();

    ret = sceFontDestroyRenderer(&renderer);
    IM_ASSERT(ret == SCE_FONT_OK);

    ret = sceFontDestroyLibrary(&library);
    IM_ASSERT(ret == SCE_FONT_OK);

    ret = sceFontMemoryTerm(&memory);
    IM_ASSERT(ret == SCE_FONT_OK);

    ImFontAtlasBuildFinish(atlas);

    // Clear cache and flag for additional glyph
    if (s_addNewGlyph)
    {
        s_additionalCodes->clear();
        s_addNewGlyph = false;
    }
    // Add glyph next frame
    if (postpone)
    {
        s_addNewGlyph = true;
    }

    s_needAtlasUpdate = false;
		
	return true;
}

// Build atlas which include only characters in input string
void ImGuiLibFont::BuildFontAtlasLimitedChar(ImFontAtlas* atlas, ImFont *target, const ImWchar *str)
{
    int ret = SCE_OK;

    IM_ASSERT(atlas->ConfigData.Size > 0);
    IM_ASSERT(atlas->TexGlyphPadding == 1); // Not supported

    SceFontMemoryInterface libFontAllocator =
    {
        /*SceFontMallocCallback=*/[](void *mspaceobject, uint32_t size)
        {
            (void)mspaceobject;
            void *pAllocatedMem = malloc(size);
            return pAllocatedMem;
        },
        /*SceFontFreeCallback=*/[](void *mspaceobject, void *address)
        {
            (void)mspaceobject;
            free(address);
        },
        /*SceFontReallocCallback=*/[](void *mspaceobject, void *address, uint32_t size)
        {
            (void)mspaceobject;
            void *pAllocatedMem = realloc(address, size);
            return pAllocatedMem;
        },
        /*SceFontCallocCallback=*/[](void *mspaceobject, uint32_t count, uint32_t size)
        {
            (void)mspaceobject;
            void *pAllocatedMem = calloc(count, size);
            return pAllocatedMem;
        },
        nullptr, nullptr
    };

    SceFontMemory memory;
    SceFontLibrary library;
    SceFontRenderer renderer;

    ret = sceFontMemoryInit(&memory, nullptr, 0, &libFontAllocator, nullptr, nullptr, nullptr);
    IM_ASSERT(ret == SCE_FONT_OK);

    ret = sceFontCreateLibrary(&memory, sceFontSelectLibraryFt(0), &library);
    IM_ASSERT(ret == SCE_FONT_OK);

    ret = sceFontSupportSystemFonts(library);
    IM_ASSERT(ret == SCE_FONT_OK);
    ret = sceFontSupportExternalFonts(library, 4, SCE_FONT_FORMAT_OPENTYPE | SCE_FONT_FORMAT_OPENTYPE_TT);
    IM_ASSERT(ret == SCE_FONT_OK);

    ret = sceFontCreateRenderer(&memory, sceFontSelectRendererFt(0), &renderer);
    IM_ASSERT(ret == SCE_FONT_OK);

    ImFontAtlasBuildRegisterDefaultCustomRects(atlas);

    atlas->TexID = nullptr;
    atlas->TexWidth = atlas->TexHeight = 0;
    atlas->TexUvScale = ImVec2(0.0f, 0.0f);
    atlas->TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    atlas->ClearTexData();

    ImVector<FreeTypeFont> fonts;
    fonts.resize(1);

    ImVec2 max_glyph_size(1.0f, 1.0f);
    ImVec2 mean_glyph_size(0.0f, 0.0f);

    // Count glyphs/ranges, initialize font
    int total_glyphs_count = 0;
    unsigned int extra_flags = 0;
    int counter = 0;
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];

        // Target font only
        if (cfg.DstFont != target)
        {
            continue;
        }

        FreeTypeFont& font_face = fonts[counter++];
        IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

        font_face.Init(cfg, extra_flags, library, renderer);

        max_glyph_size.x = ImMax(max_glyph_size.x, font_face.Info.MaxAdvanceWidth);
        max_glyph_size.y = ImMax(max_glyph_size.y, font_face.Info.Ascender - font_face.Info.Descender);

        if (!cfg.GlyphRanges)
            cfg.GlyphRanges = atlas->GetGlyphRangesDefault();
        int glyphs_count = 0;

        // Only codes in input string
        const ImWchar *wc = str;
        while (*wc != '\0')
        {
            glyphs_count++;
            wc++;
        }
        total_glyphs_count += glyphs_count;
        mean_glyph_size.x += (font_face.Info.MaxAdvanceWidth) * (float)glyphs_count;
        mean_glyph_size.y += (font_face.Info.Ascender - font_face.Info.Descender) * (float)glyphs_count;
    }
    mean_glyph_size.x /= (float)total_glyphs_count;
    mean_glyph_size.y /= (float)total_glyphs_count;

    // We need a width for the skyline algorithm. Using a dumb heuristic here to decide of width. User can override TexDesiredWidth and TexGlyphPadding if they wish.
    // Width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
    // Need only for first build
    atlas->TexWidth = (atlas->TexDesiredWidth > 0) ? atlas->TexDesiredWidth : (total_glyphs_count > 3000) ? 4096 : (total_glyphs_count > 2000) ? 2048 : (total_glyphs_count > 1000) ? 1024 : 512;

    // We don't do the original first pass to determine texture height, but just rough estimate.
    // Looks ugly inaccurate and excessive, but AFAIK with FreeType we actually need to render glyphs to get exact sizes.
    // Alternatively, we could just render all glyphs into a big shadow buffer, get their sizes, do the rectangle packing and just copy back from the 
    // shadow buffer to the texture buffer. Will give us an accurate texture height, but eat a lot of temp memory. Probably no one will notice.)
    const int total_rects = total_glyphs_count + atlas->CustomRects.size();
    float min_rects_per_row = ceilf((atlas->TexWidth / (mean_glyph_size.x + 1.0f)));
    float min_rects_per_column = ceilf(total_rects / min_rects_per_row);

    s_packNodes->resize(total_rects);

    atlas->TexHeight = (int)(min_rects_per_column * (mean_glyph_size.y + 1.0f));

    // Create texture
    atlas->TexHeight = (atlas->Flags & ImFontAtlasFlags_NoPowerOfTwoHeight) ? (atlas->TexHeight + 1) : ImUpperPowerOfTwo(atlas->TexHeight);
    atlas->TexUvScale = ImVec2(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
    atlas->TexPixelsAlpha8 = (unsigned char*)ImGui::MemAlloc(atlas->TexWidth * atlas->TexHeight);
    IM_ASSERT(atlas->TexPixelsAlpha8);
    memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);

    stbrp_init_target(&s_context, atlas->TexWidth, atlas->TexHeight, s_packNodes->Data, total_rects);

    // Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
    ImFontAtlasBuildPackCustomRects(atlas, &s_context);

    // Render characters, setup ImFont and glyphs for runtime
    counter = 0;
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        if (cfg.DstFont != target)
        {
            // Target font only
            continue;
        }

        FreeTypeFont& font_face = fonts[counter++];

        // create font surface to raster a character
        float dummyPixelSize = cfg.SizePixels * 1.5f;   // Cause Sanitizer HeapOverflow error, so adjust surface size
        void *rasterBuffer = malloc(dummyPixelSize * dummyPixelSize);
        SceFontRenderSurface surface;
        sceFontRenderSurfaceInit(&surface, rasterBuffer, dummyPixelSize, 1, dummyPixelSize, dummyPixelSize);
        sceFontRenderSurfaceSetScissor(&surface, 0, 0, dummyPixelSize, dummyPixelSize);

        ImFont* dst_font = cfg.DstFont;
        if (cfg.MergeMode)
            dst_font->BuildLookupTable();

        const float ascent = font_face.Info.Ascender;
        const float descent = font_face.Info.Descender;

        ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
        
        const float font_off_x = cfg.GlyphOffset.x;
        const float font_off_y = cfg.GlyphOffset.y + (float)(int)(dst_font->Ascent + 0.5f);

        bool multiply_enabled = (cfg.RasterizerMultiply != 1.0f);
        unsigned char multiply_table[256];
        if (multiply_enabled)
            ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);

        const ImWchar *wc = str;
        while (*wc != '\0')
        {
            ImWchar codepoint = *wc++;
            if (cfg.MergeMode && dst_font->FindGlyphNoFallback(codepoint))
                continue;

            GlyphInfo glyph_info;
            memset(rasterBuffer, 0, dummyPixelSize * dummyPixelSize);
            if (!font_face.CalcGlyphInfo(codepoint, glyph_info, surface))
            {
                continue;
            }

            // Pack rectangle
            stbrp_rect rect;
            rect.w = (uint16_t)glyph_info.Width + 1; // Account for texture filtering
            rect.h = (uint16_t)glyph_info.Height + 1;
            stbrp_pack_rects(&s_context, &rect, 1);

            // Copy rasterized pixels to main texture
            uint8_t* blit_dst = atlas->TexPixelsAlpha8 + (rect.y + 1) * atlas->TexWidth + rect.x + 1;
            stbrp_rect blit_rect = rect;
            blit_rect.w -= 1;
            blit_rect.h -= 1;
            ++blit_rect.x;
            ++blit_rect.y;
            font_face.BlitGlyph(surface, blit_rect, blit_dst, atlas->TexWidth, multiply_enabled ? multiply_table : nullptr);

            float char_advance_x_org = glyph_info.AdvanceX;
            float char_advance_x_mod = ImClamp(char_advance_x_org, cfg.GlyphMinAdvanceX, cfg.GlyphMaxAdvanceX);
            float char_off_x = font_off_x;
            if (char_advance_x_org != char_advance_x_mod)
                char_off_x += cfg.PixelSnapH ? (float)(int)((char_advance_x_mod - char_advance_x_org) * 0.5f) : (char_advance_x_mod - char_advance_x_org) * 0.5f;

            // Register glyph
            dst_font->AddGlyph((ImWchar)codepoint,
                glyph_info.OffsetX + char_off_x,
                glyph_info.OffsetY + font_off_y,
                glyph_info.OffsetX + char_off_x + blit_rect.w,
                glyph_info.OffsetY + font_off_y + blit_rect.h,
                blit_rect.x / (float)atlas->TexWidth,
                blit_rect.y / (float)atlas->TexHeight,
                (blit_rect.x + blit_rect.w) / (float)atlas->TexWidth,
                (blit_rect.y + blit_rect.h) / (float)atlas->TexHeight,
                char_advance_x_mod);
        }
        free(rasterBuffer);
    }

    // Cleanup
    for (int n = 0; n < fonts.Size; n++)
        fonts[n].Shutdown();

    ret = sceFontDestroyRenderer(&renderer);
    IM_ASSERT(ret == SCE_FONT_OK);

    ret = sceFontDestroyLibrary(&library);
    IM_ASSERT(ret == SCE_FONT_OK);

    ret = sceFontMemoryTerm(&memory);
    IM_ASSERT(ret == SCE_FONT_OK);

    ImFontAtlasBuildFinish(atlas);
}

// Release something created in "BuildFontAtlasLimitedChar"
void ImGuiLibFont::DestroyFontAtlasLimitedChar(ImFontAtlas* atlas, ImFont *target)
{
    // release texture
    ImGui::MemFree(atlas->TexPixelsAlpha8);
    atlas->TexPixelsAlpha8 = nullptr;

    // release node data of font-glyph
    stbrp_init_target(&s_context, atlas->TexWidth, atlas->TexHeight, s_packNodes->Data, s_packNodes->Size);
    if (s_packNodes) {
        s_packNodes->clear();
    }

    // clear param
    target->ClearOutputData();
}

#endif
