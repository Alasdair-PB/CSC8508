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
#include <functional>
using std::vector;

namespace NCL::CSC8508 {

    class IComponent;
    class INetworkComponent;
    class INetworkDeltaComponent;

    class ComponentManager final {
    public:

        /// <summary>
        /// Returns the Component buffer of type T
        /// </summary>
        /// <typeparam name="T">The type required for the returned IComponent Buffer</typeparam>
        /// <returns>A pointer to the IComponent buffer</returns>
        template <typename T>
            requires std::is_base_of_v<IComponent, T>
        static T* GetComponentsBuffer() {
            return reinterpret_cast<T*>(componentBuffer<T>);
        }

        // <summary>
        /// Returns a Component Iterator created of type T
        /// </summary>
        /// <typeparam name="T">The type required for the returned IComponent Iterator</typeparam>
        /// <returns>A pair including the Component Buffer and count of type T</returns>
        template <typename T>
            requires std::is_base_of_v<IComponent, T>
        static std::pair<T*, size_t> GetComponentsIterator() {
            return { componentBuffer<T>, componentCount<T> };
        }

        /// <summary>
        /// Operates on IComponent contents as Iterator
        /// </summary>
        /// <typeparam name="T">The type required for iteration of function func</typeparam>
        /// <param name="func">the function that operates on each component in the iterator of type T</param>
        template <typename T>
            requires std::is_base_of_v<IComponent, T>
        static void OperateOnContents(std::function<void(T*)> func) {
            auto [buffer, count] = GetComponentsIterator<T>();
            for (size_t i = 0; i < count; ++i)
                func(&buffer[i]);
        }

        /// <summary>
        /// Executes func on all IComponents of types T.
        /// </summary>
        /// <typeparam name="T">The type required for iteration of function func</typeparam>
        /// <param name="func">the function that operates on each component in the iterator of type T</param>
        template <typename T>
            requires std::is_base_of_v<IComponent, T>
        static void OperateOnBufferContents(std::function<void(T*)> func) {
            T* buffer = GetComponentsBuffer<T>();
            size_t count = componentCount<T>;
            for (size_t i = 0; i < count; ++i)
                func(&buffer[i]);
        }

        /// <summary>
        /// Executes func on all IComponents of types Types that derived from type T as type T.
        /// </summary>
        /// <typeparam name="T">The type required for iteration of function func</typeparam>
        /// <typeparam name="...Types">The types iterated as type T</typeparam>
        /// <param name="func">the function that operates on each component in the buffer of each type provided in Types</param>
        template <typename T, typename... Types>
        static void OperateOnBufferContentsAs(std::function<void(T*)> func) {
            (([&] {
                T* buffer = dynamic_cast<T*>(GetComponentsBuffer<Types>());
                if (buffer) {
                    size_t count = componentCount<Types>;
                    for (size_t i = 0; i < count; ++i)
                        func(&buffer[i]);
                }
            }()), ...); 
        }

        template <typename T> requires std::is_base_of_v<IComponent, T>
        using Action = std::function<void(std::function<void(T*)> func)>;
        using Operation = std::function<void()>;

        /// <summary>
        /// Executes func on all IComponents derived from IComponent TParameters
        /// </summary>
        /// <param name="func"> the function that operates on each component in the buffer of type IComponent.</param>
        static void OperateOnAllIComponentBufferOperators(std::function<void(IComponent*)> func) {
            for (Action<IComponent>* myAction : IComponentBufferOperators)
                (*myAction)(func);
        }

        /// <summary>
        /// Executes func on all IComponents derived from the INetworkComponent interface TParameters
        /// </summary>
        /// <param name="func"> the function that operates on each component in the buffer of type IComponent.</param>
        static void OperateOnAllINetworkComponentBufferOperators(std::function<void(IComponent*)> func) {
            for (Action<IComponent>* myAction : INetworkComponentBufferOperators) 
                (*myAction)(func);
        }

        /// <summary>
        /// Executes func on all IComponents derived from the INetworkDeltaComponent interface Parameters:
        /// </summary>
        /// <param name="func"> the function that operates on each component in the buffer of type IComponent.</param>
        static void OperateOnAllINetworkDeltaComponentBufferOperators(std::function<void(IComponent*)> func) {
            for (Action<IComponent>* myAction : INetworkDeltaComponentBufferOperators)
                (*myAction)(func);
        }

