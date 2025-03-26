//
// Contributors: Max Bolton
//

#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#ifndef ASSETROOTLOCATION
#define ASSETROOTLOCATION "../Assets/"
#endif

#include <fmod.hpp>
#include <map>
#ifdef USE_PS5
#include <kernel.h>
#endif
#include <deque>
#include <vector>
#include <thread>
#include <string>
#include <filesystem>
#include <iostream>

class AudioListenerComponent;
class AudioSourceComponent;

/**
* Type of Channel Group
* SFX
* MUSIC
* VOICE
* MASTER
*/
enum class ChannelGroupType {
	SFX,
	MUSIC,
	CHAT,
	MASTER
};

enum class EntitySoundGroup {
	PLAYER,
	ENEMY,
	ENVIRONMENT,
	WEAPON,
	ITEM,
};


/**
* Audio Engine is a singleton class that manages the FMOD Audio System.
* A reference to the Audio Engine is needed for the main game loop to update the audio system,
* and for each AudioObject to get the FMOD System.
*/
class AudioEngine {
public:
    /**
	* Singleton instance of Audio Engine
    */
    static AudioEngine& Instance();

	~AudioEngine();

	/**
	* Update Audio Engine
    * To be called in main game loop
    */
    void Update();

	/**
	* Get FMOD System
	* Reference needed for each AudioObject
	* @return FMOD System instance
	*/
	FMOD::System* GetSystem() {
		return audioSystem;
	}

	/**
	* Handle Destruction of Audio Engine
    */
    void Shutdown();

	#pragma region Input/Output Device Management
	/**
	* Update input device list
	* Removes output loopback devices
	*/
	void UpdateInputList();

	/**
	* Get List of input devices
	* @return std::map<int, std::string>
	*/
	std::map<int, std::string> GetInputDeviceList() {
		UpdateInputList();
		return inputDeviceList;
	}

	/**
	* Print input device list
	* [Used for debugging]
	*/
	void PrintInputList();

	/**
	* Update output Device List
	*/
	void UpdateOutputList();

	/**
	* Get List of output devices
	* @return std::map<int, std::string>
	*/

	std::map<int, std::string> GetOutputDeviceList() {
		UpdateOutputList();
		return outputDeviceList;
	}

	/**
	* Print output device list
	* [Used for debugging]
	*/
	void PrintOutputList();
	
	/**
	* Get current input device index
	* @return int index of input device
	*/
	int GetInputDeviceIndex() {
		return inputDeviceIndex;
	}

	/**
	* Set input device index
	* Ensures FMOD uses the intended input device
	*/
	void SetInputDeviceIndex(int index) {
		if (IsRecording()) {
			StopRecording();
		}
		inputDeviceIndex = index;
	}

	/**
	* Get current output device index
	* @return int index of output device
	*/
	int GetOutputDeviceIndex() {
		return outputDeviceIndex;
	}

	/**
	* Set output device index
	* Ensures FMOD uses the intended output device
	*/
	void SetOutputDeviceIndex(int index) {
		outputDeviceIndex = index;
		audioSystem->setDriver(outputDeviceIndex);
	}

	/**
	* Checks if current input device is recording
	*/
	bool IsRecording();

	/**
	* Stop Microphone recording of selected input device
	*/
	void StopRecording() {
		audioSystem->recordStop(inputDeviceIndex);
	}

	#pragma endregion

	/**
	* Get pointer to a channel group
	* @return FMOD Channel Group
	* @param type of channel group
	*/
	FMOD::ChannelGroup* GetChannelGroup(ChannelGroupType type) {
		return channelGroups[type];
	}


	/**
	* Set volume of a channel group
	* @param type of channel group
	* @param volume to set
	*/
	void SetChannelVolume(ChannelGroupType type, float volume) {
		GetChannelGroup(type)->setVolume(volume);
	}

	/**
	* Get current volume of a channel group
	* @return float volume of channel group
	* @param type of channel group
	*/
	float GetChannelVolume(ChannelGroupType type) {
		float volume;
		GetChannelGroup(type)->getVolume(&volume);
		return volume;
	}


	/**
	* Loads all sounds from specified directory into a map
	* @return true if successful
	* @param directory path (inside Assets/Audio/)
	* @param entity sound group
	*/
	bool LoadSoundDirectory(const char* directory, EntitySoundGroup group);

	/**
	* Initialise all sound assets into sound groups
	*/
	void LoadSoundAssets() {
		LoadSoundDirectory("player", EntitySoundGroup::PLAYER);
		LoadSoundDirectory("enemy", EntitySoundGroup::ENEMY);
		LoadSoundDirectory("environment", EntitySoundGroup::ENVIRONMENT);
		LoadSoundDirectory("weapon", EntitySoundGroup::WEAPON);
		LoadSoundDirectory("item", EntitySoundGroup::ITEM);
	}

	/**
	* Prints all sounds in a specific entity sound group
	* - For debugging purposes
	*/
	void printSoundsInGroup(EntitySoundGroup group) {
		std::map<std::string, FMOD::Sound*> sounds = soundGroups[group];
		for (auto& sound : sounds) {
			std::cout << sound.first << std::endl;
		}
	}

	/**
	* Get fmod sound map for specific entity sound group
	* @return map of fmod sounds
	* @param entity sound group
	*/
	std::map<std::string, FMOD::Sound*>* GetSoundGroup(EntitySoundGroup group) {
		return &soundGroups[group];
	}

	float GetMinDistance() {
		return minDistance;
	}

	float GetMaxDistance() {
		return maxDistance;
	}


private:
    AudioEngine();

    FMOD::System* audioSystem;

	std::map<int, std::string> inputDeviceList;
	int inputDeviceIndex;

	std::map<int, std::string> outputDeviceList;
	int outputDeviceIndex;

	std::map<ChannelGroupType, FMOD::ChannelGroup*> channelGroups;

	FMOD::ChannelGroup* masterGroup;

	FMOD::ChannelGroup* sfxGroup;
	FMOD::ChannelGroup* musicGroup;
	FMOD::ChannelGroup* voiceGroup;

	FMOD::ChannelGroup* CreateChannelGroups(ChannelGroupType type, const char* name);
	
	#ifdef USE_PS5	
	SceKernelModule	libfmodHandle;
	SceKernelModule	libfmodLHandle;

	#endif // USE_PS5

	std::string soundDir;

	std::map<std::string, FMOD::Sound*> playerSounds;
	std::map<std::string, FMOD::Sound*> enemySounds;
	std::map<std::string, FMOD::Sound*> environmentSounds;
	std::map<std::string, FMOD::Sound*> weaponSounds;
	std::map<std::string, FMOD::Sound*> itemSounds;

	std::map<EntitySoundGroup, std::map<std::string, FMOD::Sound*>> soundGroups;

	float minDistance = 0.5f;
	float maxDistance = 500.0f;
};

#endif
