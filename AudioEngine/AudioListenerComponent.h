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
	* Locks a section of the FMOD PCM buffer for reading/writing.
	* FMOD writes to a circular buffer so must lock portions to prevent data corruption.
	* Lock can contain two pointers if the buffer wraps around.
	* 1st pointer is the start of the audio, 2nd holds the remainder if the buffer wraps.
	* @return PCMBufferLock (struct containing buffer info)
	* @param FMOD::Sound* (sound to lock)
	* @param unsigned int (lastPos)
	* @param unsigned int (bufferSize)
	*
	PCMBufferLock LockSound(FMOD::Sound* sound, unsigned int lastPos, unsigned int bufferSize) {
		PCMBufferLock lock = {};

		unsigned int recordPos = 0;
		fSystem->getRecordPosition(inputDeviceIndex, &recordPos);


		FMOD_RESULT result = sound->lock(recordPos, bufferSize, &lock.ptr1, &lock.ptr2, &lock.len1, &lock.len2);

		if (result == FMOD_OK) {
			lock.offset = lastPos;
			lock.length = lock.len1 + lock.len2;
		}
		else {
			std::cout << "[ERROR] LockSound(): result = " << FMOD_ErrorString(result) << std::endl;
		}

		return lock;
	}*/

	PCMBufferLock LockSound(FMOD::Sound* sound, unsigned int startOffset, unsigned int buffersize) {
		PCMBufferLock lock = {};
		FMOD_RESULT result = sound->lock(startOffset, buffersize, &lock.ptr1, &lock.ptr2, &lock.len1, &lock.len2);
		if (result == FMOD_OK) {
			lock.offset = startOffset;
			lock.length = lock.len1 + lock.len2;
		}
		else {
			std::cerr << "[ERROR] LockSound(): " << FMOD_ErrorString(result) << std::endl;
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
	* Converts a PCM buffer into an FMOD sound for playback.
	* @return FMOD::Sound* (FMOD sound object)
	* @param std::vector<short>& (PCM data to convert)
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
			reinterpret_cast<const char*>(pcmData.data()),FMOD_OPENRAW | FMOD_OPENMEMORY,
			&exinfo,
			&sound
		);
		if (result != FMOD_OK) {
			// std::cerr << "[ERROR] PCMToFSound() FMOD Error: " << FMOD_ErrorString(result) << std::endl;
			return nullptr;
		}


		return sound;

	}


	#pragma endregion


	#pragma region Encoding/Decoding Pipeline

	/**
	* Extracts PCM samples from a locked buffer.
	* Handles single or double buffer chunk locks.
	* @return std::vector<short> (extracted PCM data)
	* @param PCMBufferLock* (locked buffer to extract from)
	*/
	std::vector<short> ExtractPCMData(PCMBufferLock* lock) {
		std::vector<short> pcmData;

		// Extract the first chunk (ptr1)
		if (lock->ptr1 && lock->len1 > 0) {
			short* pcmShorts = static_cast<short*>(lock->ptr1);
			pcmData.insert(pcmData.end(), pcmShorts, pcmShorts + (lock->len1 / sizeof(short)));
			
			 std::cout << "[DEBUG]ExtractPCMData(): First Chunk = " << lock->len1 / sizeof(short) << " samples." << std::endl;
		}
		else {
			std::cerr << "[ERROR] ExtractPCMData(): PCM Extract Failure, chunk 1 invalid." << std::endl;
		}

		// Extract the second chunk (ptr2) if available
		if (lock->ptr2 && lock->len2 > 0) {
			short* pcmShorts = static_cast<short*>(lock->ptr2);
			pcmData.insert(pcmData.end(), pcmShorts, pcmShorts + (lock->len2 / sizeof(short)));

			std::cout << "[Debug]ExtractPCMData(): Second Chunk = " << lock->len2 / sizeof(short) << " samples." << std::endl;
		}
		else {
			std::cerr << "[Debug]ExtractPCMData(): No chunk 2." << std::endl;
		}
			std::cout << "[Debug]ExtractPCMData(): Total Extracted Samples: " << pcmData.size() << std::endl;
		return pcmData;
	}

	void PushSamples(const std::vector<short>& samples) {
		std::lock_guard<std::mutex> lock(ringBufferMutex);

		for (short s : samples) {
			if (ringBuffer.size() < ringBufferMaxSize) {
				ringBuffer.push_back(s);
			}
			else {
				ringBuffer.pop_front();
				ringBuffer.push_back(s);
			}
		}
	}

	int PopSamples(void* dest, unsigned int bytesRequested) {
		unsigned int samplesRequested = bytesRequested / sizeof(short);

		std::lock_guard<std::mutex> lock(ringBufferMutex);

		unsigned int samplesAvailable = ringBuffer.size();
		unsigned int samplesToRead = (samplesRequested < samplesAvailable) ? samplesRequested : samplesAvailable;

		short* out = static_cast<short*>(dest);

		for (unsigned int i = 0; i < samplesToRead; i++) {
			out[i] = ringBuffer.front();
			ringBuffer.pop_front();
		}

		for (unsigned int i = samplesToRead; i < samplesRequested; i++) {
			out[i] = 0;
		}

		return samplesToRead * sizeof(short);
	}

	/**
	* Encodes a PCM sample buffer to an Opus frame.
	* Each frame contains 960 samples (20ms at 48kHz).
	* @return std::vector<unsigned char> (encoded Opus frame)
	* @param std::vector<short>& (PCM data to encode)
	*/
	std::vector<unsigned char> EncodeOpusFrame(std::vector<short>& pcmData) {
		std::vector<unsigned char> opusFrame;

		std::vector<unsigned char> tempBuffer(4096); // ensures buffer is large enough for any frame
		int frameSize = 960;

		// Ensure PCM data is always a multiple of 960 samples
		if (pcmData.size() % frameSize != 0) {
			pcmData.resize((pcmData.size() / frameSize) * frameSize);  // Trim to nearest multiple of 960
		}

		for (size_t i = 0; i < pcmData.size(); i += frameSize) {
			int encodedBytes = opus_encode(encoder, pcmData.data() + i, frameSize, tempBuffer.data(), tempBuffer.size());

			// if encoding successful, append the encoded frame to the output buffer
			if (encodedBytes > 0) {
				opusFrame.insert(opusFrame.end(), tempBuffer.begin(), tempBuffer.begin() + encodedBytes);
				std::cout << "[DEBUG] EncodeOpusFrame() Encoded " << encodedBytes << " bytes." << std::endl;
			}
			else {
				// std::cerr << "[ERROR] EncodeOpusFrame() Error Encoding PCM Data" << std::endl;
			}
		}
		return opusFrame;
	}


	/**
	* Continuously records and encodes PCM data into Opus frames.
	*
	void StreamEncodeMic() {
		static unsigned int lastPos = 0;
		unsigned int recordPos = 0;
		unsigned int frameSize = 960;  // Standard Opus frame size

		fSystem->getRecordPosition(inputDeviceIndex, &recordPos);


		std::cout << lastPos << " -> " << recordPos << std::endl;

		unsigned int availableSamples;
		if (recordPos < lastPos) {
			availableSamples = (micBufferSamples - lastPos) + recordPos;
		}
		else {
			availableSamples = recordPos - lastPos;
		}


		unsigned int completeFrames = availableSamples / frameSize;
		if (completeFrames == 0) return; // Not enough samples for a full frame


		unsigned int samplesToProcess = completeFrames * frameSize;
		PCMBufferLock lock = LockSound(micInput, lastPos, samplesToProcess * sizeof(short));


		if (lock.len1 == 0) {
			UnlockSound(micInput, &lock);
			return;
		}
		
		std::vector<short> pcmData = ExtractPCMData(&lock);

		if (pcmData.size() > samplesToProcess) {
			pcmData.resize(samplesToProcess);
		}

		// encode each frame of PCM data and store the encoded packets
		for (size_t i = 0; i < pcmData.size(); i += frameSize) {
			std::vector<short> frame(pcmData.begin() + i, pcmData.begin() + i + frameSize);
			std::vector<unsigned char> encodedPacket = EncodeOpusFrame(frame);
			if (!encodedPacket.empty()) {
				encodedPacketQueue.push_back(encodedPacket);
				// Limit the queue size.
				if (encodedPacketQueue.size() > 50) {
					encodedPacketQueue.pop_front();
				}
			}
		}

		UnlockSound(micInput, &lock);
		lastPos = (lastPos + samplesToProcess) % micBufferSamples;
	}*/

	void StreamEncodeMic() {
		static unsigned int lastPos = 0;
		unsigned int recordPos = 0;
		unsigned int frameSize = 960; // 20ms at 48kHz
		fSystem->getRecordPosition(0, &recordPos);

		unsigned int availableSamples = (recordPos < lastPos)
			? ((sampleRate - lastPos) + recordPos)
			: (recordPos - lastPos);
		unsigned int completeFrames = availableSamples / frameSize;
		if (completeFrames == 0)
			return;
		unsigned int samplesToProcess = completeFrames * frameSize;

		void* ptr1 = nullptr, * ptr2 = nullptr;
		unsigned int len1 = 0, len2 = 0;
		FMOD_RESULT result = micInput->lock(lastPos * sizeof(short), samplesToProcess * sizeof(short),
			&ptr1, &ptr2, &len1, &len2);
		if (result != FMOD_OK) {
			std::cerr << "[ERROR] micInput lock: " << FMOD_ErrorString(result) << std::endl;
			return;
		}
		std::vector<short> pcmData;
		short* s1 = static_cast<short*>(ptr1);
		pcmData.insert(pcmData.end(), s1, s1 + (len1 / sizeof(short)));
		if (ptr2 && len2 > 0) {
			short* s2 = static_cast<short*>(ptr2);
			pcmData.insert(pcmData.end(), s2, s2 + (len2 / sizeof(short)));
		}
		micInput->unlock(ptr1, ptr2, len1, len2);
		if (pcmData.size() > samplesToProcess) {
			pcmData.resize(samplesToProcess);
		}
		std::vector<unsigned char> encodedPacket = EncodeOpusFrame(pcmData);
		if (!encodedPacket.empty()) {
			encodedPacketQueue.push_back(encodedPacket);
		}

		lastPos = (lastPos + samplesToProcess) % sampleRate;
	}



	void InitPersistentSound() {
		FMOD_CREATESOUNDEXINFO exinfo = {};
		exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
		exinfo.numchannels = channels;
		exinfo.format = FMOD_SOUND_FORMAT_PCM16;
		exinfo.defaultfrequency = sampleRate;
		exinfo.length = persistentBufferSize * sizeof(short); // total buffer size in bytes

		FMOD_RESULT result = fSystem->createSound(nullptr, FMOD_OPENUSER | FMOD_LOOP_NORMAL , &exinfo, &persistentSound);

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
		currentWritePos = 0;

	}

	void UpdatePersistentPlayback(std::vector<unsigned char>& encodedPacket) {

		if (!persistentSound) return;

		std::vector<short> pcmFrame(960);
		int decodedSamples = opus_decode(decoder, encodedPacket.data(), encodedPacket.size(), pcmFrame.data(), pcmFrame.size(), 0);

		std::cout << "[DEBUG] UpdatePersistentPlayback() Decoded " << decodedSamples << " samples." << std::endl;

		if (decodedSamples < 0) {
			std::cerr << "[ERROR] UpdatePersistentPlayback() Opus Decoding Failed: " << decodedSamples << std::endl;
			return;
		}

		if (decodedSamples < static_cast<int>(pcmFrame.size())) {
			pcmFrame.resize(decodedSamples);
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

	void UpdateAudioDecodePersistent() {
		if (!encodedPacketQueue.empty()) {
			std::vector<unsigned char> packet = encodedPacketQueue.front();
			encodedPacketQueue.pop_front();  // Process oldest frame first
			UpdatePersistentPlayback(packet);
		}
	}


	/**
	* Decodes an Opus packet back into an FMOD sound.
	* @return FMOD::Sound* (decoded PCM buffer converted to FMOD sound)
	* @param std::vector<unsigned char>& (Opus data frame to decode)
	*/
	FMOD::Sound* DecodeOpusFrame(std::vector<unsigned char>& opusFrame) {
		//OpusDecoder* decoder = OpenDecoder();
		std::vector<short> pcmData(960); // 960 samples per frame

		int decodedSamples = opus_decode(decoder, opusFrame.data(), opusFrame.size(), pcmData.data(), pcmData.size(), 0);

		if (decodedSamples < 0) {
			// std::cerr << "[ERROR]DecodeOpusFrame() Opus Decoding Failed: " << decodedSamples << std::endl;
			//CloseDecoder(decoder);
			return nullptr;
		}

		if (decodedSamples < static_cast<int>(pcmData.size())) {
			pcmData.resize(decodedSamples);
		}

		std::cout << "[DEBUG] DecodeOpusFrame() Decoded " << decodedSamples << " samples." << std::endl;
		//CloseDecoder(decoder);

		return PCMToFSound(pcmData);
	}

	/**
	* Continuously decodes Opus frames into PCM and plays them back.
	* @param std::vector<unsigned char>& (encoded Opus data packet to decode)
	*/
	void PlaybackOpusFrame(std::vector<unsigned char>& encodedPacket) {

		if (encodedPacket.empty()) {
			// std::cerr << "[ERROR] PlaybackOpusFrame(): No Encoded Data to Decode!" << std::endl;
			return;
		}

		FMOD::Sound* sound = DecodeOpusFrame(encodedPacket);
		if (sound) {
			//// std::cout << "[DEBUG] Successfully decoded and created FMOD sound!" << std::endl;
			fSystem->playSound(sound, audioEngine->GetChannelGroup(ChannelGroupType::MASTER), false, &tempChannel);
		}
		else {
			// std::cerr << "[ERROR] PlaybackOpusFrame(): DecodeOpusFrame() failed!" << std::endl;
		}
	}

	#pragma endregion


	#pragma region Opus Testing




	void UpdateAudioEncode(){

		StreamEncodeMic();

		if (encodedPacketQueue.empty()) {
			//// std::cerr << "[ERROR] UpdateAudioEncode() No encoded packets available for playback!" << std::endl;
		}
	}

	/*
	void UpdateAudioDecode() {
		if (!encodedPacketQueue.empty()) {
			std::vector<unsigned char> packet = encodedPacketQueue.front();
			encodedPacketQueue.pop_front();  // Process oldest frame first

			PlaybackOpusFrame(packet);  // Decode & Play Audio
			//UpdateAudioDecodePersistent();
		}
	}*/

	void UpdateAudioDecode() {
		if (!encodedPacketQueue.empty()) {
			std::vector<unsigned char> packet = encodedPacketQueue.front();
			encodedPacketQueue.pop_front();
			std::vector<short> pcmFrame(960);  // Assume 960 samples per frame.
			int decodedSamples = opus_decode(decoder, packet.data(), packet.size(), pcmFrame.data(), pcmFrame.size(), 0);

			std::cout << "[DEBUG] UpdateAudioDecode() Decoded " << decodedSamples << " samples." << std::endl;
			if (decodedSamples < 0) {
				std::cerr << "[ERROR] Opus decoding failed: " << decodedSamples << std::endl;
				return;
			}
			if (decodedSamples < static_cast<int>(pcmFrame.size()))
				pcmFrame.resize(decodedSamples);
			PushSamples(pcmFrame);
		}
		else {
			std::cerr << "[ERROR] UpdateAudioDecode() No encoded packets available for playback!" << std::endl;
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

	std::map<int, std::string> inputDeviceList;
	int inputDeviceIndex;

	std::map<int, std::string> outputDeviceList;
	int outputDeviceIndex;

	PerspectiveCamera* camera;

	FMOD::Sound* decodeOut;
	FMOD::Channel* tempChannel;

	OpusDecoder* decoder;
	OpusEncoder* encoder;

	unsigned int micBufferSamples = (int)(48000 * sizeof(short) * 1) / sizeof(short);
	int channels = 1;
	int sampleRate = 48000;

	FMOD::Sound* persistentSound = nullptr;
	FMOD::Channel* persistentChannel = nullptr;
	unsigned int persistentBufferSize = sampleRate / 2; // 0.5 second
	unsigned int currentWritePos = 0; // in samples

	FMOD::Sound* streamSound;
	FMOD::Channel* streamChannel;

	std::deque<std::vector<unsigned char>> encodedPacketQueue;

	std::deque<short> ringBuffer;
	std::mutex ringBufferMutex;

	unsigned int ringBufferMaxSize = sampleRate / 2;

};