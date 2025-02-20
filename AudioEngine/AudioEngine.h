//
// Contributors: Max Bolton
//

#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H


#include <fmod.hpp>


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
	FMOD::System* GetSystem() { return audioSystem; }

	/**
	* Handle Destruction of Audio Engine
    */
    void Shutdown();

	void setMasterVolume(float volume) {
		masterGroup->setVolume(volume);
	}

	FMOD::ChannelGroup* getMasterGroup() {
		return masterGroup;
	}

private:
    AudioEngine();
    ~AudioEngine();

    FMOD::System* audioSystem;

	FMOD::ChannelGroup* masterGroup;

};

#endif
