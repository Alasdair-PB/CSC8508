#ifndef MESHMANAGER_H
#define MESHMANAGER_H

#include <vector>
#include "Mesh.h"
#include "RenderObject.h"

namespace NCL {
    namespace CSC8508 {
        using std::vector;
        class MeshManager final {
        public:
            void PushMesh(Mesh* mesh, size_t id) {
                allMeshes.push_back(mesh);
            }

            void CleanUp() {
                for (auto mesh : allMeshes)
                    delete mesh;
                allMeshes.clear();
            }

        private:
            MeshManager() = default;
            vector<Mesh*> allMeshes;
        };
    }
}

#endif // MESHMANAGER_H
