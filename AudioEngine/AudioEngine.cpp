//
// Contributors: Max Bolton
//

#include "AudioEngine.h"
#include "AudioListenerComponent.h"
#include "AudioSourceComponent.h"
#include <fmod.hpp>
#include <iostream>
#include <opus/opus.h>

AudioEngine::AudioEngine() : audioSystem(nullptr) {
    FMOD::System_Create(&audioSystem);


	audioSystem->setDriver(0);

	// Set FMOD to use a right-handed coordinate system
	audioSystem->init(512, FMOD_INIT_3D_RIGHTHANDED, nullptr);

	// Create channel groups
	masterGroup = CreateChannelGroups(ChannelGroupType::MASTER, "Master");

	musicGroup = CreateChannelGroups(ChannelGroupType::MUSIC, "Music");
	sfxGroup = CreateChannelGroups(ChannelGroupType::SFX, "SFX");
	voiceGroup = CreateChannelGroups(ChannelGroupType::CHAT, "Voice");

	masterGroup->addGroup(musicGroup);
	masterGroup->addGroup(sfxGroup);
	masterGroup->addGroup(voiceGroup);

	encodeThreadRunning = true;
	decodeThreadRunning = true;

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
	encodeThreadRunning = false;
	decodeThreadRunning = false;
	StopEncodeThread();
	StopDecodeThread();
    if (audioSystem) {
        audioSystem->close();
        audioSystem->release();
    }
}

FMOD::ChannelGroup* AudioEngine::CreateChannelGroups(ChannelGroupType type, const char* name) {
	FMOD::ChannelGroup* group;
	if (audioSystem->createChannelGroup(name, &group) == FMOD_OK) {
		channelGroups[type] = group;
		return group;
	}
	else {
		// Error creating channel group
		return nullptr;
	}
}


void AudioEngine::StartEncodeThread(AudioListenerComponent* listener) {
	std::cout << "Audio Encode Started" << std::endl;

	encodeThread = std::thread([this, listener]() {
		// Set the update interval to 20ms
		auto nextUpdateTime = std::chrono::steady_clock::now();
		while (encodeThreadRunning) {
			// Schedule the next update
			nextUpdateTime += std::chrono::milliseconds(20);

			// Call audio encoding if mic is recording
			if(listener->IsRecording()) listener->UpdateAudioEncode();

			// Sleep until the next scheduled update time
			std::this_thread::sleep_until(nextUpdateTime);
		}
		});
}

void AudioEngine::StopEncodeThread() {
	if (encodeThread.joinable()) {
		encodeThread.join();
		std::cout << "Encode Thread Stopped" << std::endl;
	}
}

void AudioEngine::StartDecodeThread(AudioSourceComponent* source) {
	std::cout << "Audio Decode Started" << std::endl;

	decodeThread = std::thread([this, source]() {
		// Set the update interval to 20ms
		auto nextUpdateTime = std::chrono::steady_clock::now();
		while (decodeThreadRunning) {
			// Schedule the next update
			nextUpdateTime += std::chrono::milliseconds(20);

			// Decode packets if available
			if (!encodedPacketQueue.empty()) source->UpdateAudioDecode();

			// Sleep until the next scheduled update time
			std::this_thread::sleep_until(nextUpdateTime);
		}
		});
}

void AudioEngine::StopDecodeThread() {
	if (decodeThread.joinable()) {
		decodeThread.join();
		std::cout << "Decode Thread Stopped" << std::endl;
	}
}