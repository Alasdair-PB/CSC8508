#pragma once
#include "IStateComponent.h"

namespace NCL {
	namespace Rendering {
		class MeshAnimation;
	}
	namespace CSC8508 {
		class AnimState : public IState {
		public:
			AnimState(Rendering::MeshAnimation* anim, bool loop = true) {
				this->anim = std::shared_ptr<Rendering::MeshAnimation>(anim);
				this->loop = loop;
				this->complete = false;
			}
			Rendering::MeshAnimation* GetAnimation() {
				return anim.get();
			}
			bool IsLooped() {
				return loop;
			}
			bool IsComplete() {
				return complete;
			}
			void SetComplete(bool complete = true) {
				this->complete = complete;
			}
		protected:
			std::shared_ptr<Rendering::MeshAnimation> anim;
			bool loop;
			bool complete;
		};

		class AnimationComponent : public IStateComponent {
		public:
			AnimationComponent(GameObject& gameObject);
			~AnimationComponent();

			void SetActiveState(IState* state) override {
				SetAnimation((AnimState*) state);
			}

			virtual std::unordered_set<std::type_index>& GetDerivedTypes() const override {
				static std::unordered_set<std::type_index> derivedTypes = {
					typeid(AnimationComponent),
					typeid(IStateComponent),
					typeid(IComponent)
				};
				return derivedTypes;
			}

			static const char* Name() { return "Animation"; }
			const char* GetName() const override { return Name(); }

			std::vector<Matrix4>& GetSkeleton() {
				return skeleton;
			}

			void SetAnimation(AnimState* anim);
			void TriggerAnimation(const std::string& triggerName);
			void UpdateAnimation(float dt);
			bool IsComplete() { return static_cast<AnimState*>(activeState)->IsComplete(); }
			void resetTime() {
				currentAnimFrame = 0.0f;
				animTime = 0;
			}

			void Update(float dt) override {
				IStateComponent::Update(dt, this->GetGameObject());
				UpdateAnimation(dt);
			}

        protected:
			int currentAnimFrame = 0.0f;
			float animTime = 0;
			std::vector<Matrix4> skeleton;
			RenderObject* ro;
        };
	}
}