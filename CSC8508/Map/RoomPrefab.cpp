//
// Contributors: Alfie
//

#include "RoomPrefab.h"

#include "Axis.h"
#include "BoundsComponent.h"

NCL::AABBVolume RoomPrefab::GetEncasingVolume() const {
    auto min = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    auto max = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    auto children = std::vector<GameObject*>(); // TODO: Replace these 2 lines with roomObject.GetChildren() once it exists
    children.push_back(roomObject);

    // If no children, return an encasing volume with no size (should not happen) // TODO: Review need for this
    if (children.empty()) return {Vector3(0, 0, 0)};

    for (GameObject* go : children) {
        auto* bounds = go->TryGetComponent<BoundsComponent>();
        if (bounds == nullptr) continue;
        Vector3 broadphase;
        bounds->GetBroadphaseAABB(broadphase); // TODO: Make sure this is worldspace when GameObject is updated
        Vector3 tryMin = go->GetTransform().GetOrientation() * (go->GetTransform().GetPosition() - broadphase) * go->GetTransform().GetScale();
        Vector3 tryMax = go->GetTransform().GetOrientation() * (go->GetTransform().GetPosition() + broadphase) * go->GetTransform().GetScale();
        for (Axis a = x; a <= z; a++) {
            if (tryMin[a] < min[a]) min[a] = tryMin[a];
            if (tryMax[a] < max[a]) max[a] = tryMax[a];
        }
    }

    return {Vector3(
        (max[x] - min[x]) / 2,
        (max[y] - min[y]) / 2,
        (max[z] - min[z]) / 2
        )};
}
