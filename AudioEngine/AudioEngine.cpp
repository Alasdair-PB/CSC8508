//
// Contributors: Max Bolton
//

#include "AudioEngine.h"
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

	soundDir = std::string(ASSETROOTLOCATION) + "Audio/";

	LoadSoundAssets();

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

bool AudioEngine::LoadSoundDirectory(const char* directory, EntitySoundGroup group) {

	// Get directory path
	std::string fullPath = soundDir + directory;

	// Check if directory exists
	if (!std::filesystem::exists(fullPath)) {
		//std::cerr << "Error: Directory " << fullPath << " does not exist!" << std::endl;
		return false;
	}

	uint32_t count = 0;
	// Iterate through directory
	for (const auto& entry : std::filesystem::directory_iterator(fullPath)) {
		std::string path = entry.path().string();
		std::string name = entry.path().filename().string();
		std::string soundName = name.substr(0, name.find_last_of("."));
		FMOD::Sound* sound;
		// Load sound
		if (audioSystem->createSound(path.c_str(), FMOD_3D, 0, &sound) != FMOD_OK) {
			//std::cerr << "Error loading sound (" << name << "): " << std::endl;
			break;
		}
		// Set 3D min max distance
		sound->set3DMinMaxDistance(minDistance, maxDistance);
		// Add sound to sound group
		soundGroups[group][soundName] = sound;
		count++;
	}

	return (count > 0);
}