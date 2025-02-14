//
// Contributors: Max Bolton
//

#pragma once

#include <fmod.hpp>
#include "AudioEngine.h"
#include "Maths.h"
#include "Debug.h"
#include <Transform.h>
#include "IComponent.h"
#include "GameObject.h"


using namespace NCL;
using namespace NCL::Maths;
using namespace NCL::CSC8508;


/**
* Base class for all audio objects (listeners, sound sources)
* will eventually inherit from component class for component system
* 
*/
class AudioObject : public IComponent
{

public:
	
	void setDebug(bool debug) {
		this->debug = debug;
	}


protected:
	AudioObject(GameObject& gameObject) : IComponent(gameObject) {

		audioEngine = &AudioEngine::Instance();
		fSystem = audioEngine->GetSystem();

		this->transform = &(gameObject.GetTransform());

		Vector3 zero = Vector3(0, 0, 0);
		fVelocity = VecToFMOD(zero);

	};

	Transform* transform;

	AudioEngine* audioEngine;
	FMOD::System* fSystem;

	FMOD_VECTOR fPosition;
	FMOD_VECTOR fVelocity;

	bool debug = false;

	static FMOD_VECTOR VecToFMOD(const Vector3& vec) {
		FMOD_VECTOR fVec;
		fVec.x = vec.x;
		fVec.y = vec.y;
		fVec.z = vec.z;
		return fVec;
	}


};






