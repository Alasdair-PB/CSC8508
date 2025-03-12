#include "EnemyComponent.h"


using namespace NCL;
using namespace CSC8508;


EnemyComponent::EnemyComponent(GameObject& gameObject, NavigationMesh* navMesh) : IComponent(gameObject)
{
    playerVisible = false;
    sequence = new BehaviourSequence("Path Sequence");

    sequence->AddChild(patrol);
    sequence->AddChild(chase);

    state = Ongoing;
    sequence->Reset();
}

EnemyComponent::~EnemyComponent() {
    delete sequence;
}

bool EnemyComponent::CanSeePlayer()
{
    Vector3 playerPos = getPlayerPos();
    Vector3 enemyPos = GetGameObject().GetTransform().GetPosition();

    enemyPos.y += yOffSet;
    playerPos.y += yOffSet;

    Vector3 playerToEnemy = (playerPos - enemyPos);
    auto len = Vector::Length(playerToEnemy);
    Ray ray = Ray(enemyPos, playerToEnemy);

    if (rayHit(ray, len))
        return false;

    return true;
}

