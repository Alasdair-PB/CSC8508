//
// Contributors: Max Bolton
//

#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H


#include <fmod.hpp>

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

    /**
	* Instantiate static singleton instance of Audio Engine
	* Call this to get the instance of the Audio Engine
    */
    void Init();

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

	std::deque<std::vector<unsigned char>>& GetEncodedPacketQueue() {
		return encodedPacketQueue;
	}


private:
    AudioEngine();
    ~AudioEngine();

    FMOD::System* audioSystem;


	std::map<ChannelGroupType, FMOD::ChannelGroup*> channelGroups;

	FMOD::ChannelGroup* masterGroup;

	FMOD::ChannelGroup* sfxGroup;
	FMOD::ChannelGroup* musicGroup;
	FMOD::ChannelGroup* voiceGroup;

	FMOD::ChannelGroup* CreateChannelGroups(ChannelGroupType type, const char* name);

	std::deque<std::vector<unsigned char>> encodedPacketQueue;
	
};

#endif
