/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/

Additional contributions: Alfie
*/
#include "Quaternion.h"
#include "Maths.h"

using namespace NCL;
using namespace NCL::Maths;

Quaternion::Quaternion(void)
{
	x = y = z = 0.0f;
	w = 1.0f;
}

Quaternion::Quaternion(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

Quaternion::Quaternion(double x, double y, double z, double w)
{
	this->x = (float)x;
	this->y = (float)y;
	this->z = (float)z;
	this->w = (float)w;
}

Quaternion::Quaternion(const Vector3& vector, float w) {
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
	this->w = w;
}


Quaternion::Quaternion(const Matrix4 &m) {
	w = sqrt(std::max(0.0f, (1.0f + m.array[0][0] + m.array[1][1] + m.array[2][2])))  * 0.5f;

	if (abs(w) < 0.0001f) {
		x = sqrt(std::max(0.0f, (1.0f + m.array[0][0] - m.array[1][1] - m.array[2][2]))) / 2.0f;
		y = sqrt(std::max(0.0f, (1.0f - m.array[0][0] + m.array[1][1] - m.array[2][2]))) / 2.0f;
		z = sqrt(std::max(0.0f, (1.0f - m.array[0][0] - m.array[1][1] + m.array[2][2]))) / 2.0f;

		x = (float)copysign(x, m.array[2][1] - m.array[1][2]);
		y = (float)copysign(y, m.array[0][2] - m.array[2][0]);
		z = (float)copysign(z, m.array[1][0] - m.array[0][1]);
	}
	else {
		float qrFour = 4.0f * w;
		float qrFourRecip = 1.0f / qrFour;

		x = (m.array[1][2] - m.array[2][1]) * qrFourRecip;
		y = (m.array[2][0] - m.array[0][2]) * qrFourRecip;
		z = (m.array[0][1] - m.array[1][0]) * qrFourRecip;
	}
}

Quaternion::Quaternion(const Matrix3& m) {
	w = sqrt(std::max(0.0f, (1.0f + m.array[0][0] + m.array[1][1] + m.array[2][2]))) * 0.5f;

	if (abs(w) < 0.0001f) {
		x = sqrt(std::max(0.0f, (1.0f + m.array[0][0] - m.array[1][1] - m.array[2][2]))) / 2.0f;
		y = sqrt(std::max(0.0f, (1.0f - m.array[0][0] + m.array[1][1] - m.array[2][2]))) / 2.0f;
		z = sqrt(std::max(0.0f, (1.0f - m.array[0][0] - m.array[1][1] + m.array[2][2]))) / 2.0f;

		x = (float)copysign(x, m.array[2][1] - m.array[1][2]);
		y = (float)copysign(y, m.array[0][2] - m.array[2][0]);
		z = (float)copysign(z, m.array[1][0] - m.array[0][1]);
	}
	else {
		float qrFour = 4.0f * w;
		float qrFourRecip = 1.0f / qrFour;

		x = (m.array[1][2] - m.array[2][1]) * qrFourRecip;
		y = (m.array[2][0] - m.array[0][2]) * qrFourRecip;
		z = (m.array[0][1] - m.array[1][0]) * qrFourRecip;
	}
}

float Quaternion::Dot(const Quaternion &a,const Quaternion &b){
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

void Quaternion::Normalise(){
	float magnitude = sqrt(x*x + y*y + z*z + w*w);

	if(magnitude > 0.0f){
		float t = 1.0f / magnitude;

		x *= t;
		y *= t;
		z *= t;
		w *= t;
	}
}

Quaternion Quaternion::Normalised() const {
	Quaternion temp(*this);
	temp.Normalise();
	return temp;
}

void Quaternion::CalculateW()	{
	w = 1.0f - (x*x)-(y*y)-(z*z);
	if(w < 0.0f) {
		w = 0.0f;
	}
	else{
		w = - sqrt(w);
	}
}

Quaternion Quaternion::Conjugate() const
{
	return Quaternion(-x,-y,-z,w);
}

Quaternion Quaternion::Lerp(const Quaternion &from, const Quaternion &to, float by) {
	Quaternion temp = to;

	float dot = Quaternion::Dot(from,to);

	if(dot < 0.0f) {
		temp = -to;
	}

	return (from * (1.0f - by)) + (temp * by);
}
//SIGGRAPH Shoemake
Quaternion Quaternion::Slerp(const Quaternion &from, const Quaternion &to, float by) {
	float t = by;

	float dot = std::clamp(Quaternion::Dot(from,to), -1.0f, 1.0f);

	if (dot == 1.0f) {
		return from;
	}

	float theta = std::abs(acos(dot));

	float aScale = sin((1 - t) * theta);
	float bScale = sin(t * theta);

	Quaternion q = (from * aScale) + (to * bScale);

	q *= 1.0f / sin(theta);

	q.Normalise();
	return q;
}

//http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
//Verified! Different values to above, due to difference between x/z being 'forward'
Vector3 Quaternion::ToEuler() const {
	Vector3 euler;

	float t = x*y + z*w;

	if (t > 0.4999) {
		euler.z = Maths::RadiansToDegrees(Maths::PI / 2.0f);
		euler.y = Maths::RadiansToDegrees(2.0f * atan2(x, w));
		euler.x = 0.0f;

		return euler;
	}

	if (t < -0.4999) {
		euler.z = -Maths::RadiansToDegrees(Maths::PI / 2.0f);
		euler.y = -Maths::RadiansToDegrees(2.0f * atan2(x, w));
		euler.x = 0.0f;
		return euler;
	}

	float sqx = x*x;
	float sqy = y*y;
	float sqz = z*z;

	euler.z = Maths::RadiansToDegrees(asin(2 * t));
	euler.y = Maths::RadiansToDegrees(atan2(2 * y*w - 2 * x*z, 1.0f - 2 * sqy - 2 * sqz));
	euler.x = Maths::RadiansToDegrees(atan2(2 * x*w - 2 * y*z, 1.0f - 2 * sqx - 2.0f*sqz));

	return euler;
}

//http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/
//VERIFIED AS CORRECT - Pitch and roll are changed around as the above uses x as 'forward', whereas we use -z
Quaternion Quaternion::EulerAnglesToQuaternion(float roll, float yaw, float pitch) {
	float cos1 = (float)cos(Maths::DegreesToRadians(yaw   * 0.5f));
	float cos2 = (float)cos(Maths::DegreesToRadians(pitch * 0.5f));
	float cos3 = (float)cos(Maths::DegreesToRadians(roll  * 0.5f));

	float sin1 = (float)sin(Maths::DegreesToRadians(yaw   * 0.5f));
	float sin2 = (float)sin(Maths::DegreesToRadians(pitch * 0.5f));
	float sin3 = (float)sin(Maths::DegreesToRadians(roll  * 0.5f));

	Quaternion q;

	q.x = (sin1 * sin2 * cos3) + (cos1 * cos2 * sin3);
	q.y = (sin1 * cos2 * cos3) + (cos1 * sin2 * sin3);
	q.z = (cos1 * sin2 * cos3) - (sin1 * cos2 * sin3);
	q.w = (cos1 * cos2 * cos3) - (sin1 * sin2 * sin3);

	return q;
};

Quaternion Quaternion::AxisAngleToQuaterion(const Vector3& vector, float degrees) {
	float theta		= (float)Maths::DegreesToRadians(degrees);
	float result	= (float)sin(theta / 2.0f);

	return Quaternion((float)(vector.x * result), (float)(vector.y * result), (float)(vector.z * result), (float)cos(theta / 2.0f));
}


Quaternion Quaternion::VectorsToQuaternion(Vector3 const& fromVector, Vector3 const& toVector) {

	// 1: Normalise the input vectors:
	Vector3 const from = Vector::Normalise(fromVector);
	Vector3 const to = Vector::Normalise(toVector);

	// 2: Find axis of rotation
	Vector3 axis = Vector::Cross(from, to);

	// 3: Find the angle of rotation
	float const dot = Vector::Dot(from, to);

	// If the orientation is exactly 180 degrees, Quaternions don't work properly so the maths has to be doctored accordingly:
	if (fabs(dot + 1.0f) < FLT_EPSILON)
		return { 0.0f, 1.0f, 0.0f, 0.0f };

	// If orientation is not 180 degrees, get angle of rotation
	float const angle = std::acos(dot); // in radians

	// 4: Calculate and return the quaternion
	float const result	= sin(angle / 2.0f);
	return {
		axis.x * result,
		axis.y * result,
		axis.z * result,
		cos(angle / 2.0f)
	};
}


Vector3		Quaternion::operator *(const Vector3 &a)	const {
	Quaternion newVec = *this * Quaternion(a.x, a.y, a.z, 0.0f) * Conjugate();
	return Vector3(newVec.x, newVec.y, newVec.z);
}