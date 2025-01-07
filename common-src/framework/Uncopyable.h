/**
* $Id: Uncopyable.h 5100 2009-12-04 20:54:54Z nei18232 $
*
* Class Uncopyable is a base class used to make a derived class 
* uncopyable. 
*
*/
#ifndef Uncopyable_H
#define Uncopyable_H

class Uncopyable 
{
protected:

   /**
   * Constructor
   *
   */
   Uncopyable(){} 

   /**
   * Destructor
   *
   */
   virtual ~Uncopyable(){} 

private:

   /**
   * Constructor
   *
   */
   Uncopyable(const Uncopyable&);

   /**
   * Method operator= is defined here so that a derived class of 
   * Uncopyable cannot be assigned. 
   *
   * @return Uncopyable object
   */
   Uncopyable& operator=(const Uncopyable&);

};

#endif
