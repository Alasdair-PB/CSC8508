/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "AGCRenderer.h"
#include <video_out.h>

using namespace NCL;
using namespace PS5;

const uint32_t DCB_SIZE = 1024 * 1024; //10KB command buffer storage

AGCRenderer::AGCRenderer(Window& w) : RendererBase(w){
	SceError error = sce::Agc::init();

	videoHandle = sceVideoOutOpen(SCE_USER_SERVICE_USER_ID_SYSTEM, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, NULL);
	SCE_AGC_ASSERT_MSG(videoHandle >= 0, "sceVideoOutOpen() returns handle=%d\n", videoHandle);

	sce::Agc::ResourceRegistration::registerDefaultOwner(nullptr);

	const uint32_t dcb_size = 1024 * 1024;

	swapLabels = (sce::Agc::Label*)allocator.Allocate(sizeof(sce::Agc::Label) * SWAPCOUNT, sizeof(uint64_t));

	SceVideoOutBuffers outBuffers[SWAPCOUNT];
	for (int i = 0; i < SWAPCOUNT; ++i) {
		RenderTargetTemp& buffer = backBuffers[i];

		buffer.spec.init();
		buffer.spec.m_width  = SCREENWIDTH;
		buffer.spec.m_height = SCREENHEIGHT;

		buffer.spec.m_format	= { sce::Agc::Core::TypedFormat::k8_8_8_8UNorm, sce::Agc::Core::Swizzle::kRGBA_R4S4 };
		buffer.spec.m_tileMode	= sce::Agc::CxRenderTarget::TileMode::kRenderTarget;

		buffer.renderTarget = CreateColourBufferTarget(buffer.spec);
		buffer.targetMask.init().setMask(0, 0xf);

		outBuffers[i] = {};
		outBuffers[i].data = buffer.renderTarget.getDataAddress();

		swapLabels[i].m_value = 1;

		frameContexts[i].m_dcb.init(
			allocator.Allocate(DCB_SIZE, sce::Agc::Alignment::kCommandBuffer),
			DCB_SIZE,
			nullptr,	// This would be the out-of-memory callback.
			nullptr);	// This would be the user defined payload for the out-of-memory callback.

		frameContexts[i].m_bdr.init(
			&frameContexts[i].m_dcb,
			&frameContexts[i].m_dcb);
		frameContexts[i].m_sb.init(
			256, // This is the size of a chunk in the StateBuffer, defining the largest size of a load packet's payload.
			&frameContexts[i].m_dcb,
			&frameContexts[i].m_dcb);
	}

	SceVideoOutBufferAttribute2 attribute;
	sce::Agc::Core::translate(&attribute, &backBuffers[0].spec, sce::Agc::Core::Colorimetry::kSrgb, sce::Agc::Core::Colorimetry::kBt709);
	int32_t ret = sceVideoOutRegisterBuffers2(videoHandle, videoOutputSet, 0, outBuffers, SWAPCOUNT, &attribute, SCE_VIDEO_OUT_BUFFER_ATTRIBUTE_CATEGORY_UNCOMPRESSED, nullptr);

	depthTarget = CreateDepthBufferTarget(SCREENWIDTH,SCREENHEIGHT);

	defaultSampler.init()
		.setXyFilterMode(
			sce::Agc::Core::Sampler::FilterMode::kBilinear,			//magnification
			sce::Agc::Core::Sampler::FilterMode::kAnisoBilinear		//minificaction
		)
		.setMipFilterMode(sce::Agc::Core::Sampler::MipFilterMode::kLinear);

	pixelSampler.init()
		.setXyFilterMode(
			sce::Agc::Core::Sampler::FilterMode::kPoint,	//magnification
			sce::Agc::Core::Sampler::FilterMode::kPoint		//minificaction
		)
		.setMipFilterMode(sce::Agc::Core::Sampler::MipFilterMode::kPoint);
}

AGCRenderer::~AGCRenderer() {

}

sce::Agc::CxDepthRenderTarget	 AGCRenderer::CreateDepthBufferTarget(sce::Agc::Core::DepthRenderTargetSpec& depthSpec) {
	sce::Agc::CxDepthRenderTarget target;

	SceError error = sce::Agc::Core::initialize(&target, &depthSpec);
	SCE_AGC_ASSERT_MSG(error == SCE_OK, "Failed to initialize Depth RenderTarget.");

	target.setStencilWrite(sce::Agc::CxDepthRenderTarget::StencilWrite::kDisable);

	target.setDepthClearValue(1.0f);

	return target;
}

