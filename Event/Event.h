//
// Contributors: Alfie
//

#ifndef EVENT_H
#define EVENT_H
#include <type_traits>

/**
 * Base class for all events
 */
class Event { };

template <typename T>
concept EventType = std::is_base_of_v<Event, T>;

#endif //EVENT_H
