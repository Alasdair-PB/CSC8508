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

		encodedPacketQueue = &audioEngine->GetEncodedPacketQueue();

		decoder = OpenDecoder();

		InitPersistentSound();

	}

	/**
	* Update position vectors of source for use by FMOD
	*/
	void Update(float deltaTime) override {

		Vector3 pos = transform->GetPosition();
		fPosition = VecToFMOD(pos);

		/*
		PhysicsComponent* physComp = GetGameObject().TryGetComponent<PhysicsComponent>();

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

	#pragma region FMOD Sound Loading

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
	void CycleSounds();


	/**
	* Play random sound from collection using specified delay
	*/
	void RandomSounds(int delay);

	/**
	* Stop Playback
	*/
	void StopSound() {
		fChannel ? fChannel->stop() : 0;
	};

	#pragma endregion

	#pragma region Decoding VOIP Pipeline

	std::vector<short> DecodeOpusFrame(std::vector<unsigned char>& encodedPacket) {
		std::vector<short> pcmFrame(960);
		int decodedSamples = opus_decode(decoder, encodedPacket.data(), encodedPacket.size(), pcmFrame.data(), pcmFrame.size(), 0);

		std::cout << "[DEBUG] UpdatePersistentPlayback() Decoded " << decodedSamples << " samples." << std::endl;

		if (decodedSamples < 0) {
			std::cerr << "[ERROR] UpdatePersistentPlayback() Opus Decoding Failed: " << decodedSamples << std::endl;
			return std::vector<short>();
		}

		if (decodedSamples < static_cast<int>(pcmFrame.size())) {
			pcmFrame.resize(decodedSamples);
		}

		return pcmFrame;

	}

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
		currentWritePos = 0;

	}

	void DecodePersistentPlayback(std::vector<unsigned char>& encodedPacket) {

		if (!persistentSound) return;

		std::vector<short> pcmFrame = DecodeOpusFrame(encodedPacket);

		if (pcmFrame.empty()) {
			std::cerr << "[ERROR] UpdatePersistentPlayback() Decoded PCM Frame is empty." << std::endl;
			return;
		}

		unsigned int bytesToWrite = pcmFrame.size() * sizeof(short);
		unsigned int bufferSizeBytes = persistentBufferSize * sizeof(short);

		if (currentWritePos * sizeof(short) + bytesToWrite > bufferSizeBytes) {

			unsigned int bytesUntilEnd = bufferSizeBytes - currentWritePos * sizeof(short);
			void* ptr1 = nullptr;
			void* ptr2 = nullptr;
			unsigned int len1 = 0, len2 = 0;

			FMOD_RESULT result = persistentSound->lock(currentWritePos * sizeof(short), bytesUntilEnd, &ptr1, &ptr2, &len1, &len2);

			if (result != FMOD_OK) {
				std::cerr << "[ERROR] UpdatePersistentPlayback() FMOD Error: " << FMOD_ErrorString(result) << std::endl;
				return;
			}

			// Copy the first chunk of data
			memcpy(ptr1, pcmFrame.data(), bytesUntilEnd);
			persistentSound->unlock(ptr1, ptr2, len1, len2);

			// Copy the remaining data to the start of the buffer
			unsigned int remainingBytes = bytesToWrite - bytesUntilEnd;
			result = persistentSound->lock(0, remainingBytes, &ptr1, &ptr2, &len1, &len2);

			if (result != FMOD_OK) {
				std::cerr << "[ERROR] UpdatePersistentPlayback() FMOD Error: " << FMOD_ErrorString(result) << std::endl;
				return;
			}

			memcpy(ptr1, ((char*)pcmFrame.data()) + bytesUntilEnd, remainingBytes);
			persistentSound->unlock(ptr1, ptr2, len1, len2);

			currentWritePos = remainingBytes / sizeof(short);

		}
		else { // normal write, no wrap-around
			void* ptr1 = nullptr;
			void* ptr2 = nullptr;
			unsigned int len1 = 0, len2 = 0;
			FMOD_RESULT result = persistentSound->lock(currentWritePos * sizeof(short), bytesToWrite, &ptr1, &ptr2, &len1, &len2);
			if (result != FMOD_OK) {
				std::cerr << "[ERROR] UpdatePersistentPlayback() FMOD Error: " << FMOD_ErrorString(result) << std::endl;
				return;
			}
			memcpy(ptr1, pcmFrame.data(), bytesToWrite);
			persistentSound->unlock(ptr1, ptr2, len1, len2);
			currentWritePos = (currentWritePos + pcmFrame.size()) % persistentBufferSize;
		}

	}

	void UpdateAudioDecode() {
		if (encodedPacketQueue != nullptr && !encodedPacketQueue->empty()) {
			std::vector<unsigned char> packet = encodedPacketQueue->front();
			encodedPacketQueue->pop_front();  // Process oldest frame first
			DecodePersistentPlayback(packet);
		}
	}

	#pragma endregion

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


	std::deque<std::vector<unsigned char>>* encodedPacketQueue;

	OpusDecoder* decoder;

	FMOD::Sound* persistentSound = nullptr;
	FMOD::Channel* persistentChannel = nullptr;
	unsigned int persistentBufferSize = sampleRate / 2; // 0.5 second
	unsigned int currentWritePos = 0; // in samples

};

#endif // AUDIOSOURCECOMPONENT_H