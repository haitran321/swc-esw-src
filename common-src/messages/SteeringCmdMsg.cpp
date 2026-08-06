/**
* $Id: SteeringCmdMsg.cpp 5101 2009-12-04 20:55:24Z nei18232 $
*/
#include "SteeringCmdMsg.h"

SteeringCmdMsg::SteeringCmdMsg(char *buffer, int bufSize) :
    CommandMessage(buffer, bufSize),
    _data(reinterpret_cast<SteeringCmdDataType *>(getDataBufPos()))
{
}

STATUS SteeringCmdMsg::validateData()
{
    // Validate header record length against expected record length.

    if (_header->recLen != sizeof(SteeringCmdDataType))
    {
        printf("ERROR::InvalidRecordLength, Steering Command (recLen = %d record size = %d)",
               _header->recLen, static_cast<int>(sizeof(SteeringCmdDataType)));
        return (ERROR);
    }

    // Validate data content.

    if (_data->testSource < Analog || _data->testSource > Digital)
    {
        printf("ERROR::InvalidCommandData, Steering Command testSource = %d)",
               _data->testSource);
        return (ERROR);
    }

    if (_data->RLCP < Off || _data->RLCP > On)
    {
        printf("ERROR::InvalidCommandData, Steering Command RLCP = %d)",
               _data->RLCP);
        return (ERROR);
    }

    if (_data->RLSC < Off || _data->RLSC > On)
    {
        printf("ERROR::InvalidCommandData, Steering Command RLSC = %d)",
               _data->RLSC);
        return (ERROR);
    }

    return (OK);
}

void SteeringCmdMsg::byteSwapToLocal()
{
    _data->testSource = (TestSource)fromNetworkInt(_data->testSource);
    _data->alpha = (int)fromNetworkInt(_data->alpha);
    _data->beta = (int)fromNetworkInt(_data->beta);
    _data->RLCP = (CmdOnOff)fromNetworkInt(_data->RLCP);
    _data->RLSC = (CmdOnOff)fromNetworkInt(_data->RLSC);
}