        /// <summary>
        /// Adds a component of type T to the ComponentManager Buffers of type T
        /// </summary>
        /// <typeparam name="T">The type of the IComponent</typeparam>
        /// <typeparam name="...Args">The type of arguments used to declare this IComponent</typeparam>
        /// <param name="...args">The arguments used to declare this IComponent</param>
        /// <returns>A pointer to the created IComponent</returns>
        template <typename T, typename... Args> requires std::is_base_of_v<IComponent, T>
        static T* AddComponent(Args&&... args) {
            if (componentCount<T> >= MAX_COMPONENTS<T>) {
                std::cerr << "The Component pool is filled:: Increase the max size or reduce component count" << std::endl;
                return nullptr;
            }
            T* component = new (GetComponentsBuffer<T>() + componentCount<T>) T(std::forward<Args>(args)...);
            componentCount<T>++;
            AddOperatorBuffer<T, IComponent>(IComponentBufferOperators, component);
            AddOperatorBuffer<T, INetworkComponent>(INetworkComponentBufferOperators, component);
            AddOperatorBuffer<T, INetworkDeltaComponent>(INetworkDeltaComponentBufferOperators, component);
            //AddDeallocator<T>(component);
            AddDeallocatorBuffer<T>();
            allComponents[typeid(T)].push_back(component);

            return component;
        }

        /// <summary>
        /// Cleans memory allocations created by the ComponentManager
        /// </summary>
        static void CleanUp()
        {
            for (Operation* myAction : deallocationOperations) {
                (*myAction)();
                delete myAction;
            }
            allComponents.clear();
        }

    private:
        ComponentManager() = default;

        template <typename T> requires std::is_base_of_v<IComponent, T>
        static constexpr size_t MAX_COMPONENTS = 1000;

        template <typename T> requires std::is_base_of_v<IComponent, T>
        inline static size_t componentCount;

        template <typename T> requires std::is_base_of_v<IComponent, T>
        inline static std::byte componentBuffer[MAX_COMPONENTS<T> *sizeof(T)];

        inline static std::unordered_map<std::type_index, std::vector<IComponent*>> allComponents;
        inline static std::vector<Action<IComponent>*> INetworkComponentBufferOperators;
        inline static std::vector<Action<IComponent>*> IComponentBufferOperators;
        inline static std::vector<Action<IComponent>*> INetworkDeltaComponentBufferOperators;
        inline static std::vector<Operation*> deallocationOperations;

        template <typename T> requires std::is_base_of_v<IComponent, T>
        static void InvokeDeconstructor(T* component)
        {
            if (component) {
                std::cout << component->GetName() << std::endl;
                component->~T();
            }
        }

        template <typename T> requires std::is_base_of_v<IComponent, T>
        static void FreeComponentsBuffer()
        {
            T* buffer = GetComponentsBuffer<T>();
            size_t count = componentCount<T>;
            for (size_t i = 0; i < count; i++)
                (buffer + i)->~T();
            componentCount<T> = 0;
        }

        template <typename T> requires std::is_base_of_v<IComponent, T>
        static void AddDeallocatorBuffer()
        {
            if (allComponents.find(typeid(T)) != allComponents.end())
                return;

            deallocationOperations.push_back(
                new Operation(
                    []() {
                        FreeComponentsBuffer<T>();
                    }
                ));
        }

        template <typename T> requires std::is_base_of_v<IComponent, T>
        static void AddDeallocator(T* component)
        {
            deallocationOperations.push_back(
                new Operation(
                    [component](){
                       InvokeDeconstructor<T>(component);
                    }
             ));
        }

        /// <summary>
        /// Adds a component to a buffer operator as type T
        /// </summary>
        /// <typeparam name="T">The type of IComponent to add as BufferOperator</typeparam>
        /// <typeparam name="T2">The type of the BufferOperator that Type T will be added as</typeparam>
        /// <param name="BufferOperators">The BufferOperation object of type T2</param>
        /// <param name="component">The IComponent to be added to Buffer Operations</param>
        template <typename T, typename T2> requires std::is_base_of_v<IComponent, T>
        static void AddOperatorBuffer(std::vector<Action<IComponent>*>& BufferOperators, T* component)
        {
            if (allComponents.find(typeid(T)) != allComponents.end())
                return;

            if (component->IsDerived(typeid(T2))) {
                BufferOperators.push_back(
                    new Action<IComponent>(
                        [](std::function<void(IComponent*)> func) {
                            OperateOnBufferContents<T>(
                                [&func](T* derived) { func(static_cast<IComponent*>(derived)); }
                            );
                        }
                    ));
            }
        }
    };
}

#endif // COMPONENTMANAGER_H
