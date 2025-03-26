#include "AnimationComponent.h"
#include "MeshAnimation.h"
#include "RenderObject.h"

using namespace NCL::CSC8508;

AnimationComponent::AnimationComponent(GameObject& gameObject) : StateComponent(gameObject) {
	activeState = nullptr;
	ro = gameObject.GetRenderObject();
	if (!ro) {
		std::cerr << "No render object set!" << std::endl;
	}
}

AnimationComponent::~AnimationComponent() {
	for (auto& i : allStates) {
		delete i;
	}
	for (auto& i : allTransitions) {
		delete i.second;
	}
}

void AnimationComponent::SetAnimation(AnimState* anim) {
	activeState = anim;
	resetTime();
}

void AnimationComponent::UpdateAnimation(float dt) {
	if (!ro || !activeState) {
		std::cerr << "No render object or animation set!" << std::endl;
		return;
	}

	MeshAnimation* anim = static_cast<AnimState*>(activeState)->GetAnimation();
	animTime -= dt;

	if (animTime <= 0) {
		currentAnimFrame++;
		animTime += 
		currentAnimFrame = (currentAnimFrame++) % anim->GetFrameCount();
		// handle loop end here

		std::vector<Matrix4>const& inverseBindPose = ro->GetMesh()->GetInverseBindPose();

		if (inverseBindPose.size() != anim->GetJointCount()) {
			std::cerr << "Animation skeleton does not match bind pose!" << std::endl;
			return;
		}

		const Matrix4* joints = anim->GetJointData(currentAnimFrame);

		for (int i = 0; i < skeleton.size(); ++i) {
			skeleton[i] = joints[i] * inverseBindPose[i];
		}
	}
}

void AnimationComponent::TriggerAnimation(const std::string& triggerName) {


}