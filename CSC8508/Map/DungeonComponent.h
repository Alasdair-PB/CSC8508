//
// Contributors: Alfie
//

#ifndef DUNGEONCOMPONENT_H
#define DUNGEONCOMPONENT_H

#include "IComponent.h"

using namespace NCL::CSC8508;

class DungeonComponent final : public IComponent {
public:
    explicit DungeonComponent(GameObject& gameObject) : IComponent(gameObject) { }

private:
    std::vector<GameObject*> rooms; // TODO: Maybe replace with RoomComponent? Whichever becomes more helpful
};

#endif //DUNGEONCOMPONENT_H
