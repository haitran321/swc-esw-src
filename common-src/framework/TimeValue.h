/**
 * $Id: TimeValue.h 5100 2009-12-04 20:54:54Z nei18232 $
 * @file TimeValue.h
 * @brief Contains interface declarations for class TimeValue.
 */
#ifndef TIME_VALUE_H_INCLUDED
#define TIME_VALUE_H_INCLUDED

#define CSPU_CLOCK_RATE 60e6

//#include <vxWorks.h>

/**
 * @brief Lightweight wrapper for small quantities of time that understands how
 * to convert between different units of measurement.
 * 
 * TimeValue supports these units of measurement:
 * <ul>
 * <li>Nanoseconds</li>
 * <li>Microseconds</li>
 * <li>Milliseconds</li>
 * <li>Seconds</li>
 * <li>VxWorks clock ticks</li>
 * <li>CSPU clock counts</li>
 * </ul>
 * 
 * VxWorks clock ticks are supported through the sysClkRateGet and
 * sysClkRateSet system calls.  CSPU clock counts are supported through the
 * {@link #CSPU_CLOCK_RATE} constant. 
 */
class TimeValue
{
public:

    /** Enumeration of the available units of measurement. */ 
    enum TimeUnit
    {
        Nanoseconds,
        Microseconds,
        Milliseconds,
        Seconds,
        ClockTicks,
        PBPCounts
    };
    
    /**
     * @brief Gets the system clock tick value from the operating system
     * @return System clock tick resolution
     */
    static TimeValue getSystemClockResolution();

    /**
     * @brief Sets the system clock tick value in the operating system.
     * @param resolution System clock tick resolution
     */
    static int setSystemClockResolution(const TimeValue& resolution);
    
    /** CSPU clock rate, in Hz. */
//    static const double CSPU_CLOCK_RATE = 60e6;
    
    /** Constant value of one second. */
    inline static const TimeValue& ONE_SECOND()
    {
        static TimeValue oneSecond(1.0, Seconds);
        return oneSecond;
    }
    
    /**
     * @brief Convert time from one unit of measurement to another.
     * @param value Time value
     * @param from Unit of measurement to convert from
     * @param to Unit of measurement to convert to
     * @return Converted value
     */
    static inline double convert(double value, TimeUnit from, TimeUnit to)
    {
        return convertTo(convertFrom(value, from), to);
    }
    
    /**
     * @brief Convert time from one unit of measurement to another.
     * @param value Time value
     * @param from Unit of measurement to convert from
     * @param to Unit of measurement to convert to
     * @return Converted value
     */
    template<typename T>
    static inline T convert(double value, TimeUnit from, TimeUnit to)
    {
        return static_cast<T>(convertTo(convertFrom(value, from), to));
    }
    
    /** @brief Default constructor.  Creates a 0 time value. */
    TimeValue();
    
    /**
     * @brief Creates a time value using the given units.
     * @param value The time value
     * @param units The units of measurement
     */
    TimeValue(double value, TimeUnit units);
    
    /**
     * @brief Copy constructor.
     * @param other Value to copy
     */
    TimeValue(const TimeValue& other);
    
    /** @brief Destructor. */
    ~TimeValue();
    
    /**
     * @brief Assignment operator.
     * @param other Instance to assign from
     * @return Reference to this object
     */
    TimeValue& operator=(const TimeValue& other);
    
    /**
     * Sets this object's value.
     * @param value The time value
     * @param units The units of measurement
     */
    void set(double value, TimeUnit units);
    
    /**
     * @brief Converts this value to the designated units.
     * @param units The units to convert to
     * @return This value in the designated units
     */
    inline double convertTo(TimeUnit units) const
    {
        return convertTo(_nanos, units);
    }
    
    /**
     * @brief Converts this value to the designated units.
     * @param units The units to convert to
     * @return This value in the designated units
     */
    template<typename T>
    inline T convertTo(TimeUnit units) const
    {
        return static_cast<T>(convertTo(_nanos, units));
    }
    
    ////////////////////////////////////////////////////////////////////////////
    // Mathematical operations
    ////////////////////////////////////////////////////////////////////////////
    
    /**
     * Add this value to another.
     * @param other Other time value to add
     * @return Sum
     */
    inline TimeValue operator+(const TimeValue& other) const
    {
        return _nanos + other._nanos;
    }
    
    /**
     * Subtract another value from this one.
     * @param other Other value to subtract
     * @return Difference
     */
    inline TimeValue operator-(const TimeValue& other) const
    {
        return _nanos - other._nanos;
    }
    
    /**
     * Multiply this value by a scalar
     * @param scalar Scalar value
     * @return Product
     */
    inline TimeValue operator*(double scalar) const
    {
        return _nanos * scalar;
    }
    
    /**
     * Divide this value by a scalar.
     * @param scalar Scalar value
     * @return Quotient
     */
    inline TimeValue operator/(double scalar) const
    {
        return _nanos / scalar;
    }
    
    /**
     * Divide this value by another.
     * @param other Other value
     * @return Ratio
     */
    inline double operator/(const TimeValue& other) const
    {
        return static_cast<double>(_nanos) / other._nanos;
    }
    
    /**
     * Increment this value by another.
     * @param other Other value
     * @return This
     */
    inline TimeValue& operator+=(const TimeValue& other)
    {
        _nanos += other._nanos;
        return *this;
    }
    
    /**
     * Decrement this value by another.
     * @param other Other value
     * @return This
     */
    inline TimeValue& operator-=(const TimeValue& other)
    {
        _nanos -= other._nanos;
        return *this;
    }
    
    /**
     * Scale this value.
     * @param scalar Scalar value
     * @return This
     */
    inline TimeValue& operator*=(double scalar)
    {
        _nanos = static_cast<long>(_nanos * scalar);
        return *this;
    }
    
    /**
     * Scale this value.
     * @param scalar Scalar value
     * @return This
     */
    inline TimeValue& operator/=(double scalar)
    {
        _nanos = static_cast<long>(_nanos / scalar);
        return *this;
    }
    
private:

    /**
     * Creates an instance of TimeValue.
     * @param value Time value in nanoseconds
     */
    TimeValue(double nanos);
    
    /** Primitive data type of the internal representation. */
    typedef long long value_type;

    /** Actual value, in nanoseconds. */
    value_type _nanos;
    
    /**
     * @brief Converts a time from the designated units to nanoseconds.
     * @param value Time value to convert
     * @param from Units being converted from
     * @return Converted time value
     */
    static double convertFrom(double value, TimeUnit from);
    
    /**
     * @brief Converts a time from nanoseconds to the designated units.
     * @param value Time value to convert
     * @param to Units to convert to
     * @return Converted time value
     */
    static double convertTo(double value, TimeUnit to);
};

/**
 * Multiplies a scalar by a time value.
 * @param s Scalar
 * @param t Time value
 * @return Product of the two values
 */
inline TimeValue operator*(double s, const TimeValue& t)
{
    return t * s;
}

#endif // TIME_VALUE_H_INCLUDED
