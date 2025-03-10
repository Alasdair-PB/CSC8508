//
// Contributors: Max Bolton
//

#pragma once

#include "AudioObject.h"
#include <string>
#include "Maths.h"
#include "Debug.h"
#include <Transform.h>
#include "Camera.h"
#include <opus/opus.h>
#include <fmod.hpp>
#include <fmod_errors.h>
#include <queue>
#include <deque>
#include <algorithm>
#include <vector>
#include <cstring>
#include <iostream>
#include <mutex>

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

static FMOD_RESULT F_CALLBACK PCMReadCallback(FMOD_SOUND* sound, void* data, unsigned int datalen);

/**
* Listener class for audio engine
*/
class AudioListenerComponent : public AudioObject
{
public:


	AudioListenerComponent(GameObject& gameObject, PerspectiveCamera& camera);


	/**
	*
	*/
	void OnAwake() override;

	/**
	* Update position vectors of listener for use by FMOD
	* @param deltaTime
	*/
	void Update(float deltaTime) override;

	void SetCamOrientation();

	void SetPlayerOrientation();


	#pragma region FMOD Recording

	/**
	* Init microphone sound object
	*/
	void InitMicSound();

	/**
	* Start Microphone recording of selected input device
	*/
	void RecordMic() {
		FMOD_RESULT result = fSystem->recordStart(inputDeviceIndex, micInput, true);
	}

	/**
	* Stop Microphone recording of selected input device
	*/
	void StopRecording() {
		fSystem->recordStop(inputDeviceIndex);
	}

	/**
	* Checks if current input device is recording
	*/
	bool IsRecording();

	/**
	* Toggle recording of microphone input
	*/
	void ToggleRecording();

	/**
	* Get recorded microphone buffer
	* @return FMOD::Sound* (recorded buffer)
	*/
	FMOD::Sound* getMicInput() {
		return micInput;
	}

	#pragma endregion

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



	#pragma endregion

	#pragma region Encoding/Decoding Pipeline


	void PushSamples(const std::vector<short>& samples);


	/**
	* Pop samples from the ring buffer and copy them to the destination buffer.
	* @return int (number of bytes copied)
	* @param void* (destination buffer)
	* @param unsigned int (number of bytes requested)
	*/
	int PopSamples(void* dest, unsigned int bytesRequested);

	/**
	* Encodes a PCM sample buffer to an Opus frame.
	* Each frame contains 960 samples (20ms at 48kHz).
	* @return std::vector<unsigned char> (encoded Opus frame)
	* @param std::vector<short>& (PCM data to encode)
	*/
	std::vector<unsigned char> EncodeOpusFrame(std::vector<short>& pcmData);


	/**
	* Streams PCM data from the microphone input to the Opus encoder.
	*/
	void StreamEncodeMic();


	/**
	* Initialise the persistent sound object for continuous playback.
	*/
	void InitPersistentSound();

	/**
	* Update the persistent sound buffer with the latest PCM data.
	* @param std::vector<unsigned char>& (encoded Opus packet)
	*/
	void UpdatePersistentPlayback(std::vector<unsigned char>& encodedPacket);

	#pragma endregion

	void UpdateAudioEncode(){
		StreamEncodeMic();
	}

	void UpdateAudioDecode() {
		if (!encodedPacketQueue.empty()) {
			std::vector<unsigned char> packet = encodedPacketQueue.front();
			encodedPacketQueue.pop_front();  // Process oldest frame first
			UpdatePersistentPlayback(packet);
		}
	}


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
	* Set output device index
	* Ensures FMOD uses the intended output device
	*/
	void SetOutputDeviceIndex(int index) {
		outputDeviceIndex = index;
	}

	#pragma endregion


private:

	// Listner id - always 0
	int fIndex = 0;

	std::map<int, std::string> inputDeviceList;
	int inputDeviceIndex;

	std::map<int, std::string> outputDeviceList;
	int outputDeviceIndex;

	FMOD::Sound* micInput;

	int channels = 1;
	int sampleRate = 48000;

	////Encoding/Decoding////
	OpusEncoder* encoder;

	std::deque<std::vector<unsigned char>> encodedPacketQueue;



	OpusDecoder* decoder;

	FMOD::Sound* persistentSound = nullptr;
	FMOD::Channel* persistentChannel = nullptr;
	unsigned int persistentBufferSize = sampleRate / 2; // 0.5 second
	unsigned int currentWritePos = 0; // in samples


	std::deque<short> ringBuffer;
	std::mutex ringBufferMutex;
	unsigned int ringBufferMaxSize = sampleRate / 2;
	/////////////////////////


	FMOD_VECTOR fForward;
	FMOD_VECTOR fUp;

	PerspectiveCamera* camera;


	~AudioListenerComponent();

};