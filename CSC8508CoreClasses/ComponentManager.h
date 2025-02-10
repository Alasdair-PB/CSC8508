//
// Contributors: Alasdair
//

#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H
#include <typeindex>
#include <iostream>
#include <fstream>


#include "IComponent.h"

namespace NCL::CSC8508 
{
    class IComponent;
    class ComponentManager {
    public:
        static ComponentManager& Instance() {
            static ComponentManager instance;
            return instance;
        }

        ComponentManager(const ComponentManager&) = delete;
        ComponentManager& operator=(const ComponentManager&) = delete;

        template <typename T, typename... Args>
            requires std::is_base_of_v<IComponent, T>
        T* AddComponent(Args&&... args) {
            T* component = new T(std::forward<Args>(args)...);
            componentsByType[typeid(T)].push_back(component);
            return component;
        }

        template <typename T>
            requires std::is_base_of_v<IComponent, T>
        T* TryGetComponent() {
            auto& components = componentsByType[typeid(T)];
            for (IComponent* component : components) {
                if (std::strcmp(component->GetType(), typeid(T).name()) == 0) {
                    return static_cast<T*>(component);
                }
            }
            return nullptr;
        }

        void Clear() {
            for (auto& [type, components] : componentsByType) {
                for (IComponent* component : components) {
                    delete component;
                }
                components.clear();
            }
        }

    private:
        ComponentManager() = default;
        std::unordered_map<std::type_index, std::vector<IComponent*>> componentsByType;
    };
}

#endif //COMPONENTMANAGER_H
