#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <fstream>
#include <vector>
#include <cstring>
#include <typeindex>
#include <type_traits>
#include <iostream>
using std::vector;

namespace NCL::CSC8508 {

    class SaveManager final {
    public:

        struct GameData {
            std::type_index typeID;
            uint32_t dataSize;
            std::vector<char> data;
            GameData() : typeID(typeid(void)), dataSize(0) {}
        };

        template <typename T, typename = void> struct is_container : std::false_type {};

        template <typename T> struct is_container<T, std::void_t<
            typename T::value_type, decltype(std::declval<T>().size()), decltype(std::declval<T>().data())>> : std::true_type {};

        template <typename T>
        static void SerializeContainer(const T& container, std::vector<uint8_t>& data, uint8_t*& dataPtr, uint32_t containerSize) {
            std::memcpy(dataPtr, container.data(), containerSize * sizeof(typename T::value_type));
            dataPtr += containerSize * sizeof(typename T::value_type);
        }

        static void SerializeContainer(const std::vector<size_t>& container, std::vector<char>& data, uint8_t*& dataPtr, uint32_t containerSize) {
            for (const auto& value : container) {
                std::memcpy(dataPtr, &value, sizeof(value));
                dataPtr += sizeof(value);
            }
        }

        static void SerializeContainer(const std::vector<std::string>& container, std::vector<char>& data, uint8_t*& dataPtr, uint32_t containerSize) {
            for (const auto& str : container) {
                uint32_t strLength = str.size();
                std::memcpy(dataPtr, &strLength, sizeof(strLength));
                dataPtr += sizeof(strLength);
                std::memcpy(dataPtr, str.data(), strLength);
                dataPtr += strLength;
            }
        }

        template <typename T, typename... Members>
        static GameData CreateSaveDataAsset(const T& value, Members T::*... members) {
            GameData item;
            item.typeID = std::type_index(typeid(T));
            item.dataSize = 0;

            ([&] {
                const auto& container = value.*members;

                if constexpr (std::is_trivially_copyable_v<typename std::decay_t<decltype(container)>>) 
                    item.dataSize += sizeof(container);
                else {
                    item.dataSize += sizeof(uint32_t);
                    if constexpr (std::is_same_v<typename std::decay_t<decltype(container)>::value_type, std::string>) {
                        for (const auto& str : container) {
                            item.dataSize += sizeof(uint32_t);
                            item.dataSize += str.size();
                        }
                    }
                    else
                        item.dataSize += container.size() * sizeof(typename std::decay_t<decltype(container)>::value_type);
                }
                }(), ...);

            item.data.resize(item.dataSize);
            uint8_t* dataPtr = reinterpret_cast<uint8_t*>(item.data.data());

            ([&] {
                const auto& container = value.*members;

                if constexpr (std::is_trivially_copyable_v<typename std::decay_t<decltype(container)>>) {
                    std::memcpy(dataPtr, &container, sizeof(container));
                    dataPtr += sizeof(container);
                }
                else {
                    uint32_t containerSize = container.size();
                    std::memcpy(dataPtr, &containerSize, sizeof(containerSize));
                    dataPtr += sizeof(containerSize);

                    SerializeContainer(container, item.data, dataPtr, containerSize);
                }
                }(), ...);

            return item;
        }

        template <typename T>
        static GameData CreateSaveDataAsset(const T& value) {
            GameData item;
            item.typeID = std::type_index(typeid(T));
            if constexpr (is_container<T>::value) {
                item.dataSize = sizeof(uint32_t) + value.size() * sizeof(typename T::value_type);
                item.data.resize(item.dataSize);
                uint32_t containerSize = value.size();
                std::memcpy(item.data.data(), &containerSize, sizeof(containerSize));
                std::memcpy(item.data.data() + sizeof(containerSize), value.data(), containerSize * sizeof(typename T::value_type));
            }
            else {
                item.dataSize = sizeof(value);
                item.data.resize(item.dataSize);
                std::memcpy(item.data.data(), &value, item.dataSize);
            }
            return item;
        }

