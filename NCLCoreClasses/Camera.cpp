#include "Camera.h"
#include "Window.h"

using namespace NCL;

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