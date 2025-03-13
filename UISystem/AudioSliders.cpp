#include "AudioSliders.h"
#include <filesystem>        

using namespace NCL;
using namespace UI;

AudioSliders::AudioSliders() {
	audioEngine = &AudioEngine::Instance();
}

AudioSliders::~AudioSliders() {

}

void AudioSliders::Draw() {
	ImGui::SetNextWindowPos(ImVec2(800, 300));
	ImGui::SetNextWindowSize(ImVec2(500, 125));
	ImGui::Begin("Audio Sliders");

	masterVolume = audioEngine->GetChannelVolume(ChannelGroupType::MASTER);
	masterVolume = masterVolume * 100;
	ImGui::SliderFloat("Master Volume", &masterVolume, 0, 100, "%.0f");
	masterVolume = masterVolume / 100;
	audioEngine->SetChannelVolume(ChannelGroupType::MASTER, masterVolume);

	musicVolume = audioEngine->GetChannelVolume(ChannelGroupType::MUSIC);
	musicVolume = musicVolume * 100;
	ImGui::SliderFloat("Music Volume", &musicVolume, 0, 100, "%.0f");
	musicVolume = musicVolume / 100;
	audioEngine->SetChannelVolume(ChannelGroupType::MUSIC, musicVolume);

	sfxVolume = audioEngine->GetChannelVolume(ChannelGroupType::SFX);
	sfxVolume = sfxVolume * 100;
	ImGui::SliderFloat("SFX Volume", &sfxVolume, 0, 100, "%.0f");
	sfxVolume = sfxVolume / 100;
	audioEngine->SetChannelVolume(ChannelGroupType::SFX, sfxVolume);

	voiceVolume = audioEngine->GetChannelVolume(ChannelGroupType::CHAT);
	voiceVolume = voiceVolume * 100;
	ImGui::SliderFloat("Voice Volume", &voiceVolume, 0, 100, "%.0f");
	voiceVolume = voiceVolume / 100;
	audioEngine->SetChannelVolume(ChannelGroupType::CHAT, voiceVolume);

	ImGui::End();
}
