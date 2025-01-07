
#include "InternalMsgTypes.h"

/// Pulse samples  for each of the 11 CSPU pulse types.
const short NUM_PULSE_SAMPLES[MAX_PULSE_TYPES] =
{
    0,
    0,
    0,
    20,
    40,
    0,
    80,
    0,
    16,
    160,
    31,
    312,
};

/// Pulse width (sec) for each of the 11 CSPU pulse types.
const float PULSE_WIDTHS[MAX_PULSE_TYPES] =
{
    0.0,
    1.0e-6,
    10.0e-6,
    16.0e-6,
    25e-6,
    32.0e-6,
    64.0e-6,
    125.0e-6,
    128.0e-6,
    128.0e-6,
    250.0e-6,
    250.0e-6,
    // New waveforms
//  25.0e-6,
//  1000.0e-6,
//  1000.0e-6,
//  1000.0e-6
};

/// Sample rate (Hz) for each of the 11 CSPU pulse types.
const float SAMPLE_RATE[MAX_PULSE_TYPES] =
{
    0.0,
    1.25e6,
    125.0e3,
    1.25e6,
    0.0,
    1.25e6,
    1.25e6,
    0.0,
    125.0e3,
    1.25e6,
    125.0e3,
    1.25e6,
    // New waveforms
//  5.0e6,
//  625.0e3,    // if we ended using the 1.25MHz data for this, then this should be 1.25e6/2=625e3
//  1.25e6,
//  5.0e6
};

/// Sample period (usec) for each of the 11 CSPU pulse types.
const float SAMPLE_PERIOD[MAX_PULSE_TYPES] =
{
    1.0,
    1.0e6 / 1.25e6,
    1.0e6 / 125.0e3,
    1.0e6 / 1.25e6,
    1.0,
    1.0e6 / 1.25e6,
    1.0e6 / 1.25e6,
    1.0,
    1.0e6 / 125.0e3,
    1.0e6 / 1.25e6,
    1.0e6 / 125.0e3,
    1.0e6 / 1.25e6,
    // New waveforms
//  1.0e6 / 5.0e6,      // 0.2us/sample
//  1.0e6 / 625.0e3,    // 1.6us/sample
//  1.0e6 / 1.25e6,     // 0.8us/sample
//  1.0e6 / 5.0e6       // 0.us/sample

};

