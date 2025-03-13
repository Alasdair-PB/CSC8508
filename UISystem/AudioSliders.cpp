#include "AudioSliders.h"
#include <filesystem>        

using namespace NCL;
using namespace UI;

AudioSliders::AudioSliders() {
	audioEngine = &AudioEngine::Instance();
}

AudioSliders::~AudioSliders() {

}

void AudioSliders::Draw(int winWidth, int winHeight) {
	ImGui::SetNextWindowPos(ImVec2(winWidth * 0.4, winHeight * 0.3));
	ImGui::SetNextWindowSize(ImVec2(winWidth * 0.3, winHeight * 0.12));
	bool open = true;
	ImGui::Begin("Audio Sliders", &open, ImGuiWindowFlags_NoResize);

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