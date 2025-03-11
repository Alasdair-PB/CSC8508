//
// Contributors: Alfie
//

#ifndef AXIS_H
#define AXIS_H

#include <cstdint>
#include <iostream>

enum Axis : uint8_t
{
    x,
    y,
    z,
    w
};


inline Axis operator++(Axis& axis, int) {
    return axis = static_cast<Axis>((static_cast<int>(axis) + 1) % 4);
}


inline Axis operator--(Axis& axis, int) {
    return axis = static_cast<Axis>((static_cast<int>(axis) - 1 + 4) % 4);
}

#endif //AXIS_H