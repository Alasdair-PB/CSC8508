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
	exinfo.numchannels = 1;  // Mono for voice
	exinfo.format = FMOD_SOUND_FORMAT_PCM16;
	exinfo.defaultfrequency = 48000;  // Match system sample rate
	exinfo.length = exinfo.defaultfrequency * sizeof(short) * 5;  // 5 seconds buffer

	fSystem->createSound(nullptr, FMOD_OPENUSER | FMOD_3D, &exinfo, &micInput);

	this->camera = &camera;

	// Initialise position and orientation vectors
	fPosition = VecToFMOD(transform->GetPosition());

	// Comment out for where you want the up and forward vectors to be updated from for testing
	//SetCamOrientation();
	SetPlayerOrientation();

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
		std::cout << "Recording Stopped" << std::endl;
	}
	else {
		RecordMic();
		std::cout << "Recording Started" << std::endl;
	}
}

bool AudioListenerComponent::IsRecording() {
	bool isRecording;
	FMOD_RESULT result = fSystem->isRecording(0, &isRecording);
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
		std::cout << "Device: " << device.first << " - " << device.second << std::endl;
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
		std::cout << "Device: " << device.first << " - " << device.second << std::endl;
	}
}