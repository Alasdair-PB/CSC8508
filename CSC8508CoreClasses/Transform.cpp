//
// Original author: Rich Davison
// Contributors: Alasdair, Alfie
//

#include "Transform.h"

using namespace NCL::CSC8508;

Transform::Transform()	{
	scale = Vector3(1, 1, 1);
	parentTransform = nullptr;
}

void Transform::SetParent(Transform* parentTransform) {
	this->parentTransform = parentTransform;
}

Transform& Transform::SetPosition(const Vector3& newPos) {
	position = newPos;
	return *this;
}

Transform& Transform::SetScale(const Vector3& newScale) {
	scale = newScale;
	return *this;
}

Transform& Transform::SetOrientation(const Quaternion& newOr) {
	orientation = newOr;
	return *this;
}

Vector3 Transform::GetPosition() const {
	return parentTransform == nullptr ? position : parentTransform->GetOrientation() * (parentTransform->GetScale() * position)  + parentTransform->GetPosition();
}

Vector3 Transform::GetScale() const {
	return parentTransform == nullptr ? scale : parentTransform->GetScale() * scale;
}

Quaternion Transform::GetOrientation() const {
	return parentTransform == nullptr ? orientation : parentTransform->GetOrientation() * orientation;
}

Matrix4 Transform::GetMatrix() const {
	return Matrix::Translation(GetPosition()) * Quaternion::RotationMatrix<Matrix4>(GetOrientation()) * Matrix::Scale(GetScale());
}

