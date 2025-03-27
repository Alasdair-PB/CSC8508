#include "Transform.h"

using namespace NCL::CSC8508;

Transform::Transform()	{
	scale = Vector3(1, 1, 1);
	parentTransform = nullptr;
}

Transform::~Transform()	{

}

void Transform::SetParent(Transform* parentTransform) {
	this->parentTransform = parentTransform;
}

void Transform::UpdateMatrix() {
	matrix =
		Matrix::Translation(position) *
		Quaternion::RotationMatrix<Matrix4>(orientation) *
		Matrix::Scale(scale);
	if (parentTransform != nullptr) {
		matrix =
			Matrix::Translation(parentTransform->GetPosition() + position) *
			Quaternion::RotationMatrix<Matrix4>(parentTransform->GetOrientation() * orientation) *
			Matrix::Scale(parentTransform->GetScale() * scale);
	}
}

Transform& Transform::SetPosition(const Vector3& worldPos) {
	position = worldPos;
	UpdateMatrix();
	return *this;
}

Transform& Transform::SetScale(const Vector3& worldScale) {
	scale = worldScale;
	UpdateMatrix();
	return *this;
}

Transform& Transform::SetOrientation(const Quaternion& worldOrientation) {
	orientation = worldOrientation;
	UpdateMatrix();
	return *this;
}

Vector3 Transform::GetPosition() const {
	//return parentTransform == nullptr ? position : CalculateParentOffset();
	//return parentTransform == nullptr ? position : position + parentTransform->GetPosition();
	return parentTransform == nullptr ? position : parentTransform->GetOrientation() * (parentTransform->GetScale() * position)  + parentTransform->GetPosition();
}

Vector3 Transform::GetScale() const {
	return parentTransform == nullptr ? scale : parentTransform->GetScale() * scale;
}

Quaternion Transform::GetOrientation() const {
	return parentTransform == nullptr ? orientation : parentTransform->GetOrientation() * orientation;
}

Matrix4 Transform::GetMatrix() const {
	return parentTransform == nullptr ? matrix : Matrix::Translation(parentTransform->GetPosition() + position) *
		Quaternion::RotationMatrix<Matrix4>(parentTransform->GetOrientation() * orientation) *
		Matrix::Scale(parentTransform->GetScale() * scale);
}

