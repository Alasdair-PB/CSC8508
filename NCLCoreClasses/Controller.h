/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

#include <map>
using std::vector;

class Controller {
public:
	Controller() {}
	virtual ~Controller() {}

	virtual float GetAxis(uint32_t axis) const = 0;
	virtual float GetButtonAnalogue(uint32_t button) const = 0;
	virtual bool GetButton(uint32_t button) const = 0;

	virtual float GetNamedAxis(const std::string& axis) const;
	virtual float GetNamedButtonAnalogue(const std::string& button) const;
	virtual bool GetNamedButton(const std::string& button) const;

	void MapAxis(uint32_t axis, const std::string& name);
	void MapButton(uint32_t axis, const std::string& name);
	void MapButtonAnalogue(uint32_t axis, const std::string& name);

	virtual void Update(float dt = 0.0f) {}
	uint32_t GetNamedAxisBinding(const std::string& name) const;
	virtual bool GetBoundButton(uint32_t keyId) const { return false; }

	vector<uint32_t> GetBoundButtons() const;
	vector<uint32_t> GetBoundAnalogue() const;
	vector<uint32_t> GetBoundAxis() const;

	uint32_t GetHashId(std::string str);

	uint32_t GetAxisFromHashId(const uint32_t key) const;
	uint32_t GetButtonFromHashId(const uint32_t key) const;
	uint32_t GetAnalogueFromHashId(const uint32_t key) const;

	uint32_t GetAxisHashId(const std::string& name) const;
	uint32_t GetButtonHashId(const std::string name) const;
	uint32_t GetAnalogueHashId(const std::string name) const;

	uint32_t GetAxisHashId(const uint32_t key) const;
	uint32_t GetButtonHashId(const uint32_t key) const;
	uint32_t GetAnalogueHashId(const uint32_t key) const;

	void BindMappingsToHashIds();

protected:
	std::map<std::string, uint32_t> buttonMappings;
	std::map<std::string, uint32_t> analogueMappings;
	std::map<std::string, uint32_t> axisMappings;

	std::map<uint32_t, uint32_t> buttonHashMaps;
	std::map<uint32_t, uint32_t> analogueHashMaps;
	std::map<uint32_t, uint32_t> axisHashMaps;
};