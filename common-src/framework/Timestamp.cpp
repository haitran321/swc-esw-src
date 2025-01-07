#include "Timestamp.h"

Timestamp::Timestamp()
{
}

uint64_t Timestamp::GetTimestamp()
{
    return (duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}



