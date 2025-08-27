#ifndef ModeCmdMsg_H
#define ModeCmdMsg_H

#include <string>

using namespace std;

#include "SWCMsgTypes.h"
#include "CommandMessage.h"

class ModeCmdMsg : public CommandMessage
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
   * Method getOpState returns the operational state field.
   *
   * @return Operational state value
   */
   inline OpState getOpState();

   void byteSwapToLocal();

private:

   ModeCmdDataType *_data;

};

Mode ModeCmdMsg::getMode()
{
   return(_data->mode);
}

OpState ModeCmdMsg::getOpState()
{
   return(_data->opState);
}

#endif
