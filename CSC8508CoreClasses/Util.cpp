//
// Contributors: Alfie
//

#ifndef UTIL_H
#define UTIL_H

#include <random>
#include "Transform.h"

namespace Util {
    template <typename T>
    std::vector<T> RandomiseVector(std::vector<T> const& inVector) {
        static std::random_device rd;
        static std::default_random_engine rng(rd());
        std::vector<T> out(inVector);
        std::ranges::shuffle(out, rng);
        return out;
    }

    inline void GetAllOBBVertices(Vector3 array[8], NCL::CSC8508::Transform const& worldTransform, Vector3 const& halfDimensions) {
        for (int i = 0; i < 8; i++) {
            array[i] = worldTransform.GetPosition() + worldTransform.GetOrientation() * (halfDimensions * Vector3(
                i & 1 ? 1 : -1,
                i & 2 ? 1 : -1,
                i & 4 ? 1 : -1
                ));
        }
    }
}

#endif //UTIL_H