        static size_t SaveGameData(const std::string& assetPath, GameData gameData, size_t start) {
            std::ofstream file(assetPath, std::ios::binary | std::ios::app);

            file.seekp(start, std::ios::beg);

            uint32_t magic = 0x47444D54;
            uint16_t version = 1;

            file.write(reinterpret_cast<char*>(&magic), sizeof(magic));
            file.write(reinterpret_cast<char*>(&version), sizeof(version));
            file.write(reinterpret_cast<const char*>(&gameData.typeID), sizeof(gameData.typeID));
            file.write(reinterpret_cast<const char*>(&gameData.dataSize), sizeof(gameData.dataSize));
            file.write(gameData.data.data(), gameData.dataSize);

            uint32_t checksum = magic ^ version;
            file.write(reinterpret_cast<char*>(&checksum), sizeof(checksum));

            uint32_t endMarker = 0x454E4453;
            file.write(reinterpret_cast<char*>(&endMarker), sizeof(endMarker));

            size_t end = file.tellp();
            file.close();
            return end;
        }

        static bool LoadGameData(const std::string& assetPath, GameData& gameData, size_t start) {
            std::ifstream file(assetPath, std::ios::binary);
            if (!file) {
                std::cerr << "Error: Could not open file " << assetPath << std::endl;
                return false;
            }
            file.seekg(start, std::ios::beg);

            uint32_t magic;
            uint16_t version;
            file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
            file.read(reinterpret_cast<char*>(&version), sizeof(version));

            if (magic != 0x47444D54) {
                std::cerr << "Error: Invalid file format!" << std::endl;
                return false;
            }

            file.read(reinterpret_cast<char*>(&gameData.typeID), sizeof(gameData.typeID));
            file.read(reinterpret_cast<char*>(&gameData.dataSize), sizeof(gameData.dataSize));

            uint32_t checksum;
            gameData.data.resize(gameData.dataSize);
            file.read(gameData.data.data(), gameData.dataSize);
            file.read(reinterpret_cast<char*>(&checksum), sizeof(checksum));

            uint32_t endMarker;
            file.read(reinterpret_cast<char*>(&endMarker), sizeof(endMarker));
            if (endMarker != 0x454E4453) {
                std::cerr << "Error: Missing end marker. File may be corrupted!" << std::endl;
                return false;
            }

            if (checksum != (magic ^ version))
                std::cerr << "Warning: Checksum mismatch. File may be corrupted!" << std::endl;

            file.close();
            return true;
        }

        template <typename T>
        static T LoadMember(GameData loadedData) {
            T container;
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memcpy(&container, loadedData.data.data(), sizeof(T));
                return container;
            }
            else if constexpr (is_container<T>::value) {
                uint32_t size;
                std::memcpy(&size, loadedData.data.data(), sizeof(size));
                container.resize(size);
                std::memcpy(container.data(), loadedData.data.data() + sizeof(size), size * sizeof(typename T::value_type));
                return container;
            }
            else {
                static_assert(sizeof(T) == -1, "Unsupported type for deserialization");
                return container;
            }
        }

        template <typename T, typename... Members>
        static T LoadMyData(const std::string& assetPath, const size_t allocationStart, Members T::*... members) {
            GameData loadedData;
            if (!LoadGameData(assetPath, loadedData, allocationStart)) throw std::runtime_error("Failed to load game data");
            if (loadedData.typeID != std::type_index(typeid(T))) throw std::runtime_error("Type mismatch in game data");

            T loadedStruct;

            if constexpr (sizeof...(members) == 0) 
                return LoadMember<T>(loadedData);
            else {
                size_t offset = 0;
                ([&] {
                    auto& container = loadedStruct.*members;

                    if constexpr (std::is_trivially_copyable_v<typename std::decay_t<decltype(container)>>) {
                        std::memcpy(&container, loadedData.data.data() + offset, sizeof(container));
                        offset += sizeof(container);
                        return container;
                    }
                    else {                    
                        uint32_t containerSize;
                        std::memcpy(&containerSize, loadedData.data.data() + offset, sizeof(containerSize));
                        offset += sizeof(containerSize);
                        container.resize(containerSize);

                        if constexpr (std::is_same_v<typename std::decay_t<decltype(container)>::value_type, std::string>) {
                            for (auto& str : container) {
                                uint32_t strLength;
                                std::memcpy(&strLength, loadedData.data.data() + offset, sizeof(strLength));
                                offset += sizeof(strLength);

                                str.resize(strLength);
                                std::memcpy(str.data(), loadedData.data.data() + offset, strLength);
                                offset += strLength;
                            }
                        }
                        else {
                            std::memcpy(container.data(), loadedData.data.data() + offset, containerSize * sizeof(typename std::decay_t<decltype(container)>::value_type));
                            offset += containerSize * sizeof(typename std::decay_t<decltype(container)>::value_type);
                        }
                    }
                    }(), ...);
            }
            return loadedStruct;
        }

    private:
        SaveManager() = default;
    };
}

#endif // SAVEMANAGER_H
