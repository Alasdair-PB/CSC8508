#ifndef ISERIALIZABLE_H
#define ISERIALIZABLE_H

#include "SaveManager.h"
#include <string>
#include <tuple>
#include <type_traits>

namespace NCL::CSC8508 {

	class ISerializable {
	public:
		virtual size_t Save(std::string assetPath, size_t* allocationStart) { return 0; }
		virtual void Load(std::string assetPath, size_t allocationStart) {}

		
	};
	
	struct ISerializedData {
		public:
			#define SERIALIZED_FIELD(Derived, field) std::make_tuple(#field, &Derived::field)
			#define ERROR_GET_SERIALIZED_FIELDS(T) \
			"Error: GetSerializedFields not implemented in class " #T

			template <typename T, typename = void>
			struct HasGetSerializedFields : std::false_type {};

			template <typename T>
			struct HasGetSerializedFields<T, std::void_t<decltype(T::GetSerializedFields())>> : std::true_type {};

			template<typename T>
			static T LoadISerializable(std::string assetPath, size_t allocationStart) {
				T serializedData;
				if constexpr (HasGetSerializedFields<T>::value) {
					auto fields = T::GetSerializedFields();
					serializedData = std::apply(
						[&](auto&&... args) { return SaveManager::LoadMyData<T>(assetPath, allocationStart, std::get<1>(args)...); },
						fields
					);
				}
				else
					static_assert(HasGetSerializedFields<T>::value, ERROR_GET_SERIALIZED_FIELDS(T));
				return serializedData;
			}

			template<typename T>
			static SaveManager::GameData CreateGameData(T saveInfo) {
				SaveManager::GameData saveData;
				if constexpr (HasGetSerializedFields<T>::value) {
					auto fields = T::GetSerializedFields();
					saveData = std::apply(
						[&](auto&&... args) { return SaveManager::CreateSaveDataAsset(saveInfo, std::get<1>(args)...); },
						fields
					);
				}
				else
					static_assert(HasGetSerializedFields<T>::value, ERROR_GET_SERIALIZED_FIELDS(T));
				return saveData;
			}
	};
}

#endif // ISERIALIZABLE_H
