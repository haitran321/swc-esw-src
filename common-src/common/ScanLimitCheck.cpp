#include <stdio.h>
#include <cmath>
#include "DefineUtils.h"
#include "ScanLimitCheck.h"

#define GAMMA 1.207234
#define CENTER_FREQ 442.0
#define K 533.597428    // (GAMMA*CENTER_FREQ)
#define UV_THRESHOLD 0.8703556
#define W_THRESHOLD 0.333807
#define EL_THRESHOLD 0.01658

int runSWScanLimitCheck(float alpha, float beta)
{
#ifdef PRINT_DEBUG
    printf("****************************************************************\n");
    printf("UV_THRESHOLD = %f, W_THRESHOLD = %f, EL_THRESHOLD = %f\n", UV_THRESHOLD, W_THRESHOLD, EL_THRESHOLD);
#endif

    float u = -beta/K;
    float v = alpha/K;
    float w = sqrt(1 - u*u - v*v);
    float sinEl = v*cos(0.785398) + w*sin(0.785398);   // el_b is boresight elevation at 45 degree in radians

#ifdef PRINT_DEBUG
    printf("alpha = %f, beta = %f, K = %f, u = %f, v = %f, w = %f, sinEl = %f\n", alpha, beta, K, u, v, w, sinEl);
#endif

    // Check against thresholds
    bool slResult = PASSED;
    bool failed = false;
    bool elFailed = false;

    // Check w for NAN
    if (std::isnan(w)) 
    {
        failed = true;
    }
    // Elevation test
    else if (sinEl < EL_THRESHOLD)
    {
        elFailed = true;
    }
    // u, v, w tests
    else if ((abs(u) > UV_THRESHOLD) || (abs(v) > UV_THRESHOLD) || (w < W_THRESHOLD))
    {
        // One of u, v, or w has failed, but elevation passed.  Recompute
        // rounded values of u, v, and w, then check them again.
        float signBeta = (beta > 0.0) ? 1.0 : ((beta < 0.0) ? -1.0 : 0.0);
        float signAlpha = (alpha > 0.0) ? 1.0 : ((alpha < 0.0) ? -1.0 : 0.0);
        u = -(beta - signBeta/2.0)/K;
        v = -(alpha - signAlpha/2.0)/K;
        w = sqrt(1 - u*u - v*v);

        // Re-test u, v and w
        if ((abs(u) > UV_THRESHOLD) || (abs(v) > UV_THRESHOLD) || (w < W_THRESHOLD))
        {
            // Even the rounded values fail.
            failed = true;
        }
    }

    // Set overall test result
    if (failed || elFailed)
    {
        slResult = FAILED;
#ifdef PRINT_DEBUG
        printf("slResult = %d, failed = %d, elFailed = %d\n", slResult, failed, elFailed);
#endif
    }

    return slResult;
}
