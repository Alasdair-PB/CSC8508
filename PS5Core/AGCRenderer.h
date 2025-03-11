/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "RendererBase.h"
#include "PS5MemoryAllocator.h"
#include "AGCTexture.h"
#include "AGCMesh.h"

#include <agc.h>
#include <string>

static const uint32_t SWAPCOUNT = 2;

static const uint32_t SCREENWIDTH = 1920;
static const uint32_t SCREENHEIGHT = 1080;

struct PS5Shader {
	uint8_t* allocation = nullptr;
	sce::Agc::Shader* binary = nullptr;
};

namespace NCL::PS5 {
	class AGCRenderer : public NCL::Rendering::RendererBase {
	public:
		AGCRenderer(Window& w);
		~AGCRenderer();

		const MemoryAllocator& GetAllocator() const {
			return allocator;
		}

	protected:
		void OnWindowResize(int w, int h) override;
		void OnWindowDetach() override {}; //Most renderers won't care about this

		void BeginFrame()	override;
		void EndFrame()		override;
		void SwapBuffers() override;

		void DrawBoundMesh(sce::Agc::Core::BasicContext& context, AGCMesh& m);
		void DrawBoundMeshInstanced(sce::Agc::Core::BasicContext& context, AGCMesh& m, uint32_t instanceCount);
		void CreateTriangle(AGCMesh* m);
		void CreateQuad(AGCMesh* m);

		sce::Agc::CxDepthRenderTarget	CreateDepthBufferTarget(uint32_t width, uint32_t height);
		sce::Agc::CxDepthRenderTarget	CreateDepthBufferTarget(sce::Agc::Core::DepthRenderTargetSpec& spec);

		sce::Agc::CxRenderTarget		CreateColourBufferTarget(uint32_t width, uint32_t height, bool isFloat = false);
		sce::Agc::CxRenderTarget		CreateColourBufferTarget(sce::Agc::Core::RenderTargetSpec& spec);

		uint32_t currentSwap = 0;
		int videoHandle = 0;
		int	videoOutputSet = 0;

		sce::Agc::Label* swapLabels;
		sce::Agc::Core::BasicContext frameContexts[SWAPCOUNT];

		struct RenderTargetTemp {
			sce::Agc::Core::RenderTargetSpec	spec;
			sce::Agc::CxRenderTarget			renderTarget;
			sce::Agc::CxRenderTargetMask		targetMask;
		};

		RenderTargetTemp backBuffers[SWAPCOUNT];

		sce::Agc::Core::BasicContext* frameContext = nullptr;

		sce::Agc::Core::Sampler defaultSampler;
		sce::Agc::Core::Sampler pixelSampler;

		MemoryAllocator allocator;

		sce::Agc::CxDepthRenderTarget			depthTarget;
	};
}