sce::Agc::CxDepthRenderTarget AGCRenderer::CreateDepthBufferTarget(uint32_t width, uint32_t height) {
	sce::Agc::Core::DepthRenderTargetSpec depthSpec;
	depthSpec.init();
	depthSpec.m_width = width;
	depthSpec.m_height = height;

	depthSpec.setDepthFormat(sce::Agc::CxDepthRenderTarget::DepthFormat::k32Float);
	depthSpec.setStencilFormat(sce::Agc::CxDepthRenderTarget::StencilFormat::kInvalid);

	sce::Agc::SizeAlign allocationSize = sce::Agc::Core::getSize(&depthSpec);

	depthSpec.setDepthAddress(allocator.Allocate(allocationSize.m_size, allocationSize.m_align));

	return CreateDepthBufferTarget(depthSpec);
}

sce::Agc::CxRenderTarget AGCRenderer::CreateColourBufferTarget(sce::Agc::Core::RenderTargetSpec& renderSpec) {
	sce::Agc::SizeAlign allocationSize = sce::Agc::Core::getSize(&renderSpec);

	renderSpec.setDataAddress(allocator.Allocate(allocationSize.m_size, allocationSize.m_align));

	sce::Agc::SizeAlign cmaskSizeAlign	= sce::Agc::Core::getSize(&renderSpec, sce::Agc::Core::RenderTargetComponent::kCmask);
	sce::Agc::SizeAlign fmaskSizeAlign	= sce::Agc::Core::getSize(&renderSpec, sce::Agc::Core::RenderTargetComponent::kFmask);
	sce::Agc::SizeAlign dccSizeAlign	= sce::Agc::Core::getSize(&renderSpec, sce::Agc::Core::RenderTargetComponent::kDcc);

	renderSpec.m_cmaskAddress	= allocator.Allocate(cmaskSizeAlign.m_size, cmaskSizeAlign.m_align);
	renderSpec.m_fmaskAddress	= allocator.Allocate(fmaskSizeAlign.m_size, fmaskSizeAlign.m_align);
	renderSpec.m_dccAddress		= allocator.Allocate(dccSizeAlign.m_size, dccSizeAlign.m_align);

	sce::Agc::CxRenderTarget target;

	SceError error = sce::Agc::Core::initialize(&target, &renderSpec);
	SCE_AGC_ASSERT_MSG(error == SCE_OK, "Failed to initialize Colour RenderTarget.");

	return target;
}

sce::Agc::CxRenderTarget AGCRenderer::CreateColourBufferTarget(uint32_t width, uint32_t height, bool isFloat) {
	sce::Agc::Core::RenderTargetSpec renderSpec;
	renderSpec.init();
	renderSpec.m_width = width;
	renderSpec.m_height = height;

	if (isFloat) {
		renderSpec.m_format = { sce::Agc::Core::TypedFormat::k16_16_16_16Float, sce::Agc::Core::Swizzle::kRGBA_R4S4 };
	}
	else {
		renderSpec.m_format = { sce::Agc::Core::TypedFormat::k8_8_8_8UNorm, sce::Agc::Core::Swizzle::kRGBA_R4S4 };
	}

	renderSpec.m_tileMode = sce::Agc::CxRenderTarget::TileMode::kRenderTarget;

	return CreateColourBufferTarget(renderSpec);
}

void AGCRenderer::SwapBuffers() {
	frameContexts[currentSwap].m_dcb.setFlip(videoHandle, currentSwap, SCE_VIDEO_OUT_FLIP_MODE_VSYNC, 0);

	sce::Agc::Core::gpuSyncEvent(
		&frameContexts[currentSwap].m_dcb,
		// The SyncWaitMode controls how the GPU's Command Processor (CP) handles the synchronization.
		// By setting this to kAsynchronous, we tell the CP that it doesn't have to wait for this operation
		// to finish before it can start the next frame. Instead, we could ask it to drain all graphics work
		// first, but that would be more aggressive than we need to be here.
		sce::Agc::Core::SyncWaitMode::kAsynchronous,
		// Request dirty cache lines in GL2 to be written to memory. 
		sce::Agc::Core::SyncCacheOp::kGl2Writeback,
		// Write the flip label and make it visible to the CPU.
		sce::Agc::Core::SyncLabelVisibility::kCpu,
		&swapLabels[currentSwap],
		// We write the value "1" to the flip label.
		1);

	sce::Agc::submitGraphics(
		sce::Agc::GraphicsQueue::kNormal,
		frameContexts[currentSwap].m_dcb.getSubmitPointer(),
		frameContexts[currentSwap].m_dcb.getSubmitSize());

	sce::Agc::suspendPoint();

	currentSwap = (currentSwap + 1) % SWAPCOUNT;
}

