#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <new>    
#include <utility>
#include <type_traits>
#include <iostream>
#include <vector>
#include <memory>
using std::vector;

namespace NCL::CSC8508 {

    class IComponent;
    class INetworkComponent;
    class PhysicsComponent;

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

        template <typename T>
            requires std::is_base_of_v<IComponent, T>
        static void OperateOnBufferContents(std::function<void(T*)> func) {
            T* buffer = GetComponentsBuffer<T>();
            size_t count = componentCount<T>;
            for (size_t i = 0; i < count; ++i)
                func(&buffer[i]);
        }

        template <typename T, typename... Types>
        void OperateOnBufferContentsAs(std::function<void(T*)> func) {
            (([&] {
                T* buffer = dynamic_cast<T*>(GetComponentsBuffer<Types>());
                if (buffer) {
                    size_t count = componentCount<Types>;
                    for (size_t i = 0; i < count; ++i) 
                        func(&buffer[i]);
                }
            }()), ...); 
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
                std::cerr << "The Component pool is filled:: Increase the max size or reduce component count" << std::endl;
                return nullptr;
            }
            T* component = new (GetComponentsBuffer<T>() + componentCount<T>) T(std::forward<Args>(args)...);
            componentCount<T>++;
            allComponents[typeid(T)].push_back(component);

            if (component->IsDerived(typeid(PhysicsComponent)))
                ((PhysicsComponent*)component)->GetGameObject().GetTransform();

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
        static alignas(T) std::byte componentBuffer[MAX_COMPONENTS<T> *sizeof(T)];

        inline static std::unordered_map<std::type_index, std::vector<IComponent*>> allComponents;
        inline static vector<INetworkComponent*> allNetworkComponents;
    };

    template <typename T>
        requires std::is_base_of_v<IComponent, T>
    size_t ComponentManager::componentCount<T> = 0;

    template <typename T>
        requires std::is_base_of_v<IComponent, T>
    alignas(T) std::byte ComponentManager::componentBuffer<T>[MAX_COMPONENTS<T> *sizeof(T)] = {};
}

#endif // COMPONENTMANAGER_H
