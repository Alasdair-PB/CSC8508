//
// Contributors: Max Bolton
//

#pragma once

#include <fmod.hpp>
#include "AudioEngine.h"
#include "Maths.h"
#include "Debug.h"
#include <Transform.h>
#include "IComponent.h"
#include "GameObject.h"
#include <opus/opus.h>
#include <fmod_errors.h>


using namespace NCL;
using namespace NCL::Maths;
using namespace NCL::CSC8508;

struct PCMBufferLock {
	FMOD_RESULT result;
	unsigned int offset;
	unsigned int length;
	void* ptr1;
	void* ptr2;
	unsigned int len1;
	unsigned int len2;
};


/**
* Base class for all audio objects (listeners, sound sources)
* will eventually inherit from component class for component system
* 
*/
class AudioObject : public IComponent
{

public:
	
	void setDebug(bool debug) {
		this->debug = debug;
	}

	#pragma region Encoding/Decoding Tools


	/**
	* Lock the sound to access the PCM data.
	* @return PCMBufferLock (lock object containing sound data)
	* @param FMOD::Sound* (sound to lock)
	* @param unsigned int (start offset)
	* @param unsigned int (buffer size)
	*/
	PCMBufferLock LockSound(FMOD::Sound* sound, unsigned int startOffset, unsigned int buffersize) {
		PCMBufferLock lock = {};
		lock.result = sound->lock(startOffset, buffersize, &lock.ptr1, &lock.ptr2, &lock.len1, &lock.len2);
		if (lock.result == FMOD_OK) {
			lock.offset = startOffset;
			lock.length = lock.len1 + lock.len2;
		}
		else {
			std::cerr << "[ERROR] LockSound(): " << FMOD_ErrorString(lock.result) << std::endl;
		}
		return lock;
	}


	/**
	* Release the lock so FMOD can continue recording.
	* @return bool (true if successful)
	* @param FMOD::Sound* (sound to unlock)
	* @param PCMBufferLock* (lock to release)
	*/
	bool UnlockSound(FMOD::Sound* sound, PCMBufferLock* lock) {
		FMOD_RESULT result = sound->unlock(lock->ptr1, lock->ptr2, lock->len1, lock->len2);
		return result == FMOD_OK ? true : false;
	}

	/**
	* Configures an Opus encoder to convert RAW PCM data into Opus format.
	* @return OpusEncoder* (pointer to the encoder)
	*/
	OpusEncoder* OpenEncoder() {
		int error;

		OpusEncoder* encoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_VOIP, &error);
		if (error != OPUS_OK) {
			// std::cout << "[ERROR] OpenEncoder(): " + std::to_string(error) << std::endl;
			return nullptr;
		}

		return encoder;
	}

	/**
	* Closes a specified Opus encoder.
	* @param OpusEncoder* (pointer to the encoder)
	*/
	void CloseEncoder(OpusEncoder* encoder) {
		opus_encoder_destroy(encoder);
	}

	/**
	* Configures an Opus decoder to convert Opus data into PCM samples.
	* @return OpusDecoder* (pointer to the decoder)
	*/
	OpusDecoder* OpenDecoder() {
		int error;
		OpusDecoder* decoder = opus_decoder_create(sampleRate, channels, &error);
		if (error != OPUS_OK) {
			// std::cout << "[ERROR] OpenDecoder(): " + std::to_string(error) << std::endl;
			return nullptr;
		}
		return decoder;
	}

	/**
	* Closes a specified Opus decoder.
	* @param OpusDecoder* (pointer to the decoder)
	*/
	void CloseDecoder(OpusDecoder* decoder) {
		opus_decoder_destroy(decoder);
	}

	/**
	* Convert packet to fixed size array
	* @return unsigned char* (array of packet data)
	* @param std::vector<unsigned char>& (packet data)
	* NOTE: Remember to delete the array after use
	*/
	unsigned char* PacketToArray(std::vector<unsigned char>& packet) {
		unsigned char* array = new unsigned char[packet.size()];
		for (size_t i = 0; i < packet.size(); i++) {
			array[i] = packet[i];
		}
		return array;
	}



#pragma endregion


protected:
	AudioObject(GameObject& gameObject) : IComponent(gameObject) {

		audioEngine = &AudioEngine::Instance();
		fSystem = audioEngine->GetSystem();

		this->transform = &(gameObject.GetTransform());

		Vector3 zero = Vector3(0, 0, 0);
		fVelocity = VecToFMOD(zero);

	};

	Transform* transform;

	AudioEngine* audioEngine;
	FMOD::System* fSystem;

	FMOD_VECTOR fPosition;
	FMOD_VECTOR fVelocity;

	bool debug = false;

	static FMOD_VECTOR VecToFMOD(const Vector3& vec) {
		FMOD_VECTOR fVec;
		fVec.x = vec.x;
		fVec.y = vec.y;
		fVec.z = vec.z;
		return fVec;
	}


	int channels = 1;
	int sampleRate = 48000;

};






