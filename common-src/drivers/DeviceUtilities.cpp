#include <stdio.h>
#include <strings.h>

#include "DeviceUtilities.h"

/******************************************************************************/ 
int DeviceUtilities::readMask(int mask, int value)
{
   value = mask & value; 

   // temp = left shift ((ffsLsb(mask) - 1) times
   //   - The cast to unsigned int is utilized so as not to shift the sign bit,
   //     if set, as all of the VME FW registers are technically unsigned
   //     values.
   value = static_cast<int>(static_cast<unsigned int>(value) >> (ffs(mask) - 1));

   //*(int *)readValue = readReg;
   return (value);
}

/******************************************************************************/ 
int DeviceUtilities::updateReg(int mask, int currentValue, int changeValue)
{
   int temp = 0;
   int newValue = -1;

   /* update reg */
   /* EX:  mask = 0x000F_FFF0, currentValue = 0x12341234, newValue = 0x125678
	* (ffsLsb(mask) - 1)) = 3
	* (newValue << (ffsLsb(mask) - 1)) = 0x1256780
	* 0x56780 & mask = 0x56780
	*/
   newValue = (mask >> (ffs(mask) - 1)) & changeValue;
   newValue = (newValue << (ffs(mask) - 1)) & mask;

   /* (~mask) = 0xFFF0_000F
	*  currentValue & (~mask) = 0x1230_0004
	*/
   temp = currentValue & (~mask);

   /* 0x1230_0004 | 0x56780 = 0x1235_6784
	*/
   newValue = temp | newValue;

   return (newValue);
}

/******************************************************************************/ 
void DeviceUtilities::sortCmdFIFOActions(int sortActionList[], 
                                             int addlActionList[], 
                                             int numActions)
{
    int i, j, currSortActionVal, currAddlActionVal;
    for (i = 1; i < numActions; ++i)
    {
        currSortActionVal = sortActionList[i];
        currAddlActionVal = addlActionList[i];
        for (j = i; 
             (j > 0) && 
             ((sortActionList[j-1] & TIME_OF_EXECUTION_MASK) > 
              (currSortActionVal & TIME_OF_EXECUTION_MASK));
             j--)
        {
            sortActionList[j] = sortActionList[j-1];
            addlActionList[j] = addlActionList[j-1];
        }
        
        sortActionList[j] = currSortActionVal;
        addlActionList[j] = currAddlActionVal;
    }
    
}//sortCmdFIFOActions()

