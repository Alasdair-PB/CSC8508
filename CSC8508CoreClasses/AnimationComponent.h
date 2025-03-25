#pragma once
//
// Contributors: David
//

#ifndef ANIMATIONCOMPONENT_H
#define ANIMATIONCOMPONENT_H

#include "Mesh.h"
#include "MeshAnimation.h"
#include "IComponent.h"
#include <memory>

using std::vector;

namespace NCL::CSC8508
{
	class RenderObject;

	/**
		Struct holding animation data
	*/
	struct AnimationData {
		std::shared_ptr<Rendering::MeshAnimation> anim;
		int currentAnimFrame = 0.0f;
		float animTime = 0;
	};

	class AnimationComponent : public IComponent
	{
	public:
		AnimationComponent(GameObject& gameObject, Rendering::MeshAnimation* anim = nullptr);
		~AnimationComponent();
		
		static const char* Name() { return "Animation"; }
		const char* GetName() const override { return Name(); }

		std::vector<Matrix4>& GetSkeleton() {
			return skeleton;
		}

		void SetAnimation(Rendering::MeshAnimation* inAnim);
		void UpdateAnimation(float dt);

		void Update(float dt) override {
			UpdateAnimation(dt);
		}

	protected:
		AnimationData animData;
		std::vector<Matrix4> skeleton;
		RenderObject* ro;
	};
}
#endif //ANIMATIONCOMPONENT_H