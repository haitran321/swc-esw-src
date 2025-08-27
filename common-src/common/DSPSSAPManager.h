#ifndef DSPSSAPManager_H
#define DSPSSAPManager_H

#include <stdio.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "Uncopyable.h"
#include "InternalMsgTypes.h"
#include "DefineUtils.h"

#define INT_TYPE        1        ///< Integer data type used for extract value from a sap/config file
#define FLOAT_TYPE      2        ///< Float data type used for extract value from a sap/config file
#define STRING_TYPE     3        ///< String data type used for extract value from a sap/config file

#define NUM_VME_VOLTAGE_MONITORS 4
#define NUM_VME_TEMP_MONITORS    3
#define NUM_VME_FAN_MONITORS     3

#define ALIGN_SIZE 32

/*!
 *
 * \brief Structure to map the site adjustable parameters.
 *
 */
struct DspsSap
{
    int    sap011;
    float  sap015;
 
    int    chassisFanUpperLimit;
    int    chassisFanLowerLimit;
 
    float  dcOffsets[MAX_PULSE_TYPES];
};

#define DSPS_SAP_BYTES     sizeof(DspsSap)


/**
 * Retrieves DSPS Parameters From DSPS Files
 */
class DSPSSAPManager : public Uncopyable
{
public:
    /**
     * Destructor
     */
    virtual ~DSPSSAPManager();

    /**
     * Returns the singleton object for this class.
     *
     * @return Singleton object for this class
     */
    static DSPSSAPManager& getInstance();

    STATUS ReadDataFiles();

    int GetSingleSapValue(char *fileBuf, int fileSize, char *sapName, int sapDataType, void *sapValue);

    DspsSap dspsSap;

private:

    // Disallow construction
    DSPSSAPManager();
};

#endif /* #ifndef DSPSSAPManager_H */
