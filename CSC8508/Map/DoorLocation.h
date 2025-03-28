//
// Contributors: Alfie
//

#ifndef DOORLOCATION_H
#define DOORLOCATION_H

#include "ISerializable.h"
#include "Vector.h"

using namespace NCL::Maths;

struct DoorLocation : NCL::CSC8508::ISerializedData{
    Vector3 pos;
    Vector3 dir;

    DoorLocation() = default;
    DoorLocation(Vector3 const position, Vector3 const direction)
        : pos(position), dir(direction) { }
};

#endif //DOORLOCATION_H
