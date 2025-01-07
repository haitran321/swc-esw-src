/**
* $Id: SLBModeCmdMsg.h 5101 2009-12-04 20:55:24Z nei18232 $
*
* Class SLBModeCmdMsg represents a SLB Mode Command message sent
* to RIMS and is derived from class RIMSCommandMessage. It 
* provides methods to manipulate the body of a SLB Mode Command 
* message. 
*
*/
#ifndef SLBModeCmdMsg_H
#define SLBModeCmdMsg_H

#include <string>

using namespace std;

#include "CSPUMsgTypes.h"
#include "RIMSCommandMessage.h"

class SLBModeCmdMsg : public RIMSCommandMessage
{
public:

   /**
   * Constructor
   *
   * @param buffer Raw message buffer
   * @param bufSize Raw message buffer size
   */
   SLBModeCmdMsg(char *buffer, int bufSize);

   /**
   * Destructor
   *
   */
   virtual ~SLBModeCmdMsg(){};

   /**
   * Method validateData validates data from the data section of 
   * the raw message to the member variables. 
   *
   * @return Status of operation
   */
   virtual STATUS validateData();

   /**
   * Method getType returns the shutdown type field.
   *
   * @return Shutdown type value
   */
   inline SLBModeOption getType();

   void byteSwapToLocal();

private:

   /** Shutodown command data */

   SLBModeCmdDataType *_data;

};

SLBModeOption SLBModeCmdMsg::getType()
{
   return(_data->type);
}


#endif
