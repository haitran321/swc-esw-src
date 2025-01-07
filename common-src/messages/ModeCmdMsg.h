/**
* $Id: ModeCmdMsg.h 5101 2009-12-04 20:55:24Z nei18232 $
*
* Class ModeCmdMsg represents a Mode Command message sent 
* to RIMS and is derived from class RIMSCommandMessage. It 
* provides methods to manipulate the body of a Mode Command
* message. 
*
*/
#ifndef ModeCmdMsg_H
#define ModeCmdMsg_H

#include <string>

using namespace std;

#include "CSPUMsgTypes.h"
#include "RIMSCommandMessage.h"

class ModeCmdMsg : public RIMSCommandMessage
{
public:

   /**
   * Constructor
   *
   * @param buffer Raw message buffer
   * @param bufSize Raw message buffer size
   */
   ModeCmdMsg(char *buffer, int bufSize);

   /**
   * Destructor
   *
   */
   virtual ~ModeCmdMsg(){};

   /**
   * Method validateData validates data from the data section of 
   * the raw message to the member variables. 
   *
   * @return Status of operation
   */
   virtual STATUS validateData();

   /**
   * Method getMode returns the mode field.
   *
   * @return Mode value
   */
   inline Mode getMode();

   /**
   * Method getLISSState returns the LISS state field.
   *
   * @return LISS state value
   */
   inline LISSState getLISSState();

   /**
   * Method getOpState returns the operational state field.
   *
   * @return Operational state value
   */
   inline OpState getOpState();

   /**
   * Method getSimInterference returns the simulated interference 
   * field. 
   *
   * @return Simulated interference value
   */
   inline SimInterferenceState getSimInterference();

   void byteSwapToLocal();

private:

   ModeCmdDataType *_data;

};

Mode ModeCmdMsg::getMode()
{
   return(_data->mode);
}

LISSState ModeCmdMsg::getLISSState()
{
   return(_data->lissState);
}

OpState ModeCmdMsg::getOpState()
{
   return(_data->opState);
}

SimInterferenceState ModeCmdMsg::getSimInterference()
{
   return(_data->simInterference);
}

#endif
