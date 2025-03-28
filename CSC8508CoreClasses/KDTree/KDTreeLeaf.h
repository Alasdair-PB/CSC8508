//
// Contributors: Alfie
//

#ifndef KDTREELEAF_H
#define KDTREELEAF_H
#include "KDTreeNode.h"

template <typename T>
class KDTreeLeaf : public KDTreeNode<T> {
public:
    bool Insert(KDTreeEntry<T> entry) override;

    void Get(KDTreeQuery const& query, std::vector<KDTreeEntry<T>>& output) const override;

    bool Remove(KDTreeEntry<T> const& entry) override;

    void Print() override; // TODO: Remove

protected:
    KDTreeEntry<T> entries[MAX_ITEMS] = {};
};


template<typename T>
bool KDTreeLeaf<T>::Insert(KDTreeEntry<T> entry) {

    for (int i = 0; i < MAX_ITEMS; i++) if (entries[i].IsEmpty()) {
        entries[i] = entry;
        return true;
    }

    // Return false if there's no free spot
    return false;
}


template<typename T>
void KDTreeLeaf<T>::Get(KDTreeQuery const& query, std::vector<KDTreeEntry<T>>& output) const {
    for (KDTreeEntry<T> e : entries) {
        if (e.IsEmpty()) return;
        output.push_back(e);
    }
}


template<typename T>
bool KDTreeLeaf<T>::Remove(KDTreeEntry<T> const& entry) {
    bool found = false;
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (found) {
            entries[i - 1] = entries[i];
        } else if (entries[i].value == entry.value) {
            found = true;
            entries[i] = KDTreeEntry<T>();
        }
    }
    return found;
}


template<typename T>
void KDTreeLeaf<T>::Print() {
    std::cout << "Leaf!\n";
    for (KDTreeEntry<T> e : entries)
        if (!e.IsEmpty()) std::cout << "Item found! Position = " << e.position.x << ", " << e.position.y << ", " << e.position.z
            << ", half dimensions = " << e.halfDimensions.x << ", " << e.halfDimensions.y << ", " << e.halfDimensions.z << "!\n";
        else std::cout << "____\n";
}

#endif //KDTREELEAF_H
