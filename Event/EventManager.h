//
// Contributors: Alfie
//

#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#define QUEUE PriorityQueue<EventListener<E>*>

#include "EventListener.h"
#include "EventPriority.h"
#include "PriorityQueue.h"

/**
 * A static class that holds pointers to EventListeners and calls their OnEvent() functions when an Event is called.
 */
class EventManager {
public:
    /**
     * Invokes the OnEvent() functions of all registered listeners for this event type.
     * @tparam E Event type (child class)
     * @param e Event instance
     */
    template <EventType E>
    static void Call(E* e);

    template <EventType E>
    static void RegisterListener(EventListener<E>* listener, EventPriority priority = DEFAULT);

    template <EventType E>
    static void UnregisterListener(EventListener<E>* listener);

    /**
     * Deletes all listeners of a target Event type and clears the listeners vec.
     *
     * Does NOT delete all listeners for EVERY Event type. If you want to do that, you have to keep track of what Event
     * types you've used.
     * @tparam E Event type (child class)
     */
    template <EventType E>
    static void DeleteEventListeners();

protected:
    template <EventType E>
    static QUEUE listeners;
};


template <EventType E>
QUEUE EventManager::listeners = QUEUE(EARLY); // EARLY because that's the highest value of the priority enum (i.e. the max)


template <EventType E>
void EventManager::Call(E* e) {
    for (int i = 0; i < listeners<E>.GetLength(); i++) listeners<E>[i]->OnEvent(e);
}


template<EventType E>
void EventManager::RegisterListener(EventListener<E>* listener, EventPriority priority) {
    listeners<E>.Insert(listener, priority);
}


template<EventType E>
void EventManager::UnregisterListener(EventListener<E>* listener) {
    listeners<E>.Remove(listener);
}


template <EventType E>
void EventManager::DeleteEventListeners() {
    for (EventListener<E> const* l : listeners<E>.GetValues()) { delete l; }
    listeners<E>.Clear();
}

#endif //EVENTMANAGER_H
