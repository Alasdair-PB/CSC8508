//
// Contributors: Alfie
//

#ifndef DUALTREELEAF_H
#define DUALTREELEAF_H

#include "DualTreeNode.h"

class DualTreeLeaf final : public DualTreeNode {
public:
    bool Insert(GameObject* object) override;

    void Get(Vector3<float> position, Vector3<float> halfDimensions, GameObject* output[MAX_ITEMS]) override;

    void Remove(GameObject* object) override;

private:
    GameObject* items[MAX_ITEMS] = {};
};



#endif //DUALTREELEAF_H
