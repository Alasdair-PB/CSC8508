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
