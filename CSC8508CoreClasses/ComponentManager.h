#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include <typeindex>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <memory>
#include <type_traits>

using std::vector;

namespace NCL::CSC8508 {

    class IComponent;
    class INetworkComponent;

    class ComponentManager final {
    public:

        template <typename T>
            requires std::is_base_of_v<IComponent, T>
        static T* GetComponentsBuffer() {
            return reinterpret_cast<T*>(componentBuffer<T>);
        }

        template <typename T>
            requires std::is_base_of_v<IComponent, T>
        static std::pair<T*, size_t> GetComponentsIterator() {
            return { componentBuffer<T>, componentCount<T> };
        }

        template <typename T>
            requires std::is_base_of_v<IComponent, T>
        static void OperateOnContents(std::function<void(T*)> func) {
            auto [buffer, count] = GetComponentsIterator<T>();
            for (size_t i = 0; i < count; ++i) {
                func(&buffer[i]);
            }
        }

        // Does not currently work: works for parent types e.g<InputComponent>
        // but will not for NetworkInputComponent
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

        template <typename T>
            requires std::is_base_of_v<IComponent, T>
        static void OperateOnBufferContents(std::function<void(T*)> func) {
            T* buffer = GetComponentsBuffer<T>();
            size_t count = componentCount<T>;
            for (size_t i = 0; i < count; ++i) 
                func(&buffer[i]); 
        }



        template <typename T>
            requires std::is_base_of_v<IComponent, T>
        static void OperateOnBufferContentsDynamicType(std::function<void(T*)> func)
        {
            for (auto& entry : allComponents) 
            {
                for (auto* component : entry.second) {
                    if (!component->IsDerived(typeid(T)))
                        break;
                    func(component);
                }
            }
        }

        static void OperateOnINetworkComponents(std::function<void(INetworkComponent*)> func)
        {
            for (INetworkComponent* component : allNetworkComponents)
                func(component);
        }

        template <typename T, typename... Args>
            requires std::is_base_of_v<IComponent, T>
        static T* AddComponent(Args&&... args) {
            if (componentCount<T> >= MAX_COMPONENTS<T>) {
                std::cerr << "The Component pool is filled:: Increase the max size or reduce component Count" << std::endl;
                return nullptr;
            }
            T* component = new (componentBuffer<T> +componentCount<T> *sizeof(T)) T(std::forward<Args>(args)...);
            componentCount<T>++;
            allComponents[typeid(T)].push_back(component);

            if (component->IsDerived(typeid(INetworkComponent)))
                allNetworkComponents.push_back((INetworkComponent*)component);

            return component;
        }


        static void Clear() 
        {
            for (auto& [type, componentsList] : allComponents) {
                for (auto* comp : componentsList) 
                    delete comp;
            }
            allComponents.clear();
        }

    private:
        ComponentManager() = default;

        template <typename T> requires std::is_base_of_v<IComponent, T>
        static constexpr size_t MAX_COMPONENTS = 1000;
        template <typename T> requires std::is_base_of_v<IComponent, T>
        static size_t componentCount;

        template <typename T> requires std::is_base_of_v<IComponent, T>
        static alignas(T) char componentBuffer[MAX_COMPONENTS<T> *sizeof(T)];
        inline static std::unordered_map<std::type_index, std::vector<IComponent*>> allComponents;
        inline static vector<INetworkComponent*> allNetworkComponents;

    };

    template <typename T>
        requires std::is_base_of_v<IComponent, T>
    size_t ComponentManager::componentCount<T> = 0;

    template <typename T>
        requires std::is_base_of_v<IComponent, T>
    char ComponentManager::componentBuffer<T>[MAX_COMPONENTS<T> *sizeof(T)] = {};

}

#endif // COMPONENTMANAGER_H
