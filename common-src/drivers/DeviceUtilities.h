/**
* $Id: DeviceUtilities.h 5142 2009-12-15 19:01:09Z ste38548 $
*/

#ifndef DEVICE_UTILITIES_H
#define DEVICE_UTILITIES_H

#define MASK_ALL_F   0xFFFFFFFF

#define VALID_ACTION 1
#define VALID_ACTION_MASK 0x80000000

#define TIME_OF_EXECUTION_MASK 0x007FFFFF

class DeviceUtilities
{
    public:


    /**
    * Method readMask is used for manipulate the value with
    * the mask for read cycle
    *
    * @param mask - mask
    * @param value - the value to manipulate
    * @return - manipulated value
    */
    static int readMask(int mask, int value);

    /**
     * Method updateReg() returns the resulting value of "currentValue" OR'd
     * with the appropriately masked ("mask" shifted and applied) 
     * "newValue".
     *
     * @param mask - Bitmask applicable to newValue, which is updated to 
     *             currentValue.
     * @param currentValue - The original to be updated with the 
     *                     mask-applied newValue
     * @param newValue - The new value to apply to currentValue
     * @return - Resulting value.
     */
    static int updateReg(int mask, int currentValue, int changeValue);
        
    /** 
     * Function sortCmdFIFOActions() sorts the user-provided sortActionList 
     * and addlActionList based on the TIME_OF_EXECUTION_MASK bits within 
     * sortActionList. 
     * 
     * @param sortActionList - Command FIFO containing the 
     *                         TIME_OF_EXECUTION_MASK bits for each action.
     * @param addlActionList - Additional accompanying Command FIFO that 
     *                       correlates to, and to be sorted the same as,
     *                       sortActionList.
     *                       
     * @param numActions - Number of actions to sort within the action 
     *                   listings.
     * 
     * @return int
     */
    static void sortCmdFIFOActions(int sortActionList[], 
                                    int addlActionList[], 
                                    int numActions);
};


#endif //DEVICE_UTILITIES_H
