#include "Kitten.h"
using namespace NCL;
using namespace CSC8508;


Kitten::Kitten(NavigationMesh* navMesh, GameObject* swarm) : NavMeshAgent(navMesh)
{
    sequence = new BehaviourSequence("Kitten Sequence");
    sequence->AddChild(idle);
    sequence->AddChild(goToSwarm);
    sequence->AddChild(followSwarm);
    speed = 7.5f;
    swarmCenter = swarm;
    selected = false;

    state = Ongoing;
    sequence->Reset();
}

Kitten::~Kitten() {
    delete sequence;
}

void Kitten::ThrowSelf(Vector3 dir) 
{
    auto transform = this->GetTransform();
    auto pos = transform.GetPosition();
    pos.y += 3.0f;   
    
    selected = false;

    this->GetTransform().SetPosition(pos);
    dir.y += 1.0f;
    this->GetPhysicsObject()->AddForce(dir * 80.0f);

}


