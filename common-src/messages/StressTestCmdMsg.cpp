#include "StressTestCmdMsg.h"

StressTestCmdMsg::StressTestCmdMsg(char *buffer, int bufSize) :
    CommandMessage(buffer, bufSize),
    _data(reinterpret_cast<StressTestCmdDataType *>(getDataBufPos()))
{
}

STATUS StressTestCmdMsg::validateData()
{
    // Validate header record length against expected record length.

    if (_header->recLen != sizeof(StressTestCmdDataType))
    {
        printf("ERROR::InvalidRecordLength, Stress Test Command (recLen = %d record size = %d)",
               _header->recLen, static_cast<int>(sizeof(StressTestCmdDataType)));
        return (ERROR);
    }

    // Validate data content.

    return (OK);
}

void StressTestCmdMsg::byteSwapToLocal()
{
    _data->numTest = (int)fromNetworkInt(_data->numTest);
    _data->numInc = (int)fromNetworkInt(_data->numInc);
    _data->spacingUsec = (int)fromNetworkInt(_data->spacingUsec);
    _data->alpha = (int)fromNetworkInt(_data->alpha);
    _data->alphaInc = (int)fromNetworkInt(_data->alphaInc);
    _data->beta = (int)fromNetworkInt(_data->beta);
    _data->betaInc = (int)fromNetworkInt(_data->betaInc);
}
