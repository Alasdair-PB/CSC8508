/******************************************************************************
This file is part of the Newcastle OpenGL Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "RendererBase.h"
#include "../CSC8508/Core/GameTechRendererInterface.h"

#ifdef _WIN32
#include "windows.h"
#endif

#ifdef _DEBUG
#define OPENGL_DEBUGGING
#endif

namespace NCL::Rendering {	
	class Mesh;
	class Shader;
	class Texture;

	class OGLMesh;
	class OGLShader;
	class OGLTexture;
	class OGLBuffer;

	class SimpleFont;
		
	class OGLRenderer : public RendererBase, public CSC8508::GameTechRendererInterface {
	public:
		friend class OGLRenderer;
		OGLRenderer(Window& w);
		~OGLRenderer();

		void OnWindowResize(int w, int h)	override;
		bool HasInitialised()				const override {
			return initState;
		}

		virtual bool SetVerticalSync(VerticalSyncState s);

		void Render() { RendererBase::Render(); };
		void Update(float dt) { RendererBase::Update(dt); };

	protected:			
		void BeginFrame()	override;
		void RenderFrame()	override;
		void EndFrame()		override;
		void SwapBuffers()  override;

		void UseShader(const OGLShader& s);
		void BindTextureToShader(const OGLTexture& t, const std::string& uniform, int texUnit) const;
		void BindMesh(const OGLMesh& m);
		void BindBufferAsUBO(const OGLBuffer& b, uint32_t slotID);
		void BindBufferAsSSBO(const OGLBuffer& b, uint32_t slotID);

		void DrawBoundMesh(int subLayer = 0, int numInstances = 1);
#ifdef _WIN32
		void InitWithWin32(Window& w);
		void DestroyWithWin32();
		HDC		deviceContext;		//...Device context?
		HGLRC	renderContext;		//Permanent Rendering Context	
#endif

		const OGLMesh*		boundMesh;
		const OGLShader*	activeShader;
	private:
		bool initState;
	};
}