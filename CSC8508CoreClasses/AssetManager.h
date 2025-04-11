#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "Texture.h"
#include "MaterialManager.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <set>
#include "IGameTechRenderer.h"
#include "Assets.h"
namespace fstream = std::filesystem;

namespace NCL {
    namespace CSC8508 {
        using std::vector;
        class AssetManager final {
        public:

            // Primatives should be in an Editor Folder (will do big project split in the future)
            // Legacy mshLoader first needs to be decoupled from assets folder as do other renderer functions
            static void LoadPrimativeAssets(IGameTechRenderer* renderer) {
                MaterialManager::PushTexture("basic", renderer->LoadTexture("checkerboard.png"));
                MaterialManager::PushShader("basic", renderer->LoadShader("scene.vert", "scene.frag"));
                MaterialManager::PushMesh("capsule", renderer->LoadMesh("capsule.msh"));
                MaterialManager::PushMesh("sphere", renderer->LoadMesh("sphere.msh"));
                MaterialManager::PushMesh("cube", renderer->LoadMesh("cube.msh"));
            }

            static void LoadMaterials(IGameTechRenderer* renderer) {
                MaterialManager::PushTexture("basic", renderer->LoadTexture("checkerboard.png"));
                MaterialManager::PushTexture("player", renderer->LoadTexture("MiiCharacter.png"));

                MaterialManager::PushShader("basic", renderer->LoadShader("scene.vert", "scene.frag"));
                MaterialManager::PushShader("anim", renderer->LoadShader("skinning.vert", "scene.frag"));

                std::string path = NCL::Assets::ASSETROOT + "Meshes/";
                for (const auto& entry : fstream::directory_iterator(path)) {
                    std::string filename = entry.path().stem().string();
                    std::string filenameExt = entry.path().filename().string();
                    if (entry.path().extension() == ".msh")
                        MaterialManager::PushMesh(filename, renderer->LoadMesh(filenameExt));
                }  

               path = NCL::Assets::ASSETROOT + "Textures/";
                for (const auto& entry : fstream::directory_iterator(path)) {
                    std::string filename = entry.path().stem().string();
                    std::string filenameExt = entry.path().filename().string();
                    if (entry.path().extension() == ".png")
                        MaterialManager::PushTexture(filename, renderer->LoadTexture(filenameExt));
                }

                
                //if (entry.path().extension() == ".png" || entry.path().extension() == ".PNG")
                    //    MaterialManager::PushTexture(filename, renderer->LoadTexture(filenameExt));
            }


        private:
            AssetManager() = default;

        };
    }
}

#endif // ASSETMANAGER_H
