#ifndef COMPONENTASSEMBLY_H
#define COMPONENTASSEMBLY_H
#include <unordered_map>
#include "GameObject.h"
#include "EventListener.h"

namespace NCL {
    namespace CSC8508 {
            class ComponentAssemblyDefiner : public EventListener<AddComponentEvent>
            {
            public:
                /// <summary>
                /// Initialize the IComponent type map for all possible save types
                /// </summary>
                void InitializeMap();

                /// <summary>
                /// On AddComponentEvent add a component of type T corresponding to the entry in the IComponent type map 
                /// </summary>
                /// <param name="e"></param>
                void OnEvent(AddComponentEvent* e) override {
                    AddComponent(e->GetEntry(), e->GetGameObject());
                }
            protected:
                enum ComponentMapId { Error, Bounds, Physics, NavMesh, Animation, Damageable };
                std::unordered_map<size_t, ComponentMapId> componentMap;
                bool AddComponent(size_t t, GameObject& object);

                /// <summary>
                /// Initializes the IComponent type map of type T as a hash for IComponents type paired with their corresponding enum
                /// </summary>
                /// <typeparam name="T">The type corresponding to the component Id</typeparam>
                /// <param name="id">The enum to assigned to this IComponent's type map</param>
                template<typename T>
                requires std::is_base_of_v<IComponent, T>
                void SetHash(ComponentMapId id) {
                    std::string name = T::Name();
                    size_t hashId = SaveManager::MurmurHash3_64(name.c_str(), name.size());
                    componentMap[hashId] = id;
                }
            };
    }
};
#endif //COMPONENTASSEMBLY_H
