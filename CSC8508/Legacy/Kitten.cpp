#include "Kitten.h"
using namespace NCL;
using namespace CSC8508;


Kitten::Kitten(GameObject* swarm) : GameObject()
{
    sequence = new BehaviourSequence("Kitten Sequence");
    sequence->AddChild(idle);
    sequence->AddChild(goToSwarm);
    sequence->AddChild(followSwarm);
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
   physicsComponent->GetPhysicsObject()->AddForce(dir * 80.0f);

}


