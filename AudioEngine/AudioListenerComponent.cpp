//
// Contributors: Max Bolton
//

#include "../AudioEngine/AudioListenerComponent.h"
#include <fmod.hpp>
#include <iostream>
#include <cmath>
#include <opus/opus.h>

using namespace NCL;
using namespace CSC8508;

AudioListenerComponent::AudioListenerComponent(GameObject& gameObject, PerspectiveCamera& camera) : AudioObject(gameObject) {

	// Set devices to system default
	inputDeviceIndex = 0;
	outputDeviceIndex = 0;


	this->camera = &camera;
	// Comment out for where you want the up and forward vectors to be updated from for testing
	SetCamOrientation();
	//SetPlayerOrientation();


	// Initialise position and orientation vectors
	fPosition = VecToFMOD(transform->GetPosition());


	// Initialise Encoding Pipeline
	InitMicSound();
	encoder = OpenEncoder();
	encodedPacketQueue = &audioEngine->GetEncodedPacketQueue();

	audioEngine->StartEncodeThread(this);
}

AudioListenerComponent::~AudioListenerComponent() {
	CloseEncoder(encoder);
	CloseDecoder(decoder);
	if (IsRecording()) {
		StopRecording();
	}
	if (persistentSound) {
		persistentSound->release();
	}
	if (micInput) {
		micInput->release();
	}

	delete persistentSound;
	delete micInput;
}

void AudioListenerComponent::OnAwake()
{

}

void AudioListenerComponent::Update(float deltatime) {
	Vector3 pos = transform->GetPosition();
	fPosition = VecToFMOD(transform->GetPosition());


	// Comment out for where you want the up and forward vectors to be updated from for testing
	SetCamOrientation();
	//SetPlayerOrientation();

	fSystem ? fSystem->set3DListenerAttributes(fIndex, &fPosition, &fVelocity, &fForward, &fUp) : 0;

	if (debug) {
		std::cout << "Listener Pos: " << std::to_string(pos.x) << ", " << std::to_string(pos.y) << ", " << std::to_string(pos.z) << std::endl;
	}

}

void AudioListenerComponent::SetCamOrientation() {
		Quaternion forwardRotation = Quaternion::EulerAnglesToQuaternion(camera->GetPitch(), camera->GetYaw(), 0.0f);
		fForward = VecToFMOD(forwardRotation * Vector3(0, 0, -1));

		Quaternion upRotation = Quaternion::EulerAnglesToQuaternion(camera->GetPitch(), camera->GetYaw(), 0.0f);
		fUp = VecToFMOD(upRotation * Vector3(0, 1, 0));
}

void AudioListenerComponent::SetPlayerOrientation() {

	fForward = VecToFMOD(transform->GetOrientation() * Vector3(0, 0, -1));
	fUp = VecToFMOD(transform->GetOrientation() * Vector3(0, 1, 0));

}

#pragma region FMOD Recording
void AudioListenerComponent::InitMicSound() {
	FMOD_CREATESOUNDEXINFO exinfo = {};
	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.numchannels = channels;  // Mono for voice
	exinfo.format = FMOD_SOUND_FORMAT_PCM16;
	exinfo.defaultfrequency = sampleRate;  // Match system sample rate
	exinfo.length = (int)(exinfo.defaultfrequency * sizeof(short) * 1);  // 1 seconds buffer

	fSystem->createSound(nullptr, FMOD_OPENUSER, &exinfo, &micInput);
}

void AudioListenerComponent::ToggleRecording() {
	
	if (IsRecording()) {
		StopRecording();
		// std::cout << "Recording Stopped" << std::endl;
	}
	else {
		RecordMic();
		// std::cout << "Recording Started" << std::endl;
	}
}

bool AudioListenerComponent::IsRecording() {
	bool isRecording;
	FMOD_RESULT result = fSystem->isRecording(0, &isRecording);
	if (result != FMOD_OK) {
		// std::cerr << "Error: " << FMOD_ErrorString(result) << std::endl;
		return false;
	}

	return isRecording;
}
#pragma endregion

#pragma region Input/Output Device Management

void AudioListenerComponent::UpdateInputList() {
	int numDrivers = 0;
	fSystem->getRecordNumDrivers(&numDrivers, nullptr);

	inputDeviceList.clear();

	for (int i = 0; i < numDrivers; i++) {
		char name[256];
		FMOD_RESULT result = fSystem->getRecordDriverInfo(i, name, sizeof(name), nullptr, nullptr, nullptr, nullptr, nullptr);
		if (result == FMOD_OK && !strstr(name, "[loopback]")) {
			inputDeviceList.insert({i, name});
		}
	}

}

