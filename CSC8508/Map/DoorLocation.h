//
// Contributors: Alfie
//

#ifndef DOORLOCATION_H
#define DOORLOCATION_H

#include "Vector.h"

using namespace NCL::Maths;

struct DoorLocation {
    Vector3 const pos;
    Vector3 const dir;

    DoorLocation(Vector3 const position, Vector3 const direction)
        : pos(position), dir(direction) { }
};

#endif //DOORLOCATION_H
