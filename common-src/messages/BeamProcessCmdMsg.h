/**
* $Id: BeamProcessCmdMsg.h 5101 2009-12-04 20:55:24Z nei18232 $
*
* Class BeamProcessCmdMsg represents a Shutdown Command message 
* sent to RIMS and is derived from class RIMSCommandMessage. It 
* provides methods to manipulate the body of a Shutdown Command
* message. 
*
*/
#ifndef BeamProcessCmdMsg_H
#define BeamProcessCmdMsg_H

#include <string>

using namespace std;

#include "RIMSCommandMessage.h"

class BeamProcessCmdMsg : public RIMSCommandMessage
{
public:

   /**
   * Constructor
   *
   * @param buffer Raw message buffer
   * @param bufSize Raw message buffer size
   */
   BeamProcessCmdMsg(char *buffer, int bufSize);

   /**
   * Destructor
   *
   */
   virtual ~BeamProcessCmdMsg(){};

   /**
   * Method validateData validates data from the data section of 
   * the raw message to the member variables. 
   *
   * @return Status of operation
   */
   virtual STATUS validateData();

   void byteSwapToLocal();

private:

   /** Beam Process command data */

   BeamProcessCmdDataType *_data;

};

#endif
