#ifndef Timestamp_H
#define Timestamp_H

#include <chrono>
#include <cstdint>
#include "date.h"

using namespace std::chrono;

class Timestamp
{
    public:
        Timestamp();

        uint64_t GetTimestamp();

        uint64_t GetSecondsSinceMidnight();

        uint64_t GetNanoSecondsSinceMidnight();

};


#endif // ElapsedTimer_H