void AGCRenderer::OnWindowResize(int w, int h) {

}

void AGCRenderer::BeginFrame() {
	while (swapLabels[currentSwap].m_value != 1)
	{
		sceKernelUsleep(1000);
	}
	//If we get to here, the previous frame is defnitely finished, we can now reset the fence.
	swapLabels[currentSwap].m_value = 0;

	frameContext = &frameContexts[currentSwap];

	frameContext->reset();
	frameContext->m_dcb.resetBuffer();
	frameContext->m_dcb.waitUntilSafeForRendering(videoHandle, currentSwap);

	sce::Agc::CxViewport viewPort;
	sce::Agc::Core::setViewport(&viewPort, SCREENWIDTH, SCREENHEIGHT, 0, 0, -1.0f, 1.0f);

	frameContext->m_sb.reset();
	frameContext->m_sb.setState(viewPort);
	frameContext->m_sb.setState(backBuffers[currentSwap].targetMask);
	frameContext->m_sb.setState(backBuffers[currentSwap].renderTarget);
	frameContext->m_sb.setState(depthTarget);

	sce::Agc::ShUserDataGs userData;
	userData
		.init()
		.setData(0xffffffff);
	frameContext->m_dcb.setShRegisterDirect(userData.m_regs[0]);

	sce::Agc::CxPrimitiveSetup primState;
	primState.init();
	frameContext->m_sb.setState(primState);

	frameContext->m_dcb.setIndexSize(sce::Agc::IndexSize::k32, sce::Agc::GeCachePolicy::kBypass);

	//// Clear both color and depth targets by just using toolkit functions.
	sce::Agc::Core::Encoder::EncoderValue clearColor = sce::Agc::Core::Encoder::encode(backBuffers[currentSwap].spec.getFormat(), {1,0,0,1});
	sce::Agc::Toolkit::Result tk0 = sce::Agc::Toolkit::clearRenderTargetCs(&frameContext->m_dcb, &backBuffers[currentSwap].renderTarget, clearColor);
	sce::Agc::Toolkit::Result tk1 = sce::Agc::Toolkit::clearDepthRenderTargetCs(&frameContext->m_dcb, &depthTarget);
	sce::Agc::Toolkit::Result wat = frameContext->resetToolkitChangesAndSyncToGl2(tk0 | tk1);
}

void AGCRenderer::EndFrame() {

}

void AGCRenderer::DrawBoundMesh(sce::Agc::Core::BasicContext& context, AGCMesh& m) {
	if (m.GetIndexCount() > 0) {
		context.drawIndex(m.GetIndexCount(), m.GetAGCIndexData());
	}
	else {
		context.drawIndexAuto(m.GetVertexCount());
	}
}

void AGCRenderer::DrawBoundMeshInstanced(sce::Agc::Core::BasicContext& context, AGCMesh& m, uint32_t instanceCount) {
	context.m_dcb.setNumInstances(instanceCount);
	if (m.GetIndexCount() > 0) {
		context.drawIndex(m.GetIndexCount(), m.GetAGCIndexData());
	}
	else {
		context.drawIndexAuto(m.GetVertexCount());
	}
	context.m_dcb.setNumInstances(1);
}

void AGCRenderer::CreateTriangle(AGCMesh* m) {
	m->SetVertexPositions({
		Vector3(0, 0.5, 0),
		Vector3(0.5, -0.5, 0),
		Vector3(-0.5, -0.5, 0)
	});

	m->SetVertexTextureCoords({
		Vector2(0.5, 1.0),
		Vector2(1.0, 0.0),
		Vector2(0.0, 0.0)
	});
}

void AGCRenderer::CreateQuad(AGCMesh* m) {
	m->SetVertexPositions({
		Vector3(-1, -1, 0),
		Vector3(1, -1, 0),
		Vector3(-1, 1, 0),

		Vector3(-1, 1, 0),
		Vector3(1, 1, 0),
		Vector3(1, -1, 0)
	});

	m->SetVertexTextureCoords({
		Vector2(0, 0),
		Vector2(1, 0),
		Vector2(0, 1),

		Vector2(0, 1),
		Vector2(1, 1),
		Vector2(1, 0)
	});

	m->SetVertexNormals({
		Vector3(0, 0, -1),
		Vector3(0, 0, -1),
		Vector3(0, 0, -1),

		Vector3(0, 0, -1),
		Vector3(0, 0, -1),
		Vector3(0, 0, -1),
	});


	m->SetVertexIndices({
		0,1,2,
		3,4,5
	});
}