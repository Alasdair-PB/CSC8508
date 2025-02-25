//
// Contributors: Alfie
//

#ifndef DUALTREENODE_H
#define DUALTREENODE_H

#define MAX_ITEMS 4

#include "GameObject.h"

using namespace NCL::CSC8508;

class DualTreeNode {
public:
    virtual ~DualTreeNode() = default;

    virtual bool Insert(GameObject* object) = 0;

    virtual void Get(Vector3<float> position, Vector3<float> halfDimensions, GameObject* output[MAX_ITEMS]) = 0;

    virtual void Remove(GameObject* object) = 0;
};

#endif //DUALTREENODE_H
