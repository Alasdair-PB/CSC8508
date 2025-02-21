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

	/**
	* Update position vectors of listener for use by FMOD
	* @param deltaTime
	*/
	void Update(float deltaTime) override;

	/**
	* Start Microphone recording of selected input device
	*/
	void RecordMic() {
		FMOD_RESULT result = fSystem->recordStart(inputDeviceIndex, micInput, true);
		std::cout << "Recording Result: " << result << std::endl;
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

private:

	// Listner id - always 0
	int fIndex = 0;

	FMOD_VECTOR fForward;
	FMOD_VECTOR fUp;

	FMOD::Sound* micInput;

	std::map<int, std::string> inputDeviceList;
	int inputDeviceIndex;

	std::map<int, std::string> outputDeviceList;
	int outputDeviceIndex;

	PerspectiveCamera* camera;

};