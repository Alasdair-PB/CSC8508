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


/**
 * Base class for events that can be cancelled.
 *
 * To cancel an event, listeners should use SetCancelled() at an appropriate priority. Listeners should check whether
 * CancellableEvents have been cancelled before acting on them, ideally using the MONITOR priority (since events can
 * be cancelled by a later priority listener).
 *
 * If you call a cancellable event, you NEED to check if the event has been cancelled before acting on it, it is not
 * handled automatically.
 */
class CancellableEvent : public Event
{
public:
    /**
     * Sets whether the event should be cancelled. This MAY be changed by a later event. Use a MONITOR priority event
     * to be certain
     */
    void SetCancelled(bool const cancelled) { this->cancelled = cancelled; }

    /**
     * Checks whether the event has already been cancelled. Events may still be cancelled later. To be certain, use
     * the MONITOR priority when registering your listener.
     * @return TRUE if currently cancelled, FALSE otherwise
     */
    [[nodiscard]]
    bool IsCancelled() const { return cancelled; }

protected:
    bool cancelled = false;
};


// EventType must be a child class of Event, not including Event or CancellableEvent
template <typename T>
concept EventType = std::is_base_of_v<Event, T> && !(std::is_same_v<Event, T> || std::is_same_v<CancellableEvent, T>);

#endif //EVENT_H
