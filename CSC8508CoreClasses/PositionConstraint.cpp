#include "PositionConstraint.h"
//#include "../../Common/Vector3.h"
#include "GameObject.h"
#include "PhysicsObject.h"
//#include "Debug.h"



using namespace NCL;
using namespace Maths;
using namespace CSC8508;

PositionConstraint::PositionConstraint(GameObject* a, GameObject* b, float d)
{
	objectA		= a;
	objectB		= b;
	distance	= d;
}

PositionConstraint::~PositionConstraint()
{

}


void PositionConstraint::UpdateConstraint(float dt) {
    Vector3 relativePos = objectA->GetTransform().GetPosition() - objectB->GetTransform().GetPosition();
    float currentDistance = Vector::Length(relativePos);
    float offset = distance - currentDistance;

    if (abs(offset) > 0.0f)
    {
        Vector3 offsetDir = Vector::Normalise(relativePos);
        PhysicsObject* physA = objectA->GetPhysicsObject();
        PhysicsObject* physB = objectB->GetPhysicsObject();
        Vector3 relativeVelocity = physA->GetLinearVelocity() - physB->GetLinearVelocity();

        float constraintMass = physA->GetInverseMass() + physB->GetInverseMass();

        if (constraintMass > 0.0f) {
            float velocityDot = Vector::Dot(relativeVelocity, offsetDir);
            float biasFactor = 0.01f;
            float bias = -(biasFactor / dt) * offset;
            float lambda = -(velocityDot + bias) / constraintMass;

            Vector3 aImpulse = offsetDir * lambda;
            Vector3 bImpulse = -offsetDir * lambda;

            physA->ApplyLinearImpulse(aImpulse);
            physB->ApplyLinearImpulse(bImpulse);
        }
    }
}

