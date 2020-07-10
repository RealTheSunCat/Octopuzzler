#pragma once
#include <functional>

#include "Core.h"

#define EVENT_CLASS_TYPE(type) static EventType getStaticType() { return EventType::##type; } \
                               virtual EventType getEventType() const override { return getStaticType(); } \
                               virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(categories) virtual int getCategoryFlags() const override { return (categories); }

enum class EventType
{
    None = 0,
    //WindowClose, WindowResize, WindowFocus, WindowUnfocus, WindowMove,
    KeyPressed, KeyReleased,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

enum EventCategory
{
    None     = 0,
    Window   = BIT(0),
    Input    = BIT(1),
    Keyboard = BIT(2),
    Mouse    = BIT(3)
};

class Event
{
    friend class EventDispatcher;
public:
    virtual EventType getEventType() const = 0;
    virtual const char* getName() const = 0;
    virtual int getCategoryFlags() const = 0;

    inline bool inCategory(const EventCategory category) const { return getCategoryFlags() & category; }

protected:
    bool handled = false;
};

class EventDispatcher
{
    template<typename T>
    using EventFn = std::function<bool(T&)>;

public:
    EventDispatcher(Event& _event)
        : event(_event) {}


    template<typename T>
    bool dispatch(EventFn<T> func)
    {
        if (event.getEventType() == T::getStaticType())
        {
            event.handled = func(*(T*)&event);
            return true;
        }

        return false;
    }

private:
    Event& event;
};