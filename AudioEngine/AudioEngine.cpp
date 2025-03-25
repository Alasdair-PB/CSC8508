//
// Contributors: Max Bolton
//

#include "AudioEngine.h"
#include <fmod.hpp>
#include <iostream>
#include <opus/opus.h>

AudioEngine::AudioEngine() : audioSystem(nullptr) {
	#ifdef USE_PS5
	std::string s_libraryPath = "/app0/sce_module/";
	int loaded;
	char* path = (char*)s_libraryPath.append("libfmod.prx").c_str();
	libfmodHandle = sceKernelLoadStartModule(path, 0, NULL, 0, NULL, &loaded);

	path = (char*)s_libraryPath.append("libfmodL.prx").c_str();
	libfmodLHandle = sceKernelLoadStartModule(path, 0, NULL, 0, NULL, &loaded);
	#endif // USE_PS5


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
	
	#ifdef USE_PS5
	
	int unloaded;
	unloaded = sceKernelStopUnloadModule(libfmodHandle, 0, NULL, 0, NULL, NULL);
	unloaded = sceKernelStopUnloadModule(libfmodLHandle, 0, NULL, 0, NULL, NULL);

	#endif // USE_PS5

}

AudioEngine& AudioEngine::Instance() {
	static AudioEngine instance;
	return instance;
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

#pragma region Input/Output Device Management

void AudioEngine::UpdateInputList() {
	int numDrivers = 0;
	audioSystem->getRecordNumDrivers(&numDrivers, nullptr);

	inputDeviceList.clear();

	for (int i = 0; i < numDrivers; i++) {
		char name[256];
		FMOD_RESULT result = audioSystem->getRecordDriverInfo(i, name, sizeof(name), nullptr, nullptr, nullptr, nullptr, nullptr);
		if (result == FMOD_OK && !strstr(name, "[loopback]")) {
			inputDeviceList.insert({ i, name });
		}
	}

}

void AudioEngine::PrintInputList() {
	UpdateInputList();
	for (auto& device : inputDeviceList) {
		// std::cout << "Device: " << device.first << " - " << device.second << std::endl;
	}
}

void AudioEngine::UpdateOutputList() {
	int numOutputDrivers = 0;
	audioSystem->getNumDrivers(&numOutputDrivers);  // Only counts output (playback) devices

	outputDeviceList.clear();  // Clear previous list

	for (int i = 0; i < numOutputDrivers; i++) {
		char name[256];

		// This API only provides playback devices
		FMOD_RESULT result = audioSystem->getDriverInfo(i, name, sizeof(name), nullptr, nullptr, nullptr, nullptr);

		if (result == FMOD_OK) {
			outputDeviceList.insert({ i, name });  // Store playback devices
		}
	}
}

void AudioEngine::PrintOutputList() {
	UpdateOutputList();
	for (auto& device : outputDeviceList) {
		std::cout << "Device: " << device.first << " - " << device.second << std::endl;
	}
}

bool AudioEngine::IsRecording() {
	bool isRecording;
	FMOD_RESULT result = audioSystem->isRecording(0, &isRecording);
	if (result != FMOD_OK) {
		// std::cerr << "Error: " << FMOD_ErrorString(result) << std::endl;
		return false;
	}

	return isRecording;
}

#pragma endregion


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