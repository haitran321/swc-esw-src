/**
* $Id: DeviceEvent.h 5100 2009-12-04 20:54:54Z nei18232 $
*
* Class DeviceEvent represents an event associated with a device
* within the event processor architecture. It is derived from 
* class Event. 
*
*/
#ifndef DeviceEvent_H
#define DeviceEvent_H

#include "Device.h"
#include "Event.h"

class EventProcessor;

#define NO_PARAM  -1

typedef void (EventProcessor::*EventFunc)();
typedef void (EventProcessor::*EventFuncInt)(int param);

typedef enum
{
   READ_EVENT,
   WRITE_EVENT,
   STATUS_EVENT
} EventType;

typedef enum
{
   DispatchToEventFunc,
   DispatchToEventFuncInt
} EventDispatchType;

class DeviceEvent : public Event
{
public:

   /**
   * Constructor
   *
   */
   DeviceEvent() : Event(-1), _device(NULL), _type(READ_EVENT),
      _eventFunc(NULL), _eventFuncInt(NULL), _eventParam(NO_PARAM),
      _eventDispatchType(DispatchToEventFunc)
   {
   }

   /**
   * Constructor
   *
   * @param device Device object to associate event with
   * @param type Event type
   * @param priority Event priority
   */
   DeviceEvent(Device &device, EventType type, int priority) :
      Event(priority), _device(&device), _type(type),
      _eventFunc(NULL), _eventFuncInt(NULL), _eventParam(NO_PARAM),
      _eventDispatchType(DispatchToEventFunc) {};

   /**
   * Constructor
   *
   * @param device Device object to associate event with
   * @param type Event type
   * @param priority Event priority 
   * @param eventFunc Event function 
   */
   DeviceEvent(Device &device, EventType type, int priority,
               EventFunc eventFunc) :
      Event(priority), _device(&device), _type(type), 
      _eventFunc(eventFunc), _eventFuncInt(NULL), 
      _eventParam(NO_PARAM), _eventDispatchType(DispatchToEventFunc) {};

   /**
   * Constructor
   *
   * @param device Device object to associate event with
   * @param type Event type
   * @param priority Event priority 
   * @param eventFuncInt Event function 
   */
   DeviceEvent(Device &device, EventType type, int priority,
               EventFuncInt eventFuncInt, int param) :
      Event(priority), _device(&device), _type(type), 
      _eventFunc(NULL), _eventFuncInt(eventFuncInt), 
      _eventParam(param), _eventDispatchType(DispatchToEventFuncInt) {};

   /**
   * Destructor
   *
   */
   virtual ~DeviceEvent(){};

   /**
   * Method getEventFunc returns the event function.
   *
   * @return Event function associated with this event 
   */
   inline EventFunc getEventFunc() const
   {
      return(_eventFunc);
   }

   /**
   * Method getEventFuncInt returns the event function.
   *
   * @return Event function associated with this event 
   */
   inline EventFuncInt getEventFuncInt() const
   {
      return(_eventFuncInt);
   }

   /**
   * Method getEventParam returns the event parameter to pass into 
   * event function. 
   *
   * @return Event parameter
   */
   inline int getEventParam() const
   {
      return(_eventParam);
   };

   /**
   * Method getEventDispatchType returns the event dispatch type.
   *
   * @return Event dispatch type 
   */
   inline EventDispatchType getEventDispatchType() const
   {
      return(_eventDispatchType);
   };

   /**
   * Method getType returns event type.
   *
   * @return Event type
   */
   inline EventType getType() const
   {
      return(_type);
   };

   /**
   * Method getFd returns device file descriptor.
   *
   * @return Device descriptor associated with event
   */
   inline int getFd() const
   {
      if (_device != NULL)
      {
         return(_device->getFd());
      }
      else
      {
         return(NO_FD);
      }
   };

   /**
   * Method isEvent returns if device object needs servicing, i.e. 
   * caused the current event to occur. 
   *
   * @param checkDevice Device object to check
   * @param eventType Event type
   * @return Flag indicating if device object caused event
   */
   inline bool isEvent(Device &checkDevice, int eventType) const
   {
      return(_device->getFd() == checkDevice.getFd() &&
             _type == eventType);
   }

   /**
   * Method getName return event name.
   *
   * @return Event name
   */
   inline const string &getName() const
   {
      return(_device->getName());
   }

private:

   /** Device object */

   Device *_device;

   /** Device event type */

   EventType _type;

   /** Device event function */

   EventFunc _eventFunc;

   /** Device event function with integer parameter */

   EventFuncInt _eventFuncInt;

   /** Device event parameter for _eventFuncInt */

   int _eventParam;

   /** Device event dispatch type */

   EventDispatchType _eventDispatchType;

};

#endif


