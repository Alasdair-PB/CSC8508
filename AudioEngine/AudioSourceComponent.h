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

using namespace NCL;
using namespace NCL::Maths;
using namespace NCL::CSC8508;

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

	}

	/**
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
	*/
	void cycleSounds();


	/**
	* Play random sound from collection using specified delay
	*/
	void randomSounds(int delay);

	/**
	* Stop Playback
	*/
	void StopSound() {
		fChannel ? fChannel->stop() : 0;
	};

	/**
	* Update position vectors of source for use by FMOD  
	*/
	void Update(float deltaTime) override {

		Vector3 pos = transform->GetPosition();
		fPosition = VecToFMOD(pos);

		/*PhysicsComponent* physComp = GetGameObject().TryGetComponent<PhysicsComponent>();

		if (physComp) {
			Vector3 vel = physComp->GetPhysicsObject()->GetLinearVelocity();
			fVelocity = VecToFMOD(vel);
		}
		else if(debug) {
			std::cout << "No Physics Component found for AudioSourceComponentComponent!" << std::endl;
		}*/



		fChannel->set3DAttributes(&fPosition, &fVelocity);

		fSystem->update();
 
		if (debug) {
			Debug::Print("Source Pos: " + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z), Vector2(5, 5));
		}
	}

private:

	/**
	* Destructor for Audio Source
	*/
	~AudioSourceComponent() {
		fChannel ? fChannel->stop() : 0;
	}

	FMOD::Channel* fChannel;
	ChannelGroupType defaultChannelGroup;

	std::string soundDir;
	std::map<std::string, FMOD::Sound*> fSoundCol;

	float fVolume;
};

#endif // AUDIOSOURCECOMPONENT_H