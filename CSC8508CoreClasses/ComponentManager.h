//
// Contributors: Alasdair
//

#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H
#include <typeindex>
#include <iostream>
#include <fstream>
using std::vector;
#include <vector>

namespace NCL::CSC8508 
{

    class IComponent;

    class ComponentManager final {
    public:
        template <typename T>
            requires std::is_base_of_v<IComponent, T>
        static std::vector<T*> GetAllComponents() {
            std::vector<T*> result;
            auto it = allComponents.find(typeid(T));
            if (it != allComponents.end()) {
                for (auto* comp : it->second) {
                    if (auto* derived = dynamic_cast<T*>(comp)) 
                        result.push_back(derived);
                }
            }
            return result;
        }

        template <typename T, typename... Args>
            requires std::is_base_of_v<IComponent, T>
        static T* AddComponent(Args&&... args) {
            T* component = new T(std::forward<Args>(args)...);
            allComponents[typeid(T)].push_back(component);            
            return component;
        }

        static void Clear() {
            for (auto& [type, componentsList] : allComponents) {
                for (auto* comp : componentsList) 
                    delete comp;
            }
            allComponents.clear();
        }

    private:
        ComponentManager() = default;

        template <typename T> requires std::is_base_of_v<IComponent,T>static vector<T*> components;
        inline static std::unordered_map<std::type_index, std::vector<IComponent*>> allComponents;
    };

    template <typename T>
        requires std::is_base_of_v<IComponent, T>
    vector<T*> ComponentManager::components = vector<T*>();
}

#endif //COMPONENTMANAGER_H
