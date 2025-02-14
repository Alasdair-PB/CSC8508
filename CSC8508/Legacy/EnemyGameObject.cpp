#include "EnemyGameObject.h"


using namespace NCL;
using namespace CSC8508;


EnemyGameObject::EnemyGameObject(NavigationMesh* navMesh) : NavMeshAgent(navMesh)
{
    sequence = new BehaviourSequence("Path Sequence");

    sequence->AddChild(patrol);
    sequence->AddChild(chase);

    state = Ongoing;
    sequence->Reset();
}

EnemyGameObject::~EnemyGameObject() {
    delete sequence;
}

bool EnemyGameObject::CanSeePlayer()
{
    Vector3 playerPos = getPlayerPos();
    Vector3 enemyPos = this->GetTransform().GetPosition();

    enemyPos.y += yOffSet;
    playerPos.y += yOffSet;

    Vector3 playerToEnemy = (playerPos - enemyPos);
    auto len = Vector::Length(playerToEnemy);
    Ray ray = Ray(enemyPos, playerToEnemy);

    if (rayHit(ray, len))
        return false;

    return true;
}

