//
// Contributors: Alfie
//

#include "DualTreeLeaf.h"

bool DualTreeLeaf::Insert(GameObject* object) {
    // If the items array is full
    if (items[MAX_ITEMS - 1] != nullptr) return false;

    // Insert into the first free spot
    for (int i = 0; i < MAX_ITEMS; i++) if (items[i] == nullptr) {
        items[i] = object;
        return true;
    }

    // Should never be reached, but sometimes you've gotta add some safety mechanisms yk?
    return false;
}

void DualTreeLeaf::Get(Vector3<float> position, Vector3<float> halfDimensions, GameObject* output[MAX_ITEMS]) {
    std::copy(items[0], items[MAX_ITEMS], output);
}

void DualTreeLeaf::Remove(GameObject* object) {
    bool found = false;
    for (int i = 0; i < MAX_ITEMS; i++) {

    }
}

