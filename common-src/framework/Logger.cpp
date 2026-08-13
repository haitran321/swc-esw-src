#include <string.h>
#include <sstream>
#include <iostream>
#include <stdarg.h>
#include <ctime>

#include "Logger.h"
#include "ConfigDataManager.h"
#include "DefineUtils.h"

Logger::Logger() :
    _udpDev(NULL),
    _sourcePrefix("")
{
}

Logger::~Logger()
{
    delete _udpDev;
    _udpDev = NULL;
}

Logger& Logger::getInstance()
{
    static Logger object;
    return (object);
}

STATUS Logger::initialize(const char *sourcePrefix)
{
    STATUS rc = OK;
    ConfigDataManager& configs = ConfigDataManager::getInstance();

    string LOGGER_IP_ADDRESS;
    int LOGGER_PORT;

    // For logger configs
    rc = rc || configs.get("LOGGER_IP_ADDRESS", LOGGER_IP_ADDRESS);
    rc = rc || configs.get("LOGGER_PORT", LOGGER_PORT);
    rc = rc || configs.get("LOG_LEVEL", LOG_LEVEL);

    if (sourcePrefix != NULL)
    {
        _sourcePrefix = sourcePrefix;
    }
    else
    {
        _sourcePrefix.clear();
    }

    // Open logger device
    _udpDev = new UDPNetworkDevice(NetworkClient, LOGGER_IP_ADDRESS, LOGGER_PORT, false);
    stringstream loggerName;
    loggerName << "UDP Logger Client ";
    loggerName << LOGGER_IP_ADDRESS << ":" << LOGGER_PORT;
    _udpDev->setName(loggerName.str());

    if (_udpDev->open() != OK)
    {
        std::cout << "Error openning dev " << _udpDev->getName().c_str() << std::endl;
        rc = rc || ERROR;
    }
    else
    {
        std::cout << "Successfully open Logger UDP device" << std::endl;
    }

    return rc;
}

char* Logger::toString(LogLevel level)
{
    switch (level)
    {
    case Error:
        return "ERROR";
    case Info:
        return "INFO";
    case Debug:
        return "DEBUG";
    case Trace:
        return "TRACE";
    default:
        return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, char *msg, va_list *args)
{
    STATUS rc = OK;
    // Send message to RIMS
    char text[LOGGER_MAX_MSG_SIZE];
    int size;

    // Add time to text
    time_t rawTime;
    struct tm *timeInfo;

    time(&rawTime);
    timeInfo = localtime(&rawTime);

    size = snprintf(text, sizeof text, "%03.3d %02.2d:%02.2d:%02.2d %-5.5s - ",
                    timeInfo->tm_yday,
                    timeInfo->tm_hour,
                    timeInfo->tm_min,
                    timeInfo->tm_sec,
                    toString(level));

    if (!_sourcePrefix.empty())
    {
        size += snprintf(text + size, sizeof text - size, "%s: ", _sourcePrefix.c_str());
    }

    // Add user msg
    size += vsnprintf(text + size, sizeof text - size, msg,*args);

    rc = _udpDev->write(text, size);
    if (rc == ERROR)
    {
        printf("Logger::logInfo(): UDPWrite error\n");
        printf("Logger::logInfo(): Log msg:[%s]\n", msg);
    }
}

void Logger::logInfo(char *msg, ...)
{
    if (LOG_LEVEL <= Info)
    {
        va_list args;
        va_start(args, msg);
        log(Info, msg, &args);
        va_end(args);
    }
}

void Logger::logDebug(char *msg, ...)
{
    if (LOG_LEVEL <= Debug)
    {
        va_list args;
        va_start(args, msg);
        log(Debug, msg, &args);
        va_end(args);
    }
}

void Logger::logTrace(char *msg, ...)
{
    if (LOG_LEVEL <= Trace)
    {
        va_list args;
        va_start(args, msg);
        log(Trace, msg, &args);
        va_end(args);
    }
}

void Logger::logError(char *msg, ...)
{
    if (LOG_LEVEL <= Error)
    {
        va_list args;
        va_start(args, msg);
        log(Error, msg, &args);
        va_end(args);
    }
}

