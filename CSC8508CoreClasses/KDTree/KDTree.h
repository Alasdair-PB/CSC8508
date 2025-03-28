//
// Contributors: Alfie
//

#ifndef KDTREE_H
#define KDTREE_H

#include "KDTreeBranch.h"
#include "KDTreeLeaf.h"
#include "KDTreeNode.h"

template <typename T>
class KDTree {
public:
    KDTree() { root = new KDTreeLeaf<T>(); }

    void Insert(KDTreeEntry<T> const& entry) {
        if (!root->Insert(entry)) {
            root = KDTreeBranch<T>::Split(root);
            root->Insert(entry);
        }
    }

    std::vector<KDTreeEntry<T>> Get(KDTreeQuery const& query) const {
        std::vector<KDTreeEntry<T>> out = std::vector<KDTreeEntry<T>>();
        root->Get(query, out);
        return out;
    }

    void Remove(KDTreeEntry<T> const& entry) { root->Remove(entry); }

    void Print() {
        std::cout << "Tree!\n";
        root->Print();
    }

protected:
    KDTreeNode<T>* root;
};

#endif //KDTREE_H
