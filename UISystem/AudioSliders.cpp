#include "AudioSliders.h"
#include <filesystem>        
#include <iostream>

using namespace NCL;
using namespace UI;

AudioSliders::AudioSliders() {
	audioEngine = &AudioEngine::Instance();
	masterVolume = audioEngine->GetChannelVolume(ChannelGroupType::MASTER);
	masterVolume = masterVolume * 100;

	musicVolume = audioEngine->GetChannelVolume(ChannelGroupType::MUSIC);
	musicVolume = musicVolume * 100;

	sfxVolume = audioEngine->GetChannelVolume(ChannelGroupType::SFX);
	sfxVolume = sfxVolume * 100;

	voiceVolume = audioEngine->GetChannelVolume(ChannelGroupType::CHAT);
	voiceVolume = voiceVolume * 100;

	std::function<CSC8508::PushdownState::PushdownResult(float)> masterFunc = [this](float val) -> CSC8508::PushdownState::PushdownResult {
		masterVolume = val / 100;
		audioEngine->SetChannelVolume(ChannelGroupType::MASTER, masterVolume);
		return CSC8508::PushdownState::PushdownResult::NoChange;
	};

	std::function<CSC8508::PushdownState::PushdownResult(float)> musicFunc = [this](float val) -> CSC8508::PushdownState::PushdownResult {
		musicVolume = val / 100;
		audioEngine->SetChannelVolume(ChannelGroupType::MUSIC, musicVolume);
		return CSC8508::PushdownState::PushdownResult::NoChange;
	};

	std::function<CSC8508::PushdownState::PushdownResult(float)> sfxFunc = [this](float val) -> CSC8508::PushdownState::PushdownResult {
		masterVolume = val / 100;
		audioEngine->SetChannelVolume(ChannelGroupType::SFX, sfxVolume);
		return CSC8508::PushdownState::PushdownResult::NoChange;
	};

	std::function<CSC8508::PushdownState::PushdownResult(float)> voiceFunc = [this](float val) -> CSC8508::PushdownState::PushdownResult {
		masterVolume = val / 100;
		audioEngine->SetChannelVolume(ChannelGroupType::CHAT, voiceVolume);
		return CSC8508::PushdownState::PushdownResult::NoChange;
		};

	std::function<CSC8508::PushdownState::PushdownResult()> deviceFunc = [this]() -> CSC8508::PushdownState::PushdownResult {
		audioEngine->SetInputDeviceIndex(inputDevice);
		if (ImGui::BeginCombo("Input Device", audioEngine->GetInputDeviceList()[inputDevice].data()))
		{
			for (auto const& device : audioEngine->GetInputDeviceList()) {
				const bool is_selected = (inputDevice == device.first);
				if (ImGui::Selectable(device.second.c_str(), is_selected)) {
					inputDevice = device.first;
				}

				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		audioEngine->SetOutputDeviceIndex(outputDevice);
		if (ImGui::BeginCombo("Output Device", audioEngine->GetOutputDeviceList()[outputDevice].data()))
		{
			for (auto const& device : audioEngine->GetOutputDeviceList()) {
				const bool is_selected = (outputDevice == device.first);
				if (ImGui::Selectable(device.second.c_str(), is_selected)) {
					outputDevice = device.first;
				}

				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		return CSC8508::PushdownState::PushdownResult::NoChange;
		};


	audioSlidersUI->PushSliderElement("Master Volume", masterVolume, 100, 0, masterFunc);
	audioSlidersUI->PushSliderElement("Music Volume", musicVolume, 100, 0, musicFunc);
	audioSlidersUI->PushSliderElement("SFX Volume", sfxVolume, 100, 0, sfxFunc);
	audioSlidersUI->PushSliderElement("Chat Volume", voiceVolume, 100, 0, voiceFunc);
	audioSlidersUI->PushVoidElement(deviceFunc);
}

AudioSliders::~AudioSliders() {
	delete audioSlidersUI;
}