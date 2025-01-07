/**
* $Id: Event.h 5100 2009-12-04 20:54:54Z nei18232 $
*
* Class Event is the base class that represents an event within 
* the event notification architecture. 
*  
* Events have priorities within the event list owned by an event 
* processor. Identifiers are used to identify when the event has 
* occurred within the event processor loop. 
*
*/
#ifndef Event_H
#define Event_H

class Event
{
public:

   /**
   * Constructor
   *
   */
   Event() :
      _priority(-1), _count(0) {};

   /**
   * Constructor
   *
   * @param priority Event priority
   */
   explicit Event(int priority) :
      _priority(priority), _count(0) {};

   /**
   * Destructor
   *
   */
   virtual ~Event(){};

   /**
   * Method getPriority returns the event priority.
   *
   * @return Event priority
   */
   inline int getPriority() const
   {
      return _priority;
   };

   /**
   * Method getCount returns the number of times the event has 
   * occurred. 
   *
   * @return Event count
   */
   inline int getCount() const
   {
      return _count;
   };

   /**
   * Method incCount increments the event count.
   *
   */
   inline void incCount()
   {
      _count++;
   };

private:

   /** Event priority (lower number = higher priority) */

   int _priority;

   /** Event count */

   int _count;
};

#endif


