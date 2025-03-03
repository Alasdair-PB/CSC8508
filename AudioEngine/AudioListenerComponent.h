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

using namespace NCL;
using namespace NCL::Maths;
using namespace NCL::CSC8508;

struct PCMBufferLock {
	unsigned int offset;
	unsigned int length;
	void* ptr1;
	void* ptr2;
	unsigned int len1;
	unsigned int len2;
};


/**
* Listener class for audio engine
*/
class AudioListenerComponent : public AudioObject
{
public:


	AudioListenerComponent(GameObject& gameObject, PerspectiveCamera& camera);

	/**
	* Update position vectors of listener for use by FMOD
	* @param deltaTime
	*/
	void Update(float deltaTime) override;

	void SetCamOrientation();

	void SetPlayerOrientation();


	#pragma region FMOD Recording
	/**
	* Start Microphone recording of selected input device
	*/
	void RecordMic() {
		FMOD_RESULT result = fSystem->recordStart(inputDeviceIndex, micInput, true);
		std::cout << "Recording Result: " << result << std::endl;
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

	#pragma region Opus Tools

	PCMBufferLock LockSound(FMOD::Sound* sound, unsigned int offset, unsigned int bufferSize) {
		PCMBufferLock lock = {};
		unsigned int recordPosition = 0;
		fSystem->getRecordPosition(inputDeviceIndex, &recordPosition);

		if (recordPosition == offset) {
			std::cerr << "Cannot lock sound at current record position!" << std::endl;
			return lock;
		}

		FMOD_RESULT result = sound->lock(offset, bufferSize, &lock.ptr1, &lock.ptr2, &lock.len1, &lock.len2);
		
		if (result == FMOD_OK) {
			lock.offset = 0;
			lock.length = lock.len1 + lock.len2;
		}
		else {
			std::cerr << "Failed to lock PCM data, err: " << FMOD_ErrorString(result) << std::endl;
		}
		return lock;

	}

	bool UnlockSound(FMOD::Sound* sound, PCMBufferLock* lock) {
		FMOD_RESULT result = sound->unlock(lock->ptr1, lock->ptr2, lock->len1, lock->len2);
		return result == FMOD_OK ? true : false;
	}

	OpusEncoder* OpenEncoder() {
		int error;

		OpusEncoder* encoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_VOIP, &error);
		if (error != OPUS_OK) {
			std::cout << "Opus Encoder Error: " + std::to_string(error) << std::endl;
			return nullptr;
		}

		opus_encoder_ctl(encoder, OPUS_SET_BITRATE(64000)); // Adjust bitrate for clarity
		opus_encoder_ctl(encoder, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_FULLBAND));
		opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10)); // Max quality

		return encoder;
	}

	void CloseEncoder(OpusEncoder* encoder) {
		opus_encoder_destroy(encoder);
	}

	OpusDecoder* OpenDecoder() {
		int error;
		OpusDecoder* decoder = opus_decoder_create(sampleRate, channels, &error);
		if (error != OPUS_OK) {
			std::cout << "Opus Decoder Error: " + std::to_string(error) << std::endl;
			return nullptr;
		}
		return decoder;
	}

	void CloseDecoder(OpusDecoder* decoder) {
		opus_decoder_destroy(decoder);
	}
	#pragma endregion


	#pragma region Encoding/Decoding Pipeline
	/**
	* Extracts PCM samples from the locked buffer.
	*/
	std::vector<short> ExtractPCMData(PCMBufferLock* lock) {
		std::vector<short> pcmData;

		// First Chunk

		if (lock->ptr1 && lock->len1 > 0) {
			short* pcmShorts = static_cast<short*>(lock->ptr1);
			pcmData.assign(pcmShorts, pcmShorts + (lock->len1 / sizeof(short)));

			std::cout << "[PCM Extraction] 5 sample values: ";
			for (int i = 0; i < 5; i++) {
				std::cout << pcmData[i] << " ";
			}
			std::cout << std::endl;
		}
		else {
			std::cout << "PCM Extract Failure, ptr1 == NULL or len1 is 0" << std::endl;
		}

		// Second Chunk (if exists)
		if (lock->ptr2 && lock->len2 > 0) {
			short* pcmShorts = static_cast<short*>(lock->ptr2);
			pcmData.insert(pcmData.end(), pcmShorts, pcmShorts + (lock->len2 / sizeof(short)));
			std::cout << "PCM Extracted Second Chunk" << std::endl;
		}
		else if (lock->ptr2 == nullptr) {
			//std::cout << "PCM Extract Note, ptr2 is NULL (no wrap-around in PCM buffer)";
		}

		if (!pcmData.empty()) {
			auto minMax = std::minmax_element(pcmData.begin(), pcmData.end());
			std::cout << "[PCM DEBUG] Min sample: " << *minMax.first << " | Max Sample: " << *minMax.second << std::endl;
		}


		std::cout << "Total extracted PCM Samples: " << pcmData.size() << std::endl;
		return pcmData;

	}


	/**
	* Encodes a single frame of PCM data into Opus.
	*/
	std::vector<unsigned char> EncodeOpusFrame(std::vector<short>& pcmData) {
		OpusEncoder* encoder = OpenEncoder();

		std::vector<unsigned char> opusData;
		std::vector<unsigned char> tempBuffer(4096);
		int frameSize = 960;

		if (pcmData.size() < frameSize) {
			std::cerr << "[Opus Encoding] PCM Data is less than frame size! size: " << pcmData.size() << std::endl;
			return opusData;
		}

		for (size_t i = 0; i < pcmData.size(); i += frameSize) {
			
			//break if PCM buffer is exceeded
			if (i + frameSize > pcmData.size()) break;

			int encodedBytes = opus_encode(encoder, pcmData.data() + i, frameSize, tempBuffer.data(), tempBuffer.size());

			if (encodedBytes > 0) {
				opusData.insert(opusData.end(), tempBuffer.begin(), tempBuffer.begin() + encodedBytes);
				std::cout << "[Opus Encoding] Frame " << (i/frameSize) << " | Encoded bytes: " << encodedBytes << " | First Byte: " << (int)tempBuffer[0] << std::endl;
			}
			else {
				std::cout << "[Opus Encoding] Error: " + encodedBytes << std::endl;
				break;
			}


		}

		CloseEncoder(encoder);

		return opusData;

	}


	/**
	* Continuously records and encodes PCM data into Opus frames.
	*/
	std::vector<std::vector<unsigned char>> StreamEncodeMic() {
		std::vector<std::vector<unsigned char>> encodedPackets;


		static unsigned int lastPos = 0;  // Static to persist between calls
		unsigned int recordPos = 0;
		unsigned int frameSize = 960;

		fSystem->getRecordPosition(inputDeviceIndex, &recordPos);
		std::cout << "[DEBUG] Mic Record Position | Current: " << recordPos << " | Last: " << lastPos << std::endl;

		unsigned int availableSamples = (recordPos != lastPos) ? (recordPos - lastPos) : frameSize;


		PCMBufferLock lock = LockSound(micInput, lastPos, availableSamples * sizeof(short));
		if (lock.len1 > 0) {
			std::vector<short> pcmData = ExtractPCMData(&lock);
			encodedPackets.push_back(EncodeOpusFrame(pcmData));
		}
		UnlockSound(micInput, &lock);

		// Start from beginning of buffer
		lastPos = recordPos;

		return encodedPackets;
	}




	/**
	* Decodes a single Opus packet into PCM samples.
	*/
	FMOD::Sound* DecodeOpusFrame(std::vector<unsigned char>& opusData) {
		std::cout << "[Debug] Decoding Opus Frame | " << opusData.size() << std::endl;
		OpusDecoder* decoder = OpenDecoder();
		std::vector<short> pcmData(960);

		int decodedSamples = opus_decode(decoder, opusData.data(), opusData.size(), pcmData.data(), pcmData.size(), 0);

		if (decodedSamples < 0) {
			std::cerr << "[ERROR] Opus Decoding Failed! Error Code: " << decodedSamples << std::endl;
			CloseDecoder(decoder);
			return nullptr;
		}

		if (decodedSamples < 960) {
			std::cerr << "[WARNING] Decoded frame is smaller than expected! Size: " << decodedSamples << std::endl;
		}

		pcmData.resize(decodedSamples);
		CloseDecoder(decoder);

		return PCMToFSound(pcmData);
	}



	/**
	* Continuously decodes Opus frames into PCM and plays them back.
	*/
	void StreamDecodePlayback(std::vector<std::vector<unsigned char>>& encodedPackets) {
		std::cout << "[DEBUG] StreamDecodePlayback | Packets: " << encodedPackets.size() << std::endl;

		if (encodedPackets.empty()) {
			std::cerr << "[ERROR] No Encoded Data to Decode!" << std::endl;
			return;
		}

		OpusDecoder* decoder = OpenDecoder();
		if (!decoder) {
			std::cerr << "[ERROR] Failed to Open Opus Decoder!" << std::endl;
			return;
		}

		std::queue<std::vector<short>> pcmQueue;

		for (auto& packet : encodedPackets) {
			std::cout << "[DEBUG] Calling DecodeOpusFrame()" << std::endl;

			FMOD::Sound* sound = DecodeOpusFrame(packet);
			if (sound) {
				std::cout << "[DEBUG] Successfully decoded and created FMOD sound!" << std::endl;
				fSystem->playSound(sound, audioEngine->GetChannelGroup(ChannelGroupType::MASTER), false, &tempChannel);

				// Prevent PCM queue from growing too large (causing slow playback)
				if (pcmQueue.size() > 5) {
					std::cout << "[WARNING] PCM Queue too large, discarding old frames!" << std::endl;
					pcmQueue.pop();
				}
			}
			else {
				std::cerr << "[ERROR] DecodeOpusFrame() failed!" << std::endl;
			}
		}

		CloseDecoder(decoder);
	}




	/**
	* Streams and plays decoded PCM dynamically.
	*/
	void PlayPCMBuffer(std::queue<std::vector<short>>& pcmQueue) {
		while (!pcmQueue.empty()) {
			std::vector<short> pcmData = pcmQueue.front();
			pcmQueue.pop();

			std::cout << "[DEBUG] Playing PCM | Samples: " << pcmData.size()
				<< " | First Sample: " << pcmData[0] << std::endl;

			FMOD::Sound* sound = PCMToFSound(pcmData);
			if (sound) {
				std::cout << "[DEBUG] Playing New PCM Buffer..." << std::endl;
				fSystem->playSound(sound, audioEngine->GetChannelGroup(ChannelGroupType::MASTER), false, &tempChannel);

				// Ensure FMOD processes playback immediately
				fSystem->update();
			}
			else {
				std::cerr << "[ERROR] PCMToFSound Failed!" << std::endl;
			}
		}
	}




	/**
	* Converts a PCM buffer into an FMOD sound for playback.
	*/
	FMOD::Sound* PCMToFSound(const std::vector<short>& pcmData) {
		FMOD::Sound* sound;

		FMOD_CREATESOUNDEXINFO exinfo = {};
		exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
		exinfo.length = pcmData.size() * sizeof(short);
		exinfo.format = FMOD_SOUND_FORMAT_PCM16;
		exinfo.numchannels = channels;
		exinfo.defaultfrequency = sampleRate;

		FMOD_RESULT result = fSystem->createSound(
			reinterpret_cast<const char*>(pcmData.data()),
			FMOD_OPENMEMORY | FMOD_OPENRAW,
			&exinfo,
			&sound
		);
		if (result != FMOD_OK) {
			std::cerr << "[ERROR] PCMToFSound Failed! FMOD Error: " << FMOD_ErrorString(result) << std::endl;
			return nullptr;
		}


		return sound;

	}


	#pragma endregion


	#pragma region Opus Testing
	void CheckSoundFormat(FMOD::Sound* sound) {
		FMOD_SOUND_FORMAT format;
		int channels, bits;
		sound->getFormat(nullptr, &format, &channels, &bits);

		std::cout << "Sound Format: " << format << ", Channels: " << channels << ", Bit Depth: " << bits << std::endl;

		if (format != FMOD_SOUND_FORMAT_PCM16) {
			std::cerr << "Warning: Sound is not in PCM16 format! Decoding may be incorrect." << std::endl;
		}
	}

	void UpdateAudioStreaming() {

		std::vector<std::vector<unsigned char>> encodedPackets;
		if (IsRecording()) encodedPackets = StreamEncodeMic();

		if (!encodedPackets.empty()) {
			StreamDecodePlayback(encodedPackets);
		}
		else {
			std::cout << "No encoded packets to decode!" << std::endl;
		}
	}
	#pragma endregion


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

	void OnAwake() override {

	}

private:

	// Listner id - always 0
	int fIndex = 0;

	FMOD_VECTOR fForward;
	FMOD_VECTOR fUp;

	FMOD::Sound* micInput;

	int sampleRate = 48000;
	int channels = 1;

	std::map<int, std::string> inputDeviceList;
	int inputDeviceIndex;

	std::map<int, std::string> outputDeviceList;
	int outputDeviceIndex;

	PerspectiveCamera* camera;

	FMOD::Channel* tempChannel;

};