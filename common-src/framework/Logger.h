#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include "Uncopyable.h"
#include "UDPNetworkDevice.h"

enum LogLevel
{
    Trace   = 1,
    Debug   = 2,
    Info    = 3,
    Error   = 4
};

class Logger : public Uncopyable
{
public:
    /**
     * Destructor
     */
    virtual ~Logger();

    static Logger& getInstance();

    STATUS initialize(const char *sourcePrefix = NULL);

    void logInfo(const char *msg, ...);

    void logDebug(const char *msg, ...);

    void logTrace(const char *msg, ...);

    void logError(const char *msg, ...);

private:

    // Disallow construction
    Logger();

    const char* toString(LogLevel level);

    void log(LogLevel level, const char *msg, va_list *args);

    UDPNetworkDevice *_udpDev;

    int LOG_LEVEL;

    std::string _sourcePrefix;

};


#endif  // LOGGER_H
