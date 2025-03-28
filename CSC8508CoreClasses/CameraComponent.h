//
// Contributors: Alasdair
//

#ifndef CAMERACOMPONENT_H
#define CAMERACOMPONENT_H

#include "Transform.h"
#include "IComponent.h"
#include "Camera.h"
#include "InputComponent.h"

using std::vector;

namespace NCL::CSC8508
{
	class CameraComponent : public IComponent
	{
	public:
		CameraComponent(GameObject& gameObject, Camera& camera, InputComponent& inputComponent) : 
			IComponent(gameObject), camera(camera), inputComponent(inputComponent), transform(gameObject.GetTransform()){}
		~CameraComponent() = default;

		void OnAwake() override {}

		void Update(float deltaTime) override
		{
			Vector3 dir = inputComponent.GetMouseGameWorldYawMatrix() * Vector3(0, 0, -1);
			Vector3 nextPos = transform.GetPosition();
			Vector3 currentPos = camera.GetPosition();

			nextPos -= dir * forwardOffset;
			nextPos.y += yOffset;

			LerpPosition(currentPos, nextPos);
			camera.SetPosition(nextPos);
			camera.SetYaw(inputComponent.GetMouseGameWorldYaw());
			camera.SetPitch(inputComponent.GetMouseGameWorldPitch());
		}

	protected:
		Camera& camera;
		Transform& transform;
		InputComponent& inputComponent;			
		const float yOffset = 3;
		const float forwardOffset = 10;
		const float lerpSpeed = 0.1f;

		void LerpPosition(Vector3& currentPos, Vector3& nextPos) {
			nextPos.x = std::lerp(currentPos.x, nextPos.x, lerpSpeed);
			nextPos.y = std::lerp(currentPos.y, nextPos.y, lerpSpeed);
			nextPos.z = std::lerp(currentPos.z, nextPos.z, lerpSpeed);
		}
	};
}

#endif //CAMERACOMPONENT_H
