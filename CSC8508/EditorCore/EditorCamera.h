#ifndef EDITORCAM_H
#define EDITORCAM_H

#include "GameObject.h"
#include "camera.h"

using namespace NCL;
using namespace CSC8508;

class EditorCamera {
public:
	EditorCamera(Camera* camera, Controller* controller) : 
		controller(controller), camera(camera) {
		panBinding = controller->GetNamedButtonBinding("Pan");
	}
	~EditorCamera() = default;

	void UpdateMouseGameWorldPitchYaw() {
		if (!controller->GetBoundButton(panBinding))
			return;

		mousePitch -= controller->GetNamedAxis("YLook") * sensitivity;
		mouseYaw -= controller->GetNamedAxis("XLook") * sensitivity;

		mousePitch = std::min(mousePitch, 90.0f);
		mousePitch = std::max(mousePitch, -90.0f);

		if (mouseYaw < 0)
			mouseYaw += 360.0f;
		if (mouseYaw > 360.0f)
			mouseYaw -= 360.0f;
	}

	void Update(float deltaTime) {
		float x = controller->GetNamedAxis("Sidestep");
		float y = controller->GetNamedAxis("Forward");
		float z = controller->GetNamedAxis("Up");

		UpdateMouseGameWorldPitchYaw();
		camera->SetYaw(mouseYaw);
		camera->SetPitch(mousePitch);

		Vector3 nextpos = camera->GetPosition();

		Vector3 dir;
		Matrix3 yawRotation = Matrix::RotationMatrix3x3(mouseYaw, Vector3(0, 1, 0));

		dir += yawRotation * Vector3(0, 0, -y);
		dir += yawRotation * Vector3(x, 0, 0);

		Matrix3 offsetRotation = Matrix::RotationMatrix3x3(0.0f, Vector3(0, 1, 0));
		dir = offsetRotation * dir;		
		nextpos += dir * deltaTime * cameraSpeed;
		nextpos.y += z * deltaTime * cameraSpeed;
		camera->SetPosition(nextpos);
	}

protected:
	Controller* controller;
	Camera* camera;

	uint32_t panBinding;
	float cameraSpeed = 10.0f;
	float mouseYaw = 0;
	float sensitivity = 0.5f;
	float mousePitch = 0;

};


#endif // EDITORCAM_H
