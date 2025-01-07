#ifndef SPSummaryRptMsg_H
#define SPSummaryRptMsg_H

#include <string>

using namespace std;

#include "RIMSReportMessage.h"

typedef struct
{
    unsigned char    commandInstructionValid;
    unsigned char    computationError;
    unsigned char    dspsReportType;
    unsigned char    rangeWindowReportValid;
    unsigned int     startRangeBin;
    unsigned int     stopRangeBin;
    unsigned int     numberOfThresholdDetectionsExceeded;
    unsigned int     numberOfThresholdDetections;
    float            mean;
    float            PDT;
    unsigned int     numberOfSLBDetection;
    unsigned char    tbrsReportType;
    unsigned char    pusleType;
    unsigned char    mode;
    unsigned char    state;
    unsigned int     satelliteID;
    unsigned int     agcSelect;
    unsigned int    numberOfMonopulseDetections;
    unsigned int    numberOfNoiseDetections;
    unsigned int    noiseCalculationMethod;
    float           smoothedPDTMeanNoise;
    unsigned int    numberOfRWNoiseSamples;
    float           rangeWindowNoise;
} SPSummaryRptDataType;

class SPSummaryRptMsg : public RIMSReportMessage
{
public:

   /**
   * Constructor
   *
   */
   SPSummaryRptMsg();

   /**
   * Destructor
   *
   */
   virtual ~SPSummaryRptMsg();

   void addSPSummaryRpt(SPSummaryRptDataType &data);

   /**
   * Method buildMsg builds the message given the current member 
   * variable settings. 
   *
   * @return Status of operation
   */
   virtual STATUS buildMsg();

   inline void setRecvId(int recvId);

private:

   /** SPSummary report data */

   SPSummaryRptDataType _data;

};

inline void SPSummaryRptMsg::setRecvId(int recvId)
{
   _header->recvId = recvId;
}

#endif
