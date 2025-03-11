#include "RenderObject.h"
#include "Mesh.h"

using namespace NCL::CSC8508;
using namespace NCL;

void RenderObject::SetAnimation(MeshAnimation& inAnim) {
	anim = &inAnim;

	skeleton.resize(anim->GetJointCount());
}

void RenderObject::UpdateAnimation(float dt) {
	if (!mesh || !anim) {
		return;
	}
	animTime -= dt;

	if (animTime <= 0) {
		currentAnimFrame++;
		animTime += anim->GetFrameTime();
		currentAnimFrame = (currentAnimFrame++) % anim->GetFrameCount();

		std::vector<Matrix4>const& inverseBindPose = mesh->GetInverseBindPose();

		if (inverseBindPose.size() != anim->GetJointCount()) {
			//oh no
			return;
		}

		const Matrix4* joints = anim->GetJointData(currentAnimFrame);

		for (int i = 0; i < skeleton.size(); ++i) {
			skeleton[i] = joints[i] * inverseBindPose[i];
		}
	}
}