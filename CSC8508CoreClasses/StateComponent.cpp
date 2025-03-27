#include "StateComponent.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8508;

StateComponent::StateComponent(GameObject& gameObject) : IComponent(gameObject)
{

    counter = 0.0f;
    stateMachine = new StateMachine();

    State* stateA = new State([&](float dt) -> void {
        this->MoveLeft(dt);
        });

    State* stateB = new State([&](float dt) -> void {
        this->MoveRight(dt);
        });

    stateMachine->AddState(stateA);
    stateMachine->AddState(stateB);

    stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]() -> bool {
        return this->counter > 3.0f;
        }));

    stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]() -> bool {
        return this->counter < 0.0f;
        }));
}


StateComponent::~StateComponent() {
	delete stateMachine;
}

void StateComponent::Update(float dt) {
    stateMachine -> Update(dt);
}

void StateComponent::MoveLeft(float dt) {
    physics->GetPhysicsObject()->AddForce({ -100, 0, 0 });
    counter += dt;
}

void StateComponent::MoveRight(float dt) {
    physics->GetPhysicsObject()->AddForce({ 100, 0, 0 });
    counter -= dt;
}
