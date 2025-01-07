/**
 * $Id: TimeValue.cpp 5100 2009-12-04 20:54:54Z nei18232 $
 * @file TimeValue.cpp
 * @brief Contains implementation details of class TimeValue.
 */

#include "TimeValue.h"

//#include <sysLib.h>

/**
 */
TimeValue TimeValue::getSystemClockResolution()
{
    //return ONE_SECOND() / sysClkRateGet();
    return ONE_SECOND() / 100;
}

/**
 * Sets the clock rate to the closest value that gives the desired resolution.
 * If the resolution does not give an exact integer clock rate, the clock rate
 * will be rounded down, and the resolution will not be exact.
 */
int TimeValue::setSystemClockResolution(const TimeValue& resolution)
{
//    return sysClkRateSet(static_cast<int>(ONE_SECOND() / resolution));
    return 0;
}

/**
 */
TimeValue::TimeValue() : _nanos(0)
{
}

/**
 */
TimeValue::TimeValue(double value, TimeUnit units) :
    _nanos(static_cast<value_type>(convertFrom(value, units)))
{
}

/**
 */
TimeValue::TimeValue(const TimeValue& other) : _nanos(other._nanos)
{
}

/**
 */
TimeValue::TimeValue(double value) : _nanos(static_cast<value_type>(value))
{
}

/**
 */
TimeValue::~TimeValue()
{
}

/**
 */
TimeValue& TimeValue::operator=(const TimeValue& other)
{
    _nanos = other._nanos;
    return *this;
}

/**
 */
void TimeValue::set(double value, TimeUnit units)
{
    _nanos = static_cast<value_type>(convertFrom(value, units));
}

/**
 */
double TimeValue::convertFrom(double value, TimeUnit from)
{
    switch(from)
    {
        // All of these fall through until nanoseconds is reached.
        case Seconds:
            value *= 1000;
        case Milliseconds:
            value *= 1000;
        case Microseconds:
            value *= 1000;
        case Nanoseconds:
            break;
            
        case ClockTicks:
//            value *= ONE_SECOND()._nanos / sysClkRateGet();
            value *= ONE_SECOND()._nanos / 100;
            break;
            
        case PBPCounts:
            value *= ONE_SECOND()._nanos / CSPU_CLOCK_RATE;
            break;
            
        default:
            return 0;
    }
    
    return value;
}

/**
 */
double TimeValue::convertTo(double value, TimeUnit to)
{
    switch(to)
    {
        // All of these fall through until nanoseconds is reached.
        case Seconds:
            value /= 1000;
        case Milliseconds:
            value /= 1000;
        case Microseconds:
            value /= 1000;
        case Nanoseconds:
            break;
            
        case ClockTicks:
//            value /= ONE_SECOND()._nanos / sysClkRateGet();
            value /= ONE_SECOND()._nanos / 100;
            break;
            
        case PBPCounts:
            value /= ONE_SECOND()._nanos / CSPU_CLOCK_RATE;
            break;
            
        default:
            return 0;
    }
    
    return value;
}
