#pragma once

namespace NCL::Rendering {
	class Mesh;
	class Texture;
	class Shader;
}

namespace NCL::CSC8508 {
	class UISystem;
	class GameTechRendererInterface
	{
	public:
		virtual NCL::Rendering::Mesh* LoadMesh(const std::string& name) = 0;
		virtual NCL::Rendering::Texture* LoadTexture(const std::string& name) = 0;
		virtual NCL::Rendering::Shader* LoadShader(const std::string& vertex, const std::string& fragment) = 0;
		virtual void Update(float dt) {}
		virtual void Render() {}
		virtual UISystem* GetUI() = 0;
		virtual void StartUI() {};
	};
}

