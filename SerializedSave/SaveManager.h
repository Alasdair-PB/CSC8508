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

        /// <summary>
        /// A hash function that returns a unique Id designed for 64 bit architecture
        /// </summary>
        /// <param name="key">The data to be hashed</param>
        /// <param name="len">The length of the input data as bytes</param>
        /// <param name="seed">The seed used for this hash output</param>
        /// <returns></returns>
        static size_t MurmurHash3_64(const void* key, int len, size_t seed = 0) {
            const uint8_t* data = static_cast<const uint8_t*>(key);
            size_t h = seed ^ (len * static_cast<size_t>(0xc6a4a7935bd1e995ULL));

            for (int i = 0; i < len; i++) {
                h ^= static_cast<size_t>(data[i]);
                h *= static_cast<size_t>(0xc6a4a7935bd1e995ULL);
                h ^= h >> 47;
            }
            return h;
        }


        template <typename T>
        static size_t UniqueTypeHash() {
            std::string typeName = typeid(T).name();
            return MurmurHash3_64(typeName.c_str(), typeName.size());
        }

        struct GameData {
            std::size_t typeHash;
            uint32_t dataSize;
            std::vector<char> data;
            GameData() : typeHash(UniqueTypeHash<void>()), dataSize(0) {}
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

        static void SerializeContainer(const std::vector<std::pair<size_t, std::size_t>>& container, std::vector<char>& data, uint8_t*& dataPtr, uint32_t containerSize) {
            for (const auto& value : container) {
                std::memcpy(dataPtr, &value.first, sizeof(value.first));
                dataPtr += sizeof(value.first);
                std::memcpy(dataPtr, &value.second, sizeof(value.second));
                dataPtr += sizeof(value.second);
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

        /// <summary>
        /// Creates gameData that allows data Types to saved and reinterpreted by the SaveManager
        /// </summary>
        /// <typeparam name="T">The type to be saved</typeparam>
        /// <typeparam name="...Members">The members in the Type if Type T is a container</typeparam>
        /// <param name="value">The value to be saved as a GameData Asset</param>
        /// <param name="...members">The member values of type T if Type T is a container. Can be called without members present. </param>
        /// <returns></returns>
        template <typename T, typename... Members>
        static GameData CreateSaveDataAsset(const T& value, Members T::*... members) {
            GameData item;
            item.typeHash = UniqueTypeHash<T>();
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
            item.typeHash = UniqueTypeHash<T>();

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

        const static uint32_t fileExtension = 0x70666162; // "pfab"
        const static uint32_t endConst = 0x454E4453; // "ends"
        const static uint32_t initConst = 0x696E6974; // "init"

        /// <summary>
        /// Initialises the table of contents to find the starting object in a file
        /// </summary>
        /// <param name="file">The file data is being saved to</param>
        static void InitializeTOC(std::ofstream& file) {
            size_t initObject;
            file.seekp(0, std::ios::beg);
            file.write(reinterpret_cast<char*>(&initObject), sizeof(initObject));
        }

        static void SetTOCOffsets(const std::string& assetPath, size_t start) {
            std::fstream file(assetPath, std::ios::in | std::ios::out | std::ios::binary);
            if (!file) {
                std::cerr << "Error: Could not open file " << assetPath << std::endl;
                return;
            }
            file.seekp(0, std::ios::beg);
            file.write(reinterpret_cast<char*>(&start), sizeof(start));
            file.close();
        }

        static size_t ReadTOC(std::ifstream& file, size_t start) {
            file.seekg(0, std::ios::beg);
            size_t initObject;
            file.read(reinterpret_cast<char*>(&initObject), sizeof(initObject));
            return initObject;
        }
        enum FileType {Prefab, Scene};

        /// <summary>
        /// Saves gameData as a .pfab file
        /// </summary>
        /// <param name="assetPath">The file to save this data to</param>
        /// <param name="gameData">The gameData to save</param>
        /// <param name="start">The position in memory to save this data</param>
        /// <param name="fileType">The fileType format. Scenes use a different file extension and have a larger maxObject count
        /// <returns>The next memory position free after this data</returns>
        
        static size_t SaveGameData(const std::string& assetPath, GameData gameData, size_t* memoryLocation = nullptr, bool isRootObject = true, FileType fileType = FileType::Prefab) {
            size_t start = (memoryLocation == nullptr) ? 0 : *memoryLocation;
            std::ios::openmode mode = std::ios::binary | ((start == 0) ? std::ios::trunc : std::ios::app);
            std::ofstream file(assetPath, mode);

            if (!file) {
                std::cerr << "Error: Could not open file " << assetPath << std::endl;
                return 0;
            }

            if (start == 0) {
                InitializeTOC(file);
                start = file.tellp();

                if (memoryLocation != nullptr)
                    *memoryLocation = start;
            }

            file.seekp(start, std::ios::beg);

            uint32_t magic = fileExtension;
            uint16_t version = 1;

            file.write(reinterpret_cast<char*>(&magic), sizeof(magic));
            file.write(reinterpret_cast<char*>(&version), sizeof(version));
            file.write(reinterpret_cast<const char*>(&gameData.typeHash), sizeof(gameData.typeHash));
            file.write(reinterpret_cast<const char*>(&gameData.dataSize), sizeof(gameData.dataSize));
            file.write(gameData.data.data(), gameData.dataSize);

            uint32_t checksum = magic ^ version;
            uint32_t endMarker = endConst;
            file.write(reinterpret_cast<char*>(&checksum), sizeof(checksum));
            file.write(reinterpret_cast<char*>(&endMarker), sizeof(endMarker));

            size_t end = file.tellp();

            if (isRootObject)
                SetTOCOffsets(assetPath, start);    
            file.close();
            return end;
        }

        /// <summary>
        /// Loads a .pfab file type and interprets the file into a GameData object
        /// </summary>
        /// <param name="assetPath">The file to load for this .fab data</param>
        /// <param name="gameData"The GameData that recieves the loaded data as it's GameData derived type</param>
        /// <param name="start">The memory location of Gamedata in the .pfab file</param>
        /// <returns>If the load was successful</returns>
        static bool LoadGameData(const std::string& assetPath, GameData& gameData, size_t start = 0, FileType fileType = FileType::Prefab) {
            std::ifstream file(assetPath, std::ios::binary);
            if (!file) {
                std::cerr << "Error: Could not open file " << assetPath << std::endl;
                return false;
            }
            if (start == 0) 
                file.seekg(ReadTOC(file, start), std::ios::beg);
            else 
                file.seekg(start, std::ios::beg);

            uint32_t magic;
            uint16_t version;
            file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
            file.read(reinterpret_cast<char*>(&version), sizeof(version));

            if (magic != fileExtension) {
                std::cerr << "Error: Invalid file format!" << std::endl;
                return false;
            }

            file.read(reinterpret_cast<char*>(&gameData.typeHash), sizeof(gameData.typeHash));
            file.read(reinterpret_cast<char*>(&gameData.dataSize), sizeof(gameData.dataSize));

            uint32_t checksum;
            gameData.data.resize(gameData.dataSize);
            file.read(gameData.data.data(), gameData.dataSize);
            file.read(reinterpret_cast<char*>(&checksum), sizeof(checksum));

            uint32_t endMarker;
            file.read(reinterpret_cast<char*>(&endMarker), sizeof(endMarker));
            if (endMarker != endConst) {
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
        static T LoadMyData(const std::string& assetPath, const size_t allocationStart = 0, Members T::*... members) {
            GameData loadedData;
            if (!LoadGameData(assetPath, loadedData, allocationStart)) throw std::runtime_error("Failed to load game data");
            if (loadedData.typeHash != UniqueTypeHash<T>()) throw std::runtime_error("Type mismatch in game data");
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
                        else if constexpr (std::is_same_v<typename std::decay_t<decltype(container)>::value_type, std::pair<size_t, std::type_info>>) {
                            for (auto& p : container) {
                                std::memcpy(&p.first, loadedData.data.data() + offset, sizeof(p.first));
                                offset += sizeof(p.first);

                                uint32_t typeNameLength;
                                std::memcpy(&typeNameLength, loadedData.data.data() + offset, sizeof(typeNameLength));
                                offset += sizeof(typeNameLength);

                                std::string typeName(typeNameLength, '\0');
                                std::memcpy(typeName.data(), loadedData.data.data() + offset, typeNameLength);
                                offset += typeNameLength;
                                p.second = typeid(typeName);
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
