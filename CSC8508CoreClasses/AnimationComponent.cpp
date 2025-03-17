#include "AnimationComponent.h"
#include "RenderObject.h"

using namespace NCL::CSC8508;

void AnimationComponent::SetAnimation(Rendering::MeshAnimation& inAnim) {
	animData.anim = &inAnim;
	skeleton.resize(animData.anim->GetJointCount());
}

void AnimationComponent::UpdateAnimation(float dt) {
	AnimationData& ad = animData;
	if (!ro || !ad.anim) {
		std::cerr << "No render object or animation set!" << std::endl;
		return;
	}
	
	ad.animTime -= dt;

	if (ad.animTime <= 0) {
		ad.currentAnimFrame++;
		ad.animTime += ad.anim->GetFrameTime();
		ad.currentAnimFrame = (ad.currentAnimFrame++) % ad.anim->GetFrameCount();

		std::vector<Matrix4>const& inverseBindPose = ro->GetMesh()->GetInverseBindPose();

		if (inverseBindPose.size() != ad.anim->GetJointCount()) {
			std::cerr << "Animation skeleton does not match bind pose!" << std::endl;
			return;
		}

		const Matrix4* joints = ad.anim->GetJointData(ad.currentAnimFrame);

		for (int i = 0; i < skeleton.size(); ++i) {
			skeleton[i] = joints[i] * inverseBindPose[i];
		}
	}
}
