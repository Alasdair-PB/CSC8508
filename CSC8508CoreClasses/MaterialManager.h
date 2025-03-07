#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H

#include "Texture.h"
#include "RenderObject.h"
#include "SaveManager.h"
#include <unordered_map>

namespace NCL {
    namespace CSC8508 {
        using std::vector;
        class MaterialManager final {
        public:

            static size_t GetHashId(std::string name) {
                const char* charName = name.c_str();
                return SaveManager::MurmurHash3_64(charName, std::strlen(charName));
            }

            static void PushTexture(std::string name, Texture* texture) { 
                allTextureNameMap[name] = texture;
                allTextureIdMap[GetHashId(name)] = texture;
            }

            static void PushShader(std::string name, Shader* shader) { 
                allShadersNameMap[name] = shader;
                allShadersIdMap[GetHashId(name)] = shader;
            }

            static void PushMesh(std::string name, Mesh* mesh) {
                allMeshesNameMap[name] = mesh;
                const char* charName = name.c_str();
                allMeshesIdMap[SaveManager::MurmurHash3_64(charName, std::strlen(charName))] = mesh;
            }

            static Texture* GetTexture(std::string name) {
                return allTextureNameMap.contains(name) ? allTextureNameMap[name] : nullptr;
            }

            static Shader* GetShader(std::string name) {
                return allShadersNameMap.contains(name) ? allShadersNameMap[name] : nullptr;
            }

            static Shader* GetShader(size_t name) {
                return allShadersIdMap.contains(name) ? allShadersIdMap[name] : nullptr;
            }

            static Texture* GetTexture(size_t name) {
                return allTextureIdMap.contains(name) ? allTextureIdMap[name] : nullptr;
            }     
            
            static Mesh* GetMesh(std::string name) {
                return allMeshesNameMap.contains(name) ? allMeshesNameMap[name] : nullptr;
            }

            static Mesh* GetMesh(size_t id) {
                return allMeshesIdMap.contains(id) ? allMeshesIdMap[id] : nullptr;
            }
            template<typename T>
            static size_t FindKeyByValue(T* pointer, std::unordered_map<size_t, T*> iDMap) {
                for (const auto& [key, value] : iDMap) {
                    if (value == pointer)
                        return key;
                }
                return 0;
            }

            static size_t GetShaderPointer(Shader* shaderPointer) {
                return FindKeyByValue<Shader>(shaderPointer, allShadersIdMap);
            }

            static size_t GetMeshPointer(Mesh* shaderPointer) {
                return FindKeyByValue<Mesh>(shaderPointer, allMeshesIdMap);
            }

            static size_t GetTexturePointer(Texture* texturePointer) {
                return FindKeyByValue<Texture>(texturePointer, allTextureIdMap);
            }


            static void CleanUp() {
                for (auto texture : allTextureNameMap)
                    delete texture.second;
                for (auto texture : allTextureIdMap)
                    delete texture.second;
                for (auto shader : allShadersNameMap)
                    delete shader.second;
                for (auto shader : allShadersIdMap)
                    delete shader.second;
                for (auto mesh : allMeshesNameMap)
                    delete mesh.second;
                for (auto mesh : allMeshesIdMap)
                    delete mesh.second;

                allMeshesNameMap.clear();
                allMeshesIdMap.clear();
                allTextureNameMap.clear();
                allTextureIdMap.clear();
                allShadersNameMap.clear();
                allShadersIdMap.clear();

            }

        private:
            MaterialManager() = default;
            inline static std::unordered_map<std::string, Texture*> allTextureNameMap;
            inline static std::unordered_map<std::string, Shader*> allShadersNameMap;
            inline static std::unordered_map<std::string, Mesh*> allMeshesNameMap;

            inline static std::unordered_map<size_t, Texture*> allTextureIdMap;
            inline static std::unordered_map<size_t, Shader*> allShadersIdMap;
            inline static std::unordered_map<size_t, Mesh*> allMeshesIdMap;
        };
    }
}

#endif // MATERIALMANAGER_H
