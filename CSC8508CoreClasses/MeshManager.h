#ifndef MESHMANAGER_H
#define MESHMANAGER_H

#include <vector>
#include "Mesh.h"
#include "RenderObject.h"
#include <unordered_map>

namespace NCL {
    namespace CSC8508 {
        using std::vector;
        class MeshManager final {
        public:
            static void PushMesh(std::string name, Mesh* mesh) {
                allMeshes[name] = mesh;
            }

            static Mesh* GetMesh(std::string name) {
                return allMeshes[name];
            }

            static void CleanUp() {
                for (auto mesh : allMeshes)
                    delete mesh.second;
                allMeshes.clear();
            }

        private:
            MeshManager() = default;
            inline static std::unordered_map<std::string, Mesh*> allMeshes;

        };
    }
}

#endif // MESHMANAGER_H
