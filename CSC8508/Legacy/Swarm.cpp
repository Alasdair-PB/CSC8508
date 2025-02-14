#include "Swarm.h"
using namespace NCL;
using namespace CSC8508;


Swarm::Swarm(NavigationMesh* navMesh) : NavMeshAgent(navMesh)
{
    //sequence = new BehaviourSequence("Swarm Sequence");
    //sequence->AddChild(chase);
    objects = vector<Kitten*>();
    //state = Ongoing;
    speed = 5.0f;
    //sequence->Reset();
}

Swarm::~Swarm() {
    delete sequence;
}

Vector3 lastPos;

void ReduceVelocityOnStop(float roundingPrecision, Vector3 currentPos, vector<Kitten*> objects) {
    
    auto roundToDP = [](float value, int dp) {
        float factor = std::pow(10.0f, dp);
        return std::round(value * factor) / factor;
        };

    if (roundToDP(lastPos.x, roundingPrecision) == roundToDP(currentPos.x, roundingPrecision) &&
        roundToDP(lastPos.y, roundingPrecision) == roundToDP(currentPos.y, roundingPrecision) &&
        roundToDP(lastPos.z, roundingPrecision) == roundToDP(currentPos.z, roundingPrecision)) {

        for (auto obj : objects) {
            if (obj) {
                auto physicsObject = obj->GetPhysicsObject();
                Vector3 velocity = physicsObject->GetLinearVelocity();

                Vector3 dampingForce = -velocity * 0.9f;
                physicsObject->AddForce(dampingForce);
            }
        }
        return;
    }
}

void Swarm::MoveObjectsAlongSwarm()
{
    Vector3 currentPos = this->GetTransform().GetPosition();

    for (auto obj : objects) {
        if (obj) {

            if (!obj->GetYearnsForSwarm() || !obj->GetSelected())
                continue;

            Vector3 v1 = rule1(obj, objects);
            Vector3 v2 = rule2(obj, objects);
            Vector3 v3 = rule3(obj, objects);

            Vector3 combinedForce = Vector::Normalise(v1 * ruleConfig.rule1Weight + v2 * ruleConfig.rule2Weight + v3 * ruleConfig.rule3Weight);            
            auto physObj = obj->GetPhysicsObject();

            physObj->AddForce(combinedForce * ruleConfig.forceMultiplier);
            auto pos = obj->GetTransform().GetPosition();

            auto dir = currentPos - pos;
            auto force = Vector::Normalise(dir);
            force.y = 0;

            if (Vector::Length(dir) > ruleConfig.maxDistanceToCenter && Vector::Length(physObj->GetForce()) < 1.5f)
                physObj->AddForce(force * 5.0f);

            physObj->RotateTowardsVelocity();
        }
    }
}

Vector3 GetCenter(Kitten*& b, std::vector<Kitten*>& boids, float minDis, int& count) {
    Vector3 perceived_center = Vector3(0,0,0);

    for (auto& other : boids) {

        if (!other->GetYearnsForSwarm() || !other->GetSelected())
            continue;

        auto otherPos = other->GetTransform().GetPosition();
        if (&other != &b && (Vector::Length(b->GetTransform().GetPosition() - otherPos) < minDis)) {
            perceived_center += otherPos;
            count++;
        }
    }    
    
    if (count > 0)
        perceived_center /= static_cast<float>(count);
    return perceived_center;
}

// Hard coding center as we want the swarm to always follow this object
Vector3 Swarm::rule1(Kitten*& b, std::vector<Kitten*>& boids)
{
    Vector3 perceived_center = this->GetTransform().GetPosition();
    return (perceived_center - b->GetTransform().GetPosition());
}

Vector3 Swarm::rule2(Kitten*& b, std::vector<Kitten*>& boids) {
    Vector3 c = Vector3(0, 0, 0);
    for (auto& other : boids) {
        if (!other->GetYearnsForSwarm() || !other->GetSelected())
            continue;
        if (&other != &b && Vector::Length(b->GetTransform().GetPosition() - other->GetTransform().GetPosition()) < ruleConfig.minDistanceRule2) {
            c -= (other->GetTransform().GetPosition() - b->GetTransform().GetPosition()); 
        }
    }
    c.y = 0;
    return c;
}

Vector3 Swarm::rule3(Kitten*& b, std::vector<Kitten*>& boids) {
    Vector3 perceived_velocity(0, 0, 0);
    int count = 0;
    for (auto& other : boids) {
        if (!other->GetYearnsForSwarm() || !other->GetSelected())
            continue;
        if (&other != &b && Vector::Length(b->GetTransform().GetPosition() - other->GetTransform().GetPosition()) < ruleConfig.minDistanceRule3) {
            perceived_velocity += other->GetPhysicsObject()->GetLinearVelocity();
            count++;
        }
    }

    if (count > 0) {
        perceived_velocity /= static_cast<float>(count);
        perceived_velocity.y = 0;
        return (perceived_velocity - b->GetTransform().GetPosition()) / 8.0f;
    }
    return Vector3(0, 0, 0);
}

