#ifndef COMPONENTASSEMBLY_H
#define COMPONENTASSEMBLY_H
#include <unordered_map>
#include "GameObject.h"

namespace NCL {
    namespace CSC8508 {

            class AddComponentEvent{ //; : Event {
            public:
                GameObject& gameObject;
            };

            class ComponentAssemblyDefiner // : public EventListener<AddComponentEvent>
            {
                enum ComponentMapId {Error, Bounds, Physics, NavMesh};
                std::unordered_map<size_t, ComponentMapId> componentMap;
                bool AddComponent(size_t t, GameObject& object);
            };
    }
};
#endif //COMPONENTASSEMBLY_H
