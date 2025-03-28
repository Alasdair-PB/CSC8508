/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Controller.h"
#include "SaveManager.h"

void Controller::MapAxis(uint32_t axis, const std::string& name)  {
	axisMappings.insert({name, axis});
}

void Controller::MapButton(uint32_t axis, const std::string& name) {
	buttonMappings.insert({ name, axis });
}

void Controller::MapButtonAnalogue(uint32_t axis, const std::string& name) {
	analogueMappings.insert({ name, axis });
}

float Controller::GetNamedAxis(const std::string& axis) const {
	auto a = axisMappings.find(axis);
	if (a != axisMappings.end()) {
		return GetAxis(a->second);
	}
	return 0.0f;
}

float Controller::GetNamedButtonAnalogue(const std::string& button) const {
	auto a = analogueMappings.find(button);
	if (a != analogueMappings.end()) {
		return GetButtonAnalogue(a->second);
	}
	return 0.0f;
}

bool Controller::GetNamedButton(const std::string& button) const {
	auto a = buttonMappings.find(button);
	if (a != buttonMappings.end()) {
		return GetButton(a->second);
	}
	return false;
}

uint32_t Controller::GetNamedAxisBinding(const std::string& axis) const {
	auto a = axisMappings.find(axis);
	if (a != axisMappings.end()) {
		return a->second;
	}
	return -1;
}

vector<uint32_t> Controller::GetBoundButtons() const {
	vector<uint32_t> boundButtons;
	for (auto binding : buttonMappings)
		boundButtons.push_back(binding.second);
	return boundButtons;
}

vector<uint32_t> Controller::GetBoundAnalogue() const {
	vector<uint32_t> boundAnalogue;
	for (auto binding : analogueMappings)
		boundAnalogue.push_back(binding.second);
	return boundAnalogue;
}

vector<uint32_t> Controller::GetBoundAxis() const {
	vector<uint32_t> boundAxis;
	for (auto binding : axisMappings)
		boundAxis.push_back(binding.second);
	return boundAxis;
}

uint32_t Controller::GetHashId(std::string str) {
	return NCL::CSC8508::SaveManager::MurmurHash3_64(str.c_str(), str.size());
}

uint32_t Controller::GetAxisFromHashId(const uint32_t key) const {
	auto a = axisHashMaps.find(key);
	if (a != axisHashMaps.end())
		return a->second;
	return 0.0f;
}

uint32_t Controller::GetButtonFromHashId(const uint32_t key) const {
	auto a = buttonHashMaps.find(key);
	if (a != buttonHashMaps.end())
		return a->second;
	return 0.0f;
}

uint32_t Controller::GetAnalogueFromHashId(const uint32_t key) const {
	auto a = analogueHashMaps.find(key);
	if (a != analogueHashMaps.end())
		return a->second;
	return 0.0f;
}

uint32_t Controller::GetAxisHashId(const std::string& name) const {
	auto a = axisMappings.find(name);
	if (a != axisMappings.end())
		return GetAxisHashId(a->second);
	return 0.0f;
}

uint32_t Controller::GetButtonHashId(const std::string name) const {
	auto a = buttonMappings.find(name);
	if (a != buttonMappings.end())
		return GetButtonHashId(a->second);
	return 0.0f;
}

uint32_t Controller::GetAnalogueHashId(const std::string name) const {
	auto a = axisMappings.find(name);
	if (a != axisMappings.end())
		return GetAxisHashId(a->second);
	return 0.0f;
}

uint32_t Controller::GetAxisHashId(const uint32_t key) const {
	uint32_t foundKey = 0;
	for (const auto& pair : axisHashMaps) {
		if (pair.second == key) {
			foundKey = pair.first;
			break;
		}
	}
	return foundKey;
}
uint32_t Controller::GetButtonHashId(const uint32_t key) const {
	uint32_t foundKey = 0;
	for (const auto& pair : buttonHashMaps) {
		if (pair.second == key) {
			foundKey = pair.first;
			break;
		}
	}
	return foundKey;
}
uint32_t Controller::GetAnalogueHashId(const uint32_t key) const {
	uint32_t foundKey = 0;
	for (const auto& pair : analogueHashMaps) {
		if (pair.second == key) {
			foundKey = pair.first;
			break;
		}
	}
	return foundKey;
}

void Controller::BindMappingsToHashIds() {
	for (std::pair<std::string, uint32_t> binding : analogueMappings)
		analogueHashMaps[GetHashId(binding.first)] = binding.second;
	for (std::pair<std::string, uint32_t> binding : buttonMappings)
		buttonHashMaps[GetHashId(binding.first)] = binding.second;
	for (std::pair<std::string, uint32_t> binding : axisMappings)
		axisHashMaps[GetHashId(binding.first)] = binding.second;
}