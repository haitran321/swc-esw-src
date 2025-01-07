#include <arpa/inet.h>
#include <string.h>
#include "EndianUtils.h"

// **********************************************************************
/**
* Function fromNetworkInt converts a Network integer (big endian) to a
* ARM integer (little endian). 
*  
* @param value Network int  
* @return ARM int 
*/
   
int fromNetworkInt(int  value)
{
   //ToLittleEndian32(value);
   return(ntohl(value));
}

// **********************************************************************
// **********************************************************************
/**
* Function toNetworkInt converts a ARM integer (little 
* endian) to a Network integer (big endian). 
*  
* @param value ARM int 
* @return Network int
*/
int toNetworkInt(int   value)
{
   //ToBigEndian32(value);
   return(htonl(value));
}

// **********************************************************************
// **********************************************************************
/**
* Function toNetworkFloat converts a ARM float (little 
* endian) to a Network float (big endian). 
*  
* @param value ARM float 
* @return Network float
*/
float toNetworkFloat(float value)
{
   float newValue;
   int  floatInt;
   int  *floatPtr = (int  *)&value;

   floatInt = htonl(*floatPtr);
   memcpy(&newValue, &floatInt, sizeof(value));
   return(newValue);
}
// **********************************************************************
// **********************************************************************
/**
* Function fromNetworkFloat converts a Network float (big endian) to a ARM (little 
* endian) float
*  
* @param value Network  float 
* @return ARM float
*/
float fromNetworkFloat(float value)
{
   float newValue;
   int  floatInt;
   int  *floatPtr = (int  *)&value;
   
   floatInt = ntohl(*floatPtr);
   memcpy(&newValue, &floatInt, sizeof(value));
   return(newValue);
}

