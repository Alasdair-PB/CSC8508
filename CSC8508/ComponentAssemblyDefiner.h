#ifndef COMPONENTASSEMBLY_H
#define COMPONENTASSEMBLY_H
#include <unordered_map>
#include "GameObject.h"
#include "EventManager.h"

namespace NCL {
    namespace CSC8508 {
            class ComponentAssemblyDefiner : public EventListener<AddComponentEvent>
            {
                enum ComponentMapId {Error, Bounds, Physics, NavMesh};
                std::unordered_map<size_t, ComponentMapId> componentMap;
                bool AddComponent(size_t t, GameObject& object);
            public:
                void InitializeMap();

                template<typename T>
                void SetHash(ComponentMapId id) {
                    componentMap[SaveManager::MurmurHash3_64(typeid(T).name(), std::strlen(typeid(T).name()))] = id;
                }

                void OnEvent(AddComponentEvent* e) override {
                    AddComponent(e->GetEntry(), e->GetGameObject());
                }
            };
    }
};
#endif //COMPONENTASSEMBLY_H
