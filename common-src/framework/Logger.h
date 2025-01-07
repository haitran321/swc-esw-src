#ifndef LOGGER_H
#define LOGGER_H

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

    STATUS initialize();

    void logInfo(char *msg, ...);

    void logDebug(char *msg, ...);

    void logTrace(char *msg, ...);

    void logError(char *msg, ...);

private:

    // Disallow construction
    Logger();

    char* toString(LogLevel level);

    void log(LogLevel level, char *msg, va_list *args);

    UDPNetworkDevice *_udpDev;

    int LOG_LEVEL;

};


#endif  // LOGGER_H
