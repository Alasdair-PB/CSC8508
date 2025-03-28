//
// Contributors: Alfie
//

#ifndef KDTREEENTRY_H
#define KDTREEENTRY_H

using namespace NCL::Maths;

struct KDTreeQuery {
    Vector3 position;
    Vector3 halfDimensions;

    KDTreeQuery() = default;

    KDTreeQuery(Vector3 const position, Vector3 const halfDimensions) {
        this->position = position;
        this->halfDimensions = halfDimensions;
    }
};

template <typename T>
struct KDTreeEntry : KDTreeQuery {
    T* value;

    KDTreeEntry() = default;

    KDTreeEntry(T* const value, Vector3 const position, Vector3 const halfDimensions)
        : KDTreeQuery(position, halfDimensions) { this->value = value; }

    bool IsEmpty() { return value == nullptr; }
};

#endif //KDTREEENTRY_H
