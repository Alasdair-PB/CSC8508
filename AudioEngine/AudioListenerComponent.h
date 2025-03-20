//
// Contributors: Max Bolton
//

#pragma once

#include "AudioObject.h"
#include <string>
#include "Maths.h"
#include "Debug.h"
#include <Transform.h>
#include "Camera.h"
#include <opus/opus.h>
#include <fmod.hpp>
#include <fmod_errors.h>
#include <queue>
#include <deque>
#include <algorithm>
#include <vector>
#include <cstring>
#include <iostream>
#include <mutex>

using namespace NCL;
using namespace NCL::Maths;
using namespace NCL::CSC8508;



/**
* Listener class for audio engine
*/
class AudioListenerComponent : public AudioObject
{
public:


	AudioListenerComponent(GameObject& gameObject, PerspectiveCamera& camera);

	~AudioListenerComponent();

	/**
	*
	*/
	void OnAwake() override;

	/**
	* Update position vectors of listener for use by FMOD
	* @param deltaTime
	*/
	void Update(float deltaTime) override;

	void SetCamOrientation();

	void SetPlayerOrientation();


	#pragma region FMOD Recording

	/**
	* Init microphone sound object
	*/
	void InitMicSound();

	/**
	* Start Microphone recording of selected input device
	*/
	FMOD_RESULT RecordMic() {
		return fSystem->recordStart(inputDeviceIndex, micInput, true);

	}

	/**
	* Stop Microphone recording of selected input device
	*/
	void StopRecording() {
		fSystem->recordStop(inputDeviceIndex);
	}

	/**
	* Checks if current input device is recording
	*/
	bool IsRecording();

	/**
	* Toggle recording of microphone input
	*/
	void ToggleRecording();

	/**
	* Get recorded microphone buffer
	* @return FMOD::Sound* (recorded buffer)
	*/
	FMOD::Sound* getMicInput() {
		return micInput;
	}

	#pragma endregion

	#pragma region Encoding VOIP Pipeline

	/**
	* Convert packet to fixed size array
	* @return unsigned char* (array of packet data)
	* @param std::vector<unsigned char>& (packet data)
	* NOTE: Remember to delete the array after use
	*/
	unsigned char* PacketToArray(std::vector<unsigned char>& packet) {
		unsigned char* array = new unsigned char[packet.size()];
		for (size_t i = 0; i < packet.size(); i++) {
			array[i] = packet[i];
		}
		return array;
	}


	#pragma endregion

	#pragma region Decoding VOIP Pipeline


	// OLD Decode Update Loop
	/*
	void UpdateAudioDecode() {
		if (encodedPacketQueue != nullptr && !encodedPacketQueue->empty()) {
			std::vector<unsigned char> packet = encodedPacketQueue->front();
			encodedPacketQueue->pop_front();  // Process oldest frame first
			DecodePersistentPlayback(PacketToArray(packet));
		}
	}*/

	#pragma endregion


	#pragma region Input/Output Device Management
	/**
	* Update input device list
	* Removes output loopback devices
	*/
	void UpdateInputList();

	/**
	* Get List of input devices
	* @return std::map<int, std::string> 
	*/
	std::map<int, std::string> GetInputDeviceList() {
		UpdateInputList();
		return inputDeviceList;
	}

	/**
	* Print input device list
	* [Used for debugging]
	*/
	void PrintInputList();

	/**
	* Update output Device List
	*/
	void UpdateOutputList();

	/**
	* Get List of output devices
	* @return std::map<int, std::string>
	*/

	std::map<int, std::string> GetOutputDeviceList() {
		UpdateOutputList();
		return outputDeviceList;
	}

	/**
	* Print output device list
	* [Used for debugging]
	*/
	void PrintOutputList();

	/**
	* Set input device index
	* Ensures FMOD uses the intended input device
	*/
	void SetInputDeviceIndex(int index) {
		if (IsRecording()) {
			StopRecording();
		}
		inputDeviceIndex = index;
	}


	/**
	* Set output device index
	* Ensures FMOD uses the intended output device
	*/
	void SetOutputDeviceIndex(int index) {
		outputDeviceIndex = index;
	}

	#pragma endregion


	protected:

	// Listner id - always 0
	int fIndex = 0;

	std::map<int, std::string> inputDeviceList;
	int inputDeviceIndex;

	std::map<int, std::string> outputDeviceList;
	int outputDeviceIndex;

	FMOD::Sound* micInput;




	FMOD_VECTOR fForward;
	FMOD_VECTOR fUp;

	PerspectiveCamera* camera;

};