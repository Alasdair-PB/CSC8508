#pragma once
#ifdef USE_PS5
#include "../CSC8508CoreClasses/GameWorld.h"

#include "../CSC8508/GameTechRendererInterface.h"

#include "../PS5Core/AGCRenderer.h"

#include "../PS5Core/AGCBuffer.h"

#include "../Assets/Shaders/PSSL/Interop.h"				//Always include this before any PSSL headers
#include "../Assets/Shaders/PSSL/ShaderConstants.psslh"
#include "../Assets/Shaders/PSSL/TechObject.psslh"

namespace NCL {
	namespace Rendering {
		class Mesh;
		class Texture;
		class Shader;
	}
	namespace PS5 {
		class AGCTexture;
		class AGCShader;
	}

	namespace CSC8508 {
		class RenderObject;

		class GameTechAGCRenderer : 
			public NCL::PS5::AGCRenderer,
			public NCL::CSC8508::GameTechRendererInterface	
		{
		public:
			GameTechAGCRenderer(GameWorld& world);
			~GameTechAGCRenderer();

			virtual Mesh*		LoadMesh(const std::string& name)				override;
			virtual Texture*	LoadTexture(const std::string& name)			override;
			virtual Shader*		LoadShader(const std::string& vertex, const std::string& fragment)	override;

			void Render() override { NCL::PS5::AGCRenderer::Render(); }
			void Update(float dt) override { NCL::PS5::AGCRenderer::Update(dt); }

		protected:
			void RenderFrame()	override;
			void UpdateObjectList();

			NCL::PS5::AGCTexture* CreateFrameBufferTextureSlot(const std::string& name);

			vector<const RenderObject*> activeObjects;

			void WriteRenderPassConstants();
			void DrawObjects();
			void UpdateDebugData();
			
			void RenderDebugLines();
			void RenderDebugText();

			void ShadowmapPass();
			void SkyboxPass();
			void MainRenderPass();

			void DisplayRenderPass();

			void GPUSkinningPass();

			void InitialiseImGui();

			Shader*		defaultShader;

			GameWorld&	gameWorld;

			/*
			Handling buffers in AGC isn't too bad, as they are a small wrapper around an existing
			memory allocation. Here I have a small struct that will fill out a memory allocation with
			all of the data required by the frame. We can then make Buffers out of this at any
			offset we want to send to our shaders - in this case we're going to use one big allocation
			to hold both the constants used by shaders, as well as all of the debug vertices, and object
			matrices. No fancy suballocations here, the allocator is as simple as it gets - it just 
			advances or 'bumps' a pointer along. Perfect for recording a frame's data to memory!
			*/
			struct BumpAllocator {
				char* dataStart;//Start of our allocated memory
				char* data;		//Current write point of our memory
				size_t			bytesWritten;

				template<typename T>
				void WriteData(T value) {
					memcpy(data, &value, sizeof(T));
					data += sizeof(T);
					bytesWritten += sizeof(T);
				}
				void WriteData(void* inData, size_t byteCount) {
					memcpy(data, inData, byteCount);
					data += byteCount;
					bytesWritten += byteCount;
				}

				void AlignData(size_t alignment) {
					char* oldData = data;
					data = (char*)((((uintptr_t)data + alignment - 1) / (uintptr_t)alignment) * (uintptr_t)alignment);
					bytesWritten += data - oldData;
				}

				void Reset() {
					bytesWritten = 0;
					data = dataStart;
				}
			};

			struct FrameData {
				sce::Agc::Core::Buffer constantBuffer;
				sce::Agc::Core::Buffer objectBuffer;

				sce::Agc::Core::Buffer debugLineBuffer;
				sce::Agc::Core::Buffer debugTextBuffer;

				BumpAllocator data;

				int globalDataOffset	= 0;	//Where does the global data start in the buffer?
				int objectStateOffset	= 0;	//Where does the object states start?
				int debugLinesOffset	= 0;	//Where do the debug lines start?
				int debugTextOffset		= 0;	//Where do the debug text verts start?

				size_t lineVertCount = 0;
				size_t textVertCount = 0;
			};

			struct SkinningJob {
				GameObject* object;
				uint32_t outputIndex;
			};

			FrameData* allFrames;
			FrameData* currentFrame;
			int currentFrameIndex;

			NCL::PS5::AGCMesh* quadMesh;

			sce::Agc::Core::Texture*	bindlessTextures;
			sce::Agc::Core::Buffer*		bindlessBuffers;
			uint32_t bufferCount;

			sce::Agc::Core::Buffer textureBuffer;
			std::map<std::string, NCL::PS5::AGCTexture*> textureMap;

			sce::Agc::Core::Buffer arrayBuffer;

			NCL::PS5::AGCTexture* defaultTexture;
			NCL::PS5::AGCTexture* skyboxTexture;

			NCL::PS5::AGCShader* skinningCompute;

			NCL::PS5::AGCShader* defaultVertexShader;
			NCL::PS5::AGCShader* defaultPixelShader;

			NCL::PS5::AGCShader* shadowVertexShader;
			NCL::PS5::AGCShader* shadowPixelShader;

			NCL::PS5::AGCShader* skyboxVertexShader;
			NCL::PS5::AGCShader* skyboxPixelShader;

			NCL::PS5::AGCShader* debugLineVertexShader;
			NCL::PS5::AGCShader* debugLinePixelShader;

			NCL::PS5::AGCShader* debugTextVertexShader;
			NCL::PS5::AGCShader* debugTextPixelShader;

			NCL::PS5::AGCShader* gammaCompute;

			sce::Agc::CxDepthRenderTarget		shadowTarget;
			NCL::PS5::AGCTexture*				shadowMap; //ptr into bindless array
			sce::Agc::Core::Sampler				shadowSampler;

			sce::Agc::CxRenderTarget			screenTarget;
			NCL::PS5::AGCTexture*				screenTex; //ptr into bindless array

			std::vector<SkinningJob> frameJobs;
		};
	}
}
#endif // !USE_PS5