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

        template <typename T, typename = void>
        struct is_container : std::false_type {};

        template <typename T>
        struct is_container<T, std::void_t<typename T::value_type, decltype(std::declval<T>().size()), decltype(std::declval<T>().data())>> : std::true_type {};

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
            else if constexpr (std::is_class_v<T>) {
                // Rework paramters to declare members names
                item.dataSize = sizeof(uint32_t) + value.vec.size() * sizeof(typename T::vec::value_type);
                item.data.resize(item.dataSize);
                uint32_t containerSize = value.vec.size();
                std::memcpy(item.data.data(), &containerSize, sizeof(containerSize));
                std::memcpy(item.data.data() + sizeof(containerSize), value.vec.data(), containerSize * sizeof(typename T::vec::value_type));
            }
            else {
                item.dataSize = sizeof(value);
                item.data.resize(item.dataSize);
                std::memcpy(item.data.data(), &value, item.dataSize);
            }
            return item;
        }

        static void SaveGameData(const std::string& filename, GameData gameData) {
            std::ofstream file(filename, std::ios::binary);
            uint32_t magic = 0x47444D54;
            uint16_t version = 1;

            file.write(reinterpret_cast<char*>(&magic), sizeof(magic));
            file.write(reinterpret_cast<char*>(&version), sizeof(version));
            file.write(reinterpret_cast<const char*>(&gameData.typeID), sizeof(gameData.typeID));
            file.write(reinterpret_cast<const char*>(&gameData.dataSize), sizeof(gameData.dataSize));
            file.write(gameData.data.data(), gameData.dataSize);
 
            uint32_t checksum = magic ^ version;
            file.write(reinterpret_cast<char*>(&checksum), sizeof(checksum));
            file.close();
        }

        static bool LoadGameData(const std::string& filename, GameData& gameData) {
            std::ifstream file(filename, std::ios::binary);
            if (!file) {
                std::cerr << "Error: Could not open file " << filename << std::endl;
                return false;
            }

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

            if (checksum != (magic ^ version)) 
                std::cerr << "Warning: Checksum mismatch. File may be corrupted!" << std::endl;

            file.close();
            return true;
        }

        static bool LoadCustomFormat(const std::string& filename, std::vector<GameData>& entries) {
            std::ifstream file(filename, std::ios::binary);
            if (!file) {
                std::cerr << "Error: Could not open file " << filename << std::endl;
                return false;
            }
            uint32_t magic;
            uint16_t version;
            uint32_t count;
            file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
            file.read(reinterpret_cast<char*>(&version), sizeof(version));
            file.read(reinterpret_cast<char*>(&count), sizeof(count));

            if (magic != 0x47444D54) {
                std::cerr << "Error: Invalid file format!" << std::endl;
                return false;
            }

            entries.clear();
            for (uint32_t i = 0; i < count; i++) {
                GameData entry;
                file.read(reinterpret_cast<char*>(&entry.typeID), sizeof(entry.typeID));
                file.read(reinterpret_cast<char*>(&entry.dataSize), sizeof(entry.dataSize));
                entry.data.resize(entry.dataSize);
                file.read(entry.data.data(), entry.dataSize);

                entries.push_back(entry);
            }
            uint32_t checksum;
            file.read(reinterpret_cast<char*>(&checksum), sizeof(checksum));

            if (checksum != (magic ^ version ^ count)) 
                std::cerr << "Warning: Checksum mismatch. File may be corrupted!" << std::endl;
            file.close();
            return true;
        }

        template <typename T>
        static T LoadMyData(const std::string& filename) {
            GameData loadedData;
            if (!LoadGameData(filename, loadedData)) 
                throw std::runtime_error("Failed to load game data");
            if (loadedData.typeID != std::type_index(typeid(T))) 
                throw std::runtime_error("Type mismatch in game data");

            if constexpr (std::is_trivially_copyable_v<T>) {
                T loadedType;
                std::memcpy(&loadedType, loadedData.data.data(), sizeof(T));
                return loadedType;
            }
            else if constexpr (is_container<T>::value) {
                uint32_t size;
                std::memcpy(&size, loadedData.data.data(), sizeof(size));

                T container;
                container.resize(size);
                std::memcpy(container.data(), loadedData.data.data() + sizeof(size), size * sizeof(typename T::value_type));
                return container;
            }
            else if constexpr (std::is_class_v<T>) {
                T loadedStruct;
                size_t offset = 0;

                std::memcpy(&loadedStruct, loadedData.data.data() + offset, sizeof(T) - sizeof(typename T::vec));
                offset += sizeof(T) - sizeof(typename T::vec);
                uint32_t vectorSize;
                std::memcpy(&vectorSize, loadedData.data.data() + offset, sizeof(vectorSize));
                offset += sizeof(vectorSize);
                loadedStruct.vec.resize(vectorSize);
                std::memcpy(loadedStruct.vec.data(), loadedData.data.data() + offset, vectorSize * sizeof(typename T::vec::value_type));
                return loadedStruct;
            }
            else 
                static_assert(sizeof(T) == -1, "Unsupported type for deserialization");
        }

        template <typename T>
        struct HasPointerMember {
            template <typename U, typename = void>
            struct check_pointer_members : std::false_type {};
            template <typename U>
            struct check_pointer_members<U, std::enable_if_t<
                std::is_pointer<typename std::remove_reference<decltype(U::x)>::type>::value
                >> : std::true_type {};
            static constexpr bool value = check_pointer_members<T>::value;
        };

        template <typename T>
        static void LoadData() {
            std::vector<GameData> loadedData;
            if (LoadCustomFormat("game_data.gdmt", loadedData)) {
                for (const auto& entry : loadedData) {
                    if (entry.typeID == std::type_index(typeid(T))) {
                        T loadedType;
                        std::memcpy(&loadedType, entry.data.data(), sizeof(loadedType));
                        std::cout << "Loaded type value: " << loadedType << std::endl;
                    }
                }
            }
        }        
        
        template <typename... Types>
        static std::tuple<Types...> LoadAllData() {
            std::vector<GameData> loadedData;
            std::tuple<Types...> loadedTypes;
            if (LoadCustomFormat("game_data.gdmt", loadedData)) {
                int index = 0;
                (..., (std::get<Types>(loadedTypes) = [&]() {
                    for (const auto& entry : loadedData) {
                        if (entry.typeID == std::type_index(typeid(Types))) {
                            Types loadedType;
                            std::memcpy(&loadedType, entry.data.data(), sizeof(loadedType));
                            return loadedType;
                        }
                    }
                    return Types();
                    }()));
            }
            return loadedTypes;
        }

    private:
        SaveManager() = default;
    };
}

#endif // SAVEMANAGER_H
