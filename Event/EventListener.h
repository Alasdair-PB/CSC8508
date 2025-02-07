//
// Contributors: Alfie
//

#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

template <typename E>
class EventListener
{
public:
    virtual ~EventListener() = default;

    /**
     * Function invoked by the Event Manager when an event of the target event type is called.
     * @tparam E Event type class
     * @param e Event instance pointer
     */
    virtual void OnEvent(E* e) = 0;
};

#endif //EVENTLISTENER_H
