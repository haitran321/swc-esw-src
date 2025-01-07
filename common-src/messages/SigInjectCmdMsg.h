/**
* $Id: SigInjectCmdMsg.h 5101 2009-12-04 20:55:24Z nei18232 $
*
* Class SigInjectCmdMsg represents a Signal Injection Command
* message sent to RIMS and is derived from class
* RIMSCommandMessage. It provides methods to manipulate the body
* of a Signal Injection Command message.
*
*/
#ifndef SigInjectCmdMsg_H
#define SigInjectCmdMsg_H

#include <string>

using namespace std;

#include "RIMSCommandMessage.h"

class SigInjectCmdMsg : public RIMSCommandMessage
{
public:

   /**
   * Constructor
   *
   * @param buffer Raw message buffer
   * @param bufSize Raw message buffer size
   */
   SigInjectCmdMsg(char *buffer, int bufSize);

   /**
   * Destructor
   *
   */
   virtual ~SigInjectCmdMsg(){};

   /**
   * Method validateData validates data from the data section of 
   * the raw message to the member variables. 
   *
   * @return Status of operation
   */
   virtual STATUS validateData();

   void byteSwapToLocal();

private:

   /** Signal Injection command data */

   SigInjectCmdDataType *_data;

};

#endif
