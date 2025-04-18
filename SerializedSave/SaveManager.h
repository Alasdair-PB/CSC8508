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

        struct GameData {
            uint32_t dataSize;
            std::vector<char> data;
            GameData() : dataSize(0) {}
        };

        template <typename T, typename = void> struct is_container : std::false_type {};

        template <typename T> struct is_container<T, std::void_t<
            typename T::value_type, decltype(std::declval<T>().size()), decltype(std::declval<T>().data())>> : std::true_type {};

        template <typename T, template <typename, typename> class Template>
        struct is_specialization : std::false_type {};

        template <typename T1, typename T2, template <typename, typename> class Template>
        struct is_specialization<Template<T1, T2>, Template> : std::true_type {};

        const static uint32_t fileExtension = 0x70666162; // "pfab"
        const static uint32_t worldExtension = 0x776C7264; // "wrld"

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

        static uint32_t GetFileExtension(FileType fileType) {
            switch (fileType) {
            case Prefab: {
                return fileExtension;
                break;
            }
            case Scene: {
                return worldExtension;
                break;
            }
            default: { break; }
            }
            return fileExtension;
        }

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

            
            uint32_t magic = GetFileExtension(fileType);
            uint16_t version = 1;

            file.write(reinterpret_cast<char*>(&magic), sizeof(magic));
            file.write(reinterpret_cast<char*>(&version), sizeof(version));
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

            if (magic != fileExtension && magic != worldExtension) {
                std::cerr << "Error: Invalid file format!" << std::endl;
                return false;
            }
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

        /// <summary>
        /// Loads data as Type T from GameData saved in a assetfile
        /// </summary>
        /// <typeparam name="T">The type GameData will be loaded as</typeparam>
        /// <typeparam name="...Members">The type of members within Type T members will be loaded as, can be left empty</typeparam>
        /// <param name="assetPath">The path to the save asset</param>
        /// <param name="allocationStart">The offet within the GameData file to begin loading from</param>
        /// <param name="...members">The members within Type T members will be loaded as</param>
        /// <returns>The loaded data as type T</returns>
        template <typename T, typename... Members>
        static T LoadMyData(const std::string& assetPath, const size_t allocationStart = 0, Members T::*... members) {
            GameData loadedData;
            if (!LoadGameData(assetPath, loadedData, allocationStart)) std::cerr << "Failed to load game data" << std::endl;
            T loadedStruct;
            size_t offset = 0;

            if constexpr (sizeof...(members) == 0)
                return LoadMember<T>(loadedData, offset);
            else {
                ([&] {
                    auto& container = loadedStruct.*members;
                    container = LoadMember<typename std::decay_t<decltype(container)>>(loadedData, offset);
                }(), ...);
            }
            return loadedStruct;
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
            item.dataSize = 0;

            if constexpr (sizeof...(members) == 0)
                return ResizeAndSaveMember(value, item);
            else {
                ([&] {
                    const auto& container = value.*members; 
                    CalculateSize(container, item);
                }(), ...);

                item.data.resize(item.dataSize);
                uint8_t* dataPtr = reinterpret_cast<uint8_t*>(item.data.data());

                ([&] {
                   const auto& container = value.*members;
                   SaveMember(container, item, dataPtr);
                   }(), ...);
                return item;
            }
        }

        /// <summary>
        /// Resize the char array in GameData to save data into and then save the data
        /// </summary>
        /// <typeparam name="T">The type of the saved value</typeparam>
        /// <param name="value">The Type value to be saved</param>
        /// <param name="item">The saved data to be filled</param>
        /// <returns>The saved data struct</returns>
        template <typename T>
        static GameData ResizeAndSaveMember(const T& value, GameData& item) {
            CalculateSize<T>(value, item);
            item.data.resize(item.dataSize);
            uint8_t* dataPtr = reinterpret_cast<uint8_t*>(item.data.data());
            SaveMember<T>(value, item, dataPtr);
            return item;
        }

        /// <summary>
        /// Resize the char array in GameData to save data into
        /// </summary>
        /// <typeparam name="T">The type of the saved value</typeparam>
        /// <param name="container">The Type value to be saved</param>
        /// <param name="item">The saved data to be filled</param>
        template <typename T>
        static void CalculateSize(const T& container, GameData& item) {
            if constexpr (std::is_same_v<T, std::string>) {
                item.dataSize += sizeof(uint32_t);
                item.dataSize += container.size();
            }
            else if constexpr (is_specialization<T, std::pair>::value) {
                using FirstType = typename T::first_type;
                using SecondType = typename T::second_type;
                CalculateSize(container.first, item);
                CalculateSize(container.second, item);
            }
            else if constexpr (is_container<T>::value) {
                item.dataSize += sizeof(uint32_t);
                using ValueType = typename T::value_type;

                for (const auto& val : container)
                    CalculateSize(val, item);
            }
            else if constexpr (std::is_trivially_copyable_v<T>) 
                item.dataSize += sizeof(T);
            else 
                static_assert(sizeof(T) == -1, "Unsupported type for serialization");
        }

        /// <summary>
        /// Saves type T into GameData by checking T against container types
        /// </summary>
        /// <typeparam name="T">The type of the saved value</typeparam>
        /// <param name="container">The Type value to be saved</param>
        /// <param name="item">The item to save this value into</param>
        /// <param name="dataPtr">The offset in memory from the last saved value</param>
        template <typename T>
        static void SaveMember(const T& container, GameData& item, uint8_t*& dataPtr) {
            if constexpr (is_specialization<T, std::pair>::value) {
                using FirstType = typename T::first_type;
                using SecondType = typename T::second_type;

                SaveMember(container.first, item, dataPtr);
                SaveMember(container.second, item, dataPtr);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                uint32_t strLength = container.size();
                std::memcpy(dataPtr, &strLength, sizeof(strLength));
                dataPtr += sizeof(strLength);

                std::memcpy(dataPtr, container.data(), strLength);
                dataPtr += strLength;
            } 
            else if constexpr (is_container<T>::value) {
                uint32_t containerSize = container.size();
                std::memcpy(dataPtr, &containerSize, sizeof(containerSize));
                dataPtr += sizeof(containerSize);

                for (const auto& p : container)
                    SaveMember(p, item, dataPtr);
            }
            else if constexpr (std::is_trivially_copyable_v<T>) {
                std::memcpy(dataPtr, &container, sizeof(container));
                dataPtr += sizeof(container);
            }
            else {
                static_assert(sizeof(T) == -1, "Unsupported type for serialization");
            }
        }

        /// <summary>
        /// Copies GameData into Type T by checking T against container types
        /// </summary>
        /// <typeparam name="T">The type of the saved value to be read</typeparam>
        /// <param name="GameData">The data to be loaded</param>
        /// <param name="offset">The offset in memory from the last saved value</param>
        /// <returns>The value loaded from memory as type T</returns>
        template <typename T>
        static T LoadMember(GameData loadedData, size_t& offset) {
            T container;

            if constexpr (is_specialization<T, std::pair>::value) {
                using FirstType = typename T::first_type;
                using SecondType = typename T::second_type;

                FirstType first = LoadMember<FirstType>(loadedData, offset);
                SecondType second = LoadMember<SecondType>(loadedData, offset);
                container = { first, second };
            } 
            else if constexpr (std::is_same_v<T, std::string>) {
                uint32_t strLength;
                std::memcpy(&strLength, loadedData.data.data() + offset, sizeof(strLength));
                offset += sizeof(strLength);

                container.resize(strLength);
                std::memcpy(container.data(), loadedData.data.data() + offset, strLength);
                offset += strLength;
            }
            else if constexpr (is_container<T>::value) {
                uint32_t containerSize = container.size();
                std::memcpy(&containerSize, loadedData.data.data() + offset, sizeof(containerSize));
                offset += sizeof(containerSize);
                container.resize(containerSize);

                using ValueType = typename T::value_type;
                for (size_t i = 0; i < containerSize; ++i)
                    container[i] = LoadMember<ValueType>(loadedData, offset);
            }
            else if constexpr (std::is_trivially_copyable_v<T>) {
                std::memcpy(&container, loadedData.data.data() + offset, sizeof(container));
                offset += sizeof(container);
            }
            else {
                static_assert(sizeof(T) == -1, "Unsupported type for deserialization");
            }
            return container;
        }

    private:
        SaveManager() = default;
    };
}

#endif // SAVEMANAGER_H
