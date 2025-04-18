//
// Original author: Rich Davison
// Contributors: Alasdair, Alfie
//

#pragma once
#include <vector>
using namespace NCL::Maths;

namespace NCL::CSC8508 {
	class Transform {
	public:
		Transform();

		~Transform() = default;

		/// <summary> Sets the local position of this Transform </summary>
		/// <param name="newPos">The new local Position</param>
		/// <returns>A pointer to this Transform</returns>
		Transform & SetPosition(const Vector3& newPos);

		/// <summary> Sets the local scale of this Transform </summary>
		/// <param name="newScale">The new local Scale</param>
		/// <returns>A pointer to this Transform</returns>
		Transform & SetScale(const Vector3& newScale);

		/// <summary> Sets the local Orientation of this Transform </summary>
		/// <param name="newOr">The new local Orientation </param>
		/// <returns>A pointer to this Transform</returns>
		Transform & SetOrientation(const Quaternion& newOr);

		/// <summary> Get the gloabl position of this Transform </summary>
		/// <returns> The global position as a Vector3</returns>
		[[nodiscard]] Vector3 GetPosition() const;

		/// <summary> Get the gloabl position of this Transform </summary>
		/// <returns> The global position as a Vector3</returns>
		[[nodiscard]] Vector3 GetScale() const;

		/// <summary> Get the gloabl orientation of this Transform </summary>
		/// <returns> The global orientation as a Quaternion</returns>
		[[nodiscard]] Quaternion GetOrientation() const;

		/// <summary> Get the gloabl matrix of this Transform </summary>
		/// <returns> The global matrix as a Matrix4</returns>
		[[nodiscard]] Matrix4 GetMatrix() const;

		/// <summary> Get the local position of this Transform </summary>
		/// <returns> The local position as a Vector3</returns>
		[[nodiscard]] Vector3 GetLocalPosition() const { return position; }

		/// <summary> Get the local scale of this Transform </summary>
		/// <returns> The local scale as a Vector3</returns>
		[[nodiscard]] Vector3 GetLocalScale() const { return scale; }

		/// <summary> Get the local orientation of this Transform </summary>
		/// <returns> The local orientation as a Quaternion</returns>
		[[nodiscard]] Quaternion GetLocalOrientation() const { return orientation; }

		/// <summary> Get the local matrix of this Transform </summary>
		/// <returns> The local matrux as a Matrix4</returns>
		[[nodiscard]] Matrix4 GetLocalMatrix() const { return matrix; }

		void SetParent(Transform* parentTransform);

	protected:
		Matrix4 matrix;
		Quaternion orientation;
		Vector3 position;
		Vector3 scale;
		Transform* parentTransform;
	};
}