void AudioListenerComponent::PrintInputList() {
	UpdateInputList();
	for (auto& device : inputDeviceList) {
		// std::cout << "Device: " << device.first << " - " << device.second << std::endl;
	}
}

void AudioListenerComponent::UpdateOutputList() {
	int numOutputDrivers = 0;
	fSystem->getNumDrivers(&numOutputDrivers);  // Only counts output (playback) devices

	outputDeviceList.clear();  // Clear previous list

	for (int i = 0; i < numOutputDrivers; i++) {
		char name[256];

		// This API only provides playback devices
		FMOD_RESULT result = fSystem->getDriverInfo(i, name, sizeof(name), nullptr, nullptr, nullptr, nullptr);

		if (result == FMOD_OK) {
			outputDeviceList.insert({ i, name });  // Store playback devices
		}
	}
}

void AudioListenerComponent::PrintOutputList() {
	UpdateOutputList();
	for (auto& device : outputDeviceList) {
		std::cout << "Device: " << device.first << " - " << device.second << std::endl;
	}
}

#pragma endregion

#pragma region Encoding VOIP Pipeline

std::vector<unsigned char> AudioListenerComponent::EncodeOpusFrame(std::vector<short>& pcmData) {
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
			//std::cout << "[DEBUG] EncodeOpusFrame() Encoded " << encodedBytes << " bytes." << std::endl;
		}
		else {
			// std::cerr << "[ERROR] EncodeOpusFrame() Error Encoding PCM Data" << std::endl;
		}
	}
	return opusFrame;
}

void AudioListenerComponent::StreamEncodeMic() {
	static unsigned int lastPos = 0;
	unsigned int recordPos = 0;
	unsigned int frameSize = 960; // 20ms at 48kHz
	fSystem->getRecordPosition(0, &recordPos);


	unsigned int availableSamples;
	// Calculate the number of samples available to read
	if (recordPos < lastPos) {
		availableSamples = (sampleRate - lastPos) + recordPos;
	}
	else {
		availableSamples = recordPos - lastPos;
	}

	// Calculate the number of complete frames available
	unsigned int completeFrames = availableSamples / frameSize;
	if (completeFrames == 0) return;

	// Calculate the number of samples to process
	unsigned int samplesToProcess = completeFrames * frameSize;

	// Lock the sound to access the PCM data
	PCMBufferLock lock = LockSound(micInput, lastPos * sizeof(short), samplesToProcess * sizeof(short));
	if (lock.result != FMOD_OK) {
		std::cerr << "[ERROR] micInput lock: " << FMOD_ErrorString(lock.result) << std::endl;
		return;
	}

	// init pcmData vector
	std::vector<short> pcmData;

	// insert first chunk of data
	short* firstChunk = static_cast<short*>(lock.ptr1);
	pcmData.insert(pcmData.end(), firstChunk, firstChunk + (lock.len1 / sizeof(short)));

	// insert second chunk if it exists
	if (lock.ptr2 && lock.len2 > 0) {
		short* secndChunk = static_cast<short*>(lock.ptr2);
		pcmData.insert(pcmData.end(), secndChunk, secndChunk + (lock.len2 / sizeof(short)));
	}

	// Data has been copied, release lock
	UnlockSound(micInput, &lock);

	// Push samples to the ring buffer
	if (pcmData.size() > samplesToProcess) {
		pcmData.resize(samplesToProcess);
	}
	std::vector<unsigned char> encodedPacket = EncodeOpusFrame(pcmData);
	if (!encodedPacket.empty()) {
		encodedPacketQueue->push_back(encodedPacket);
	}

	// Update last position
	lastPos = (lastPos + samplesToProcess) % sampleRate;
}

void AudioListenerComponent::InitPersistentSound() {
	FMOD_CREATESOUNDEXINFO exinfo = {};
	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.numchannels = channels;
	exinfo.format = FMOD_SOUND_FORMAT_PCM16;
	exinfo.defaultfrequency = sampleRate;
	exinfo.length = persistentBufferSize * sizeof(short); // total buffer size in bytes

	FMOD_RESULT result = fSystem->createSound(nullptr, FMOD_OPENUSER | FMOD_LOOP_NORMAL, &exinfo, &persistentSound);

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

#pragma endregion

std::vector<short> AudioListenerComponent::DecodeOpusFrame(std::vector<unsigned char>& encodedPacket) {
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

void AudioListenerComponent::DecodePersistentPlayback(std::vector<unsigned char>& encodedPacket) {

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
