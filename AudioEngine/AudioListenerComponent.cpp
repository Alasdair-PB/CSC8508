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


	// Create sound object for microphone input
	FMOD_CREATESOUNDEXINFO exinfo = {};
	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.numchannels = channels;  // Mono for voice
	exinfo.format = FMOD_SOUND_FORMAT_PCM16;
	exinfo.defaultfrequency = sampleRate;  // Match system sample rate
	exinfo.length = (int)(exinfo.defaultfrequency * sizeof(short) * 1);  // 1 seconds buffer

	fSystem->createSound(nullptr, FMOD_OPENUSER, &exinfo, &micInput);


	encoder = OpenEncoder();
	decoder = OpenDecoder();

	// Create sound object for microphone input
	FMOD_CREATESOUNDEXINFO streamExinfo = {};
	streamExinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	streamExinfo.numchannels = channels;  // Mono for voice
	streamExinfo.format = FMOD_SOUND_FORMAT_PCM16;
	streamExinfo.defaultfrequency = sampleRate;  // Match system sample rate
	streamExinfo.length = ringBufferMaxSize * sizeof(short);
	streamExinfo.pcmreadcallback = PCMReadCallback;
	streamExinfo.userdata = this;


	FMOD_RESULT result = fSystem->createSound(0, FMOD_OPENUSER | FMOD_LOOP_NORMAL, &streamExinfo, &streamSound);
	if (result != FMOD_OK) {
		std::cerr << "[ERROR] createSound (stream): " << FMOD_ErrorString(result) << std::endl;
	}
	result = fSystem->playSound(streamSound, audioEngine->GetChannelGroup(ChannelGroupType::MASTER), false, &streamChannel);
	if (result != FMOD_OK) {
		std::cerr << "[ERROR] playSound (stream): " << FMOD_ErrorString(result) << std::endl;
	}


	this->camera = &camera;

	// Initialise position and orientation vectors
	fPosition = VecToFMOD(transform->GetPosition());

	// Comment out for where you want the up and forward vectors to be updated from for testing
	//SetCamOrientation();
	SetPlayerOrientation();

	InitPersistentSound();

}

void AudioListenerComponent::Update(float deltatime) {
	Vector3 pos = transform->GetPosition();
	fPosition = VecToFMOD(transform->GetPosition());


	// Comment out for where you want the up and forward vectors to be updated from for testing
	//SetCamOrientation();
	SetPlayerOrientation();

	fSystem ? fSystem->set3DListenerAttributes(fIndex, &fPosition, &fVelocity, &fForward, &fUp) : 0;

	if (debug) {
		Debug::Print("Listener Pos: " + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z), Vector2(5, 5));
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

void AudioListenerComponent::ToggleRecording() {
	
	if (IsRecording()) {
		StopRecording();
		//// std::cout << "Recording Stopped" << std::endl;
	}
	else {
		RecordMic();
		//// std::cout << "Recording Started" << std::endl;
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
	for (auto& device : outputDeviceList) {
		// std::cout << "Device: " << device.first << " - " << device.second << std::endl;
	}
}

static FMOD_RESULT F_CALLBACK PCMReadCallback(FMOD_SOUND* sound, void* data, unsigned int datalen) {
	void* userData = nullptr;
	FMOD_RESULT result = FMOD_Sound_GetUserData(sound, &userData);

	if (result != FMOD_OK || userData == nullptr) {
		return FMOD_ERR_INVALID_HANDLE;
	}

	AudioListenerComponent* instance = static_cast<AudioListenerComponent*>(userData);
	instance->PopSamples(data, datalen);
	return FMOD_OK;
}