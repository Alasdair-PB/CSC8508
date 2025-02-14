//
// Contributors: Max Bolton
//

#pragma once

#include "AudioObject.h"
#include <string>
#include "Maths.h"
#include "Debug.h"
#include <Transform.h>

using namespace NCL;
using namespace NCL::Maths;
using namespace NCL::CSC8508;

/**
* Listener class for audio engine
*/
class AudioListenerComponent : public AudioObject
{
public:


	AudioListenerComponent(GameObject& gameObject) : AudioObject(gameObject) {

	}

	/*
	* Update position vectors of listener for use by FMOD
	*/
	void Update(float deltaTime) override {
		Vector3 pos = transform->GetPosition();
		Vector3 forward = transform->GetOrientation() * Vector3(0, 0, -1);
		Vector3 up = transform->GetOrientation() * Vector3(0, 1, 0);


		fPosition = VecToFMOD(pos);
		fForward = VecToFMOD(forward);
		fUp = VecToFMOD(up);

		fSystem ? fSystem->set3DListenerAttributes(fIndex, &fPosition, &fVelocity, &fForward, &fUp) : 0;

		if (debug) {
			Debug::Print("Listener Pos: " + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z), Vector2(5, 5));
		}

	}

	

private:

	// Listner id - always 0
	int fIndex = 0;

	FMOD_VECTOR fForward;
	FMOD_VECTOR fUp;

};