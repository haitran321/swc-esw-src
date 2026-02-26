#include <unistd.h>     // for sleep()
#include <cmath>
#include "IOMHWMgr.h"
#include "DeviceUtilities.h"
#include "ConfigDataManager.h"

IOMHWMgr::IOMHWMgr() :
_logger(Logger::getInstance())
{
}

IOMHWMgr::~IOMHWMgr()
{
}

IOMHWMgr &IOMHWMgr::getInstance()
{
    static IOMHWMgr object;
    return (object);
}

void IOMHWMgr::close()
{
}

STATUS IOMHWMgr::initialize()
{
    STATUS rc = OK;

    _logger.logInfo("IOHWMgr Initializing");

    // Get config parameters
    ConfigDataManager &configs = ConfigDataManager::getInstance();
//  rc = rc || configs.get("FORCE_TEST_MODE", FORCE_TEST_MODE);

    return rc;
}
