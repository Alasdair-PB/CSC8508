#include "Camera.h"
#include "Window.h"
#include <cmath>

using namespace NCL;

void Camera::UpdateCamera(float dt) {
	if (!activeController) {
		return;
	}

	pitch	-= activeController->GetNamedAxis("YLook");
	yaw		-= activeController->GetNamedAxis("XLook");

	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);

	if (yaw <0) {
		yaw += 360.0f;
	}
	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}
	Vector3 dir = Vector3();	
	Matrix3 yawRotation = Matrix::RotationMatrix3x3(yaw, Vector3(0, 1, 0));	
	dir += yawRotation * Vector3(0, 0, -1);

	auto nextPos = getPlayerPos();	
	nextPos -=  dir * 30.0f;
	nextPos.y += 20;


	float lerpSpeed = 0.1f;
	position.x = std::lerp(position.x, nextPos.x, lerpSpeed);
	position.y = std::lerp(position.y, nextPos.y, lerpSpeed);
	position.z = std::lerp(position.z, nextPos.z, lerpSpeed);
}

Matrix4 Camera::BuildViewMatrix() const {
	return	Matrix::Rotation(-pitch, Vector3(1, 0, 0)) *
		Matrix::Rotation(-yaw, Vector3(0, 1, 0)) *
		Matrix::Translation(-position);
};

Matrix4 PerspectiveCamera::BuildProjectionMatrix(float currentAspect) const {
	return Matrix::Perspective(nearPlane, farPlane, currentAspect, fov);
}

Matrix4 OrhographicCamera::BuildProjectionMatrix(float currentAspect) const {
	return Matrix::Orthographic(left, right, bottom, top, nearPlane, farPlane);
}