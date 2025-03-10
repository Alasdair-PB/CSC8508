#pragma once
#ifndef USE_PS5
#include "../OpenGLRendering/OGLRenderer.h"
#include "../OpenGLRendering/OGLShader.h"
#include "../OpenGLRendering/OGLTexture.h"
#include "../OpenGLRendering/OGLMesh.h"

#include "GameWorld.h"

namespace NCL {
	namespace CSC8508 {
		class RenderObject;
		class UISystem;

		class GameTechRenderer : public OGLRenderer {
		public:
			GameTechRenderer(GameWorld& world);
			~GameTechRenderer();

			Mesh* LoadMesh(const std::string& name);
			Texture* LoadTexture(const std::string& name);
			Shader* LoadShader(const std::string& vertex, const std::string& fragment);

			UISystem* GetUI() override { return uiSystem; };

		protected:
			void NewRenderLines();
			void NewRenderText();
			void NewRenderTextures();

			void RenderFrame()	override;

			OGLShader* defaultShader;

			GameWorld& gameWorld;

			void BuildObjectList();
			void SortObjectList();
			void RenderShadowMap();
			void RenderCamera();
			void RenderSkybox();

			void LoadSkybox();
			void StartUI();

			void SetDebugStringBufferSizes(size_t newVertCount);
			void SetDebugLineBufferSizes(size_t newVertCount);
			vector<const RenderObject*> activeObjects;

			OGLShader* debugShader;
			OGLShader* skyboxShader;
			OGLMesh* skyboxMesh;
			OGLMesh* debugTexMesh;
			GLuint		skyboxTex;

			//shadow mapping things
			OGLShader* shadowShader;
			GLuint		shadowTex;
			GLuint		shadowFBO;
			Matrix4     shadowMatrix;

			Vector4		lightColour;
			float		lightRadius;
			Vector3		lightPosition;

			//Debug data storage things
			vector<Vector3> debugLineData;

			vector<Vector3> debugTextPos;
			vector<Vector4> debugTextColours;
			vector<Vector2> debugTextUVs;

			GLuint lineVAO;
			GLuint lineVertVBO;
			size_t lineCount;

			GLuint textVAO;
			GLuint textVertVBO;
			GLuint textColourVBO;
			GLuint textTexVBO;
			size_t textCount;

			UISystem* uiSystem;
		};
	}
}
#endif // DEBUG


