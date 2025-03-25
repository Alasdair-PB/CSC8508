#ifndef BINDINGMANAGER_H
#define BINDINGMANAGER_H

#include "RenderObject.h"
#include "SaveManager.h"
#include <unordered_map>

namespace NCL {
    namespace CSC8508 {
        using std::vector;
        class BindingManager final {
        public:

            static size_t GetHashId(std::string name) {
                const char* charName = name.c_str();
                return SaveManager::MurmurHash3_64(charName, std::strlen(charName));
            }

            static void PushTexture(std::string name, Mesh* texture) { 
                allMeshesNameMap[name] = texture;
            }

            static Mesh* GetMesh(std::string name) {
                return allMeshesNameMap.contains(name) ? allMeshesNameMap[name] : nullptr;
            }

            template<typename T>
            static size_t FindKeyByValue(T* pointer, std::unordered_map<size_t, T*> iDMap) {
                for (const auto& [key, value] : iDMap) {
                    if (value == pointer)
                        return key;
                }
                return 0;
            }

            static void CleanUp() {
                for (auto mesh : allMeshesNameMap)
                    delete mesh.second;
                allMeshesNameMap.clear();
            }

        private:
            BindingManager() = default;
            inline static std::unordered_map<std::string, Mesh*> allMeshesNameMap;
        };
    }
}

#endif // BINDINGMANAGER_H
