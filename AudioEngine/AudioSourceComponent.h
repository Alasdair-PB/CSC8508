//
// Contributors: Max Bolton
//

#ifndef AUDIOSOURCECOMPONENT_H
#define AUDIOSOURCECOMPONENT_H

#ifndef ASSETROOTLOCATION
#define ASSETROOTLOCATION "../Assets/"
#endif

#include "AudioObject.h"
#include <string>
#include <filesystem>
#include <iostream>
#include "Maths.h"
#include "Debug.h"
#include <Transform.h>
#include "PhysicsComponent.h"
#include "PhysicsObject.h"

/**
* Audio Source class for audio engine
*/
class AudioSourceComponent : public AudioObject
{
public:


	/**
	* Constructor for Audio Source
	* Sets default file path for audio files
	*/
	AudioSourceComponent(GameObject& gameObject, ChannelGroupType type = ChannelGroupType::MASTER) : AudioObject(gameObject) {
		soundDir = std::string(ASSETROOTLOCATION) + "Audio/";

		fChannel = nullptr;
		defaultChannelGroup = type;
		fVolume = 1.0f;


		// Initialise Persistent Sound
		InitPersistentSound();
	}

	/**
	* Destructor for Audio Source
	*/
	~AudioSourceComponent() {
		fChannel ? fChannel->stop() : 0;
		delete persistentSound;
	}

	void OnAwake() override {}


	/**
	* Update position vectors of source for use by FMOD
	*/
	void Update(float deltaTime) override {

		Vector3 pos = transform->GetPosition();
		fPosition = VecToFMOD(pos);

		
		PhysicsComponent* physComp = GetGameObject().TryGetComponent<PhysicsComponent>();

		if (physComp) {
			Vector3 vel = physComp->GetPhysicsObject()->GetLinearVelocity();
			fVelocity = VecToFMOD(vel);
		}
		else if(debug) {
			std::cout << "No Physics Component found for AudioSourceComponentComponent!" << std::endl;
		}
		
		fChannel->set3DAttributes(&fPosition, &fVelocity);

		fSystem->update();

		if (debug) {
			std::cout << "Source Pos: " << std::to_string(pos.x) << ", " + std::to_string(pos.y) << ", " << std::to_string(pos.z) << std::endl;
		}

	}

	#pragma region FMOD Sound Loading

	/**
	* [No longer used as audio engine stores all sounds]
	* Load sound from file
	* Different modes include FMOD_3D, FMOD_2D, FMOD_DEFAULT, FMOD_LOOP_NORMAL
	* FMOD_3D is defualt for spacial audio
	* Creates sound using Audio File Path
	* Sets volume and 3D min max distance
	* @return sound loaded status (true if successful)
	* @param filename name of audio file
	* @param volume of sound
	* @param mode of sound
	*/
	bool LoadSound(const char* filename, float vol = 1.0f, FMOD_MODE mode = FMOD_3D) {
		FMOD::Sound* sound;

		std::string fullPath = soundDir + filename;

		std::string name = filename;
		name = name.substr(0, name.find_last_of("."));

		if (fSystem->createSound(fullPath.c_str(), mode, 0, &sound) != FMOD_OK) {
			std::cerr << "Error loading sound (" << filename << "): " << std::endl;
			return false;
		}
		
		fVolume = vol;
		sound->set3DMinMaxDistance(0.5f, 500.0f);
		fSoundCol[name] = sound;
		return true;
	}

	void setSoundCollection(std::map<std::string, FMOD::Sound*> sounds) {
		fSoundCol = sounds;
	}

	/**
	* Play created sound
	* @return sound played status (true if successful)
	* @param name of sound
	*/
	bool PlaySound(const char* soundName) {

		FMOD::Sound* fSound = fSoundCol[soundName];


		if (!fSound) {
			std::cerr << "Error: fSound is nullptr, cannot play sound!" << std::endl;
			return false;
		}

		FMOD_RESULT result = fSystem->playSound(fSound, audioEngine->GetChannelGroup(defaultChannelGroup), false, &fChannel);
		if (result != FMOD_OK) {
			std::cerr << "Error playing sound: " << result << std::endl;
			return false;
		}

		if (fChannel) {
			fChannel->setPaused(false);
			fChannel->setVolume(fVolume);
			return true;
		}
	}

	/**
	* Play sound object
	* @return sound played status (true if successful)
	* @param sound object to play
	*/
	bool PlaySoundObj(FMOD::Sound* sound) {
		if (!sound) {
			std::cerr << "Error: fSound is nullptr, cannot play sound!" << std::endl;
			return false;
		}
		FMOD_RESULT result = fSystem->playSound(sound, audioEngine->GetChannelGroup(ChannelGroupType::CHAT), false, &fChannel);
		if (result != FMOD_OK) {
			std::cerr << "Error playing sound: " << result << std::endl;
			return false;
		}
		if (fChannel) {
			fChannel->setPaused(false);
			fChannel->setVolume(fVolume);
			return true;
		}
	}

	/**
	* Cycle through sounds in collection
	* @param delay between sounds
	* @return true if successfully cycled through all sounds
	*/
	bool CycleSounds(float delay) {
		for (auto& sound : fSoundCol) {
			PlaySound(sound.first.c_str());
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(delay * 1000))); //TODO - Find a way to 
		}
		return true;
	}


	/**
	* Play random sound from collection
	* @return sound played status (true if successful)
	*/
	bool RandomSound() {
		int randomIndex = rand() % fSoundCol.size();
		auto it = fSoundCol.begin();
		std::advance(it, randomIndex);
		return PlaySound(it->first.c_str());
	}

	/**
	* Stop Playback
	*/
	void StopSound() {
		fChannel ? fChannel->stop() : 0;
	};

	#pragma endregion

	void InitPersistentSound() {
		FMOD_CREATESOUNDEXINFO exinfo = {};
		exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
		exinfo.numchannels = channels;
		exinfo.format = FMOD_SOUND_FORMAT_PCM16;
		exinfo.defaultfrequency = sampleRate;
		exinfo.length = persistentBufferSize * sizeof(short); // total buffer size in bytes

		FMOD_RESULT result = fSystem->createSound(nullptr, FMOD_OPENUSER | FMOD_LOOP_NORMAL | FMOD_3D, &exinfo, &persistentSound);

		if (result != FMOD_OK) {
			std::cerr << "[ERROR] InitPersistentSound() FMOD Error: " << FMOD_ErrorString(result) << std::endl;
			persistentSound = nullptr;
			return;
		}

		result = fSystem->playSound(persistentSound, audioEngine->GetChannelGroup(ChannelGroupType::MASTER), true, &persistentChannel); // NOTE-STARTS PAUSED

		if (result != FMOD_OK) {
			std::cerr << "[ERROR] InitPersistentSound() FMOD Error: " << FMOD_ErrorString(result) << std::endl;
			return;
		}

		persistentChannel->setPaused(false);

	}

	std::pair<FMOD::Sound*, FMOD::Channel*> GetPersistentPair() {
		return std::make_pair(persistentSound, persistentChannel);
	}


private:


	FMOD::Channel* fChannel;
	ChannelGroupType defaultChannelGroup;

	std::string soundDir;
	std::map<std::string, FMOD::Sound*> fSoundCol;

	float fVolume;

	// VOIP Playback
	FMOD::Channel* persistentChannel = nullptr;
	FMOD::Sound* persistentSound = nullptr;
	unsigned int persistentBufferSize = sampleRate / 2; // 0.5 second

};

#endif // AUDIOSOURCECOMPONENT_H