#include "Timestamp.h"

Timestamp::Timestamp()
{
}

uint64_t Timestamp::GetTimestamp()
{
//  return (duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    return (duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count());
}

uint64_t Timestamp::GetSecondsSinceMidnight()
{
    // Get current time
    auto now = std::chrono::high_resolution_clock::now();

    // Convert to time_point representing today at midnight
    auto today = date::floor<date::days>(now);

    // Calculate the duration since midnight
    auto time_since_midnight = now - today;

    // Extract seconds from the duration
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(time_since_midnight).count();

    return(seconds);
}

uint64_t Timestamp::GetNanoSecondsSinceMidnight()
{
    // Get current time
    auto now = std::chrono::high_resolution_clock::now();

    // Convert to time_point representing today at midnight
    auto today = date::floor<date::days>(now);

    // Calculate the duration since midnight
    auto time_since_midnight = now - today;

    // Extract nanoseconds from the duration
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(time_since_midnight).count();

    return(nanoseconds);
}





