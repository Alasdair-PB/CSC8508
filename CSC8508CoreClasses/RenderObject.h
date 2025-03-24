#pragma once
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include "Buffer.h"

namespace NCL {
	using namespace NCL::Rendering;

	namespace CSC8508 {
		class Transform;
		using namespace Maths;

		class RenderObject
		{
		public:
			RenderObject(Transform* inTransform, Mesh* inMesh, Texture* inTex, Shader* inShader) {
				buffer = nullptr;

				transform = inTransform;
				mesh = inMesh;
				texture = inTex;
				shader = inShader;
				colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			}
			~RenderObject() {}

			void SetDefaultTexture(Texture* t) {
				texture = t;
			}

			Buffer* GetGPUBuffer() const {
				return buffer;
			}

			void SetGPUBuffer(Buffer* b) {
				buffer = b;
			}

			Texture* GetDefaultTexture() const {
				return texture;
			}

			Mesh* GetMesh() const {
				return mesh;
			}

			Transform* GetTransform() const {
				return transform;
			}

			Shader* GetShader() const {
				return shader;
			}

			void SetColour(const Vector4& c) {
				colour = c;
			}

			Vector4 GetColour() const {
				return colour;
			}	

			float GetCameraDistance() const { return distanceFromCamera; }
			void SetCameraDistance(float f) { distanceFromCamera = f; }
			static const bool CompareByCameraDistance(const RenderObject* a, const RenderObject* b) {
				return a->distanceFromCamera < b->distanceFromCamera;
			}

		protected:
			Buffer* buffer;
			Mesh* mesh;
			Texture* texture;
			Shader* shader;
			Transform* transform;
			Vector4	colour;

			float distanceFromCamera;
		};
	}
}
