#ifndef Timestamp_H
#define Timestamp_H

#include <chrono>
#include <cstdint>

using namespace std::chrono;

class Timestamp
{
    public:
        Timestamp();

        uint64_t GetTimestamp();

};


#endif // ElapsedTimer_H
