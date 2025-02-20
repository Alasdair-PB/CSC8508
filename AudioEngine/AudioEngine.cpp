//
// Contributors: Max Bolton
//

#include "AudioEngine.h"
#include <fmod.hpp>
#include <iostream>

AudioEngine::AudioEngine() : audioSystem(nullptr) {
    FMOD::System_Create(&audioSystem);
	// Set FMOD to use a right-handed coordinate system
	audioSystem->init(512, FMOD_INIT_3D_RIGHTHANDED, nullptr);
}


AudioEngine::~AudioEngine() {
	Shutdown();
}

AudioEngine& AudioEngine::Instance() {
	static AudioEngine instance;
	return instance;
}

void AudioEngine::Init() {
	FMOD::System_Create(&audioSystem);
	audioSystem->init(512, FMOD_INIT_NORMAL, nullptr);
}

void AudioEngine::Update() {
    audioSystem->update();
}

void AudioEngine::Shutdown() {
    if (audioSystem) {
        audioSystem->close();
        audioSystem->release();
    }
}
