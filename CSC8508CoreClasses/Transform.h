#pragma once
#include <vector>
#include "Maths.h"

using std::vector;

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8508 {
		class Transform
		{
		public:
			Transform();
			~Transform();

			Transform& SetPosition(const Vector3& worldPos);
			Transform& SetScale(const Vector3& worldScale);
			Transform& SetOrientation(const Quaternion& newOr);

			Vector3 GetPosition() const;
			Vector3 GetScale() const;			
			Quaternion GetOrientation() const;
			Matrix4 GetMatrix() const;

			Vector3 GetLocalPosition() const { return position;}
			Vector3 GetLocalScale() const { return scale;}
			Quaternion GetLocalOrientation() const { return orientation;}
			Matrix4 GetLocalMatrix() const { return matrix; }

			void UpdateMatrix();
			void SetParent(Transform* parentTransform);

		protected:
			Matrix4		matrix;
			Quaternion	orientation;
			Vector3		position;
			Vector3		scale;
			Transform* parentTransform;
		};
	}
}

