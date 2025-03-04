#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H

#include "Texture.h"
#include "RenderObject.h"
#include <unordered_map>

namespace NCL {
    namespace CSC8508 {
        using std::vector;
        class MaterialManager final {
        public:
            static void PushTexture(std::string name, Texture* texture) {
                allTexture[name] = texture;
            }

            static void PushShader(std::string name, Shader* texture) {
                allShaders[name] = texture;
            }

            static Texture* GetTexture(std::string name) {
                return allTexture[name];
            }

            static Shader* GetShader(std::string name) {
                return allShaders[name];
            }

            static void CleanUp() {
                for (auto texture : allTexture)
                    delete texture.second;
                for (auto shader : allShaders)
                    delete shader.second;
                allTexture.clear();
                allShaders.clear();
            }

        private:
            MaterialManager() = default;
            inline static std::unordered_map<std::string, Texture*> allTexture;
            inline static std::unordered_map<std::string, Shader*> allShaders;

        };
    }
}

#endif // MATERIALMANAGER_H
