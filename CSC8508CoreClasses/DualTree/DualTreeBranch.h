//
// Contributors: Alfie
//

#ifndef DUALTREEBRANCH_H
#define DUALTREEBRANCH_H

#include "DualTreeNode.h"

class DualTreeBranch final : public DualTreeNode {
public:
    DualTreeBranch(DualTreeNode* lessNode, DualTreeNode* moreNode) : lessNode(lessNode), moreNode(moreNode) { }

    bool Insert(GameObject* object) override;

    GameObject** Get(Vector3<float> position, Vector3<float> halfDimensions, std::vector<GameObject*>& output) override;

    void Remove(GameObject* object) override;

private:
    DualTreeNode* lessNode;
    DualTreeNode* moreNode;
};

#endif //DUALTREEBRANCH_H
