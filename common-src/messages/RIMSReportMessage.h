/**
* $Id: RIMSReportMessage.h 5101 2009-12-04 20:55:24Z nei18232 $
*
* Class RIMSReportMessage is the base class that represents 
* report messages sent to RIMS. It provides methods common to 
* all RIMS report messages. It is derived from the base class 
* RIMSMessage. 
*
*/
#ifndef RIMSReportMessage_H
#define RIMSReportMessage_H

#include "CSPUMsgTypes.h"
#include "RIMSMessage.h"

class RIMSReportMessage : public RIMSMessage
{
public:

   /**
   * Constructor
   *
   */
   RIMSReportMessage();

   /**
   * Constructor
   *
   * @param msgId Message id for RIMS Report message
   */
   explicit RIMSReportMessage(RIMSMessageId msgId);

   /**
   * Destructor
   *
   */
   virtual ~RIMSReportMessage(){};

   /**
   * Method buildMsg is a virtual method that is implemented by a 
   * derived report message class. This method is called by the 
   * derived class to build the header of the RIMS Report message, 
   * and the derived class builds the data portion. 
   *
   * @return Status of operation
   */
   virtual STATUS buildMsg();

   /**
   * Method setPBPId sets the PBP id in the RIMS Report header.
   *
   * @param pbpId PBP id
   */
   inline void setPBPId(int pbpId);

   void headerByteSwapToNetwork();

protected:

   /**
   * Method addData copies data from the input data buffer to the 
   * data portion of the RIMS Report message at the current data 
   * pointer. 
   *
   * @param data Data buffer to copy data from
   * @param size Number of bytes to copy
   * @return Status of operation
   */
   STATUS addData(const char *data, int size);

};

inline void RIMSReportMessage::setPBPId(int pbpId)
{
   _header->pbpId = pbpId;
}

#endif
