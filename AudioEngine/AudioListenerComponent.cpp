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

	this->camera = &camera;
	// Comment out for where you want the up and forward vectors to be updated from for testing
	SetCamOrientation();
	//SetPlayerOrientation();


	// Initialise position and orientation vectors
	fPosition = VecToFMOD(transform->GetPosition());

}

AudioListenerComponent::~AudioListenerComponent() {
	
	if (micInput) {
		micInput->release();
	}
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
	
	if (audioEngine->IsRecording()) {
		audioEngine->StopRecording();
		// std::cout << "Recording Stopped" << std::endl;
	}
	else {
		RecordMic();
		// std::cout << "Recording Started" << std::endl;
	}
}


#pragma endregion


