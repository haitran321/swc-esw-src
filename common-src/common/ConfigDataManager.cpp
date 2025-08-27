#include "ConfigDataManager.h"

ConfigDataManager::ConfigDataManager() :
Properties()
{
}

STATUS ConfigDataManager::load(const char* fileName)
{
    return Properties::load(fileName);
}

ConfigDataManager &ConfigDataManager::getInstance()
{
   static ConfigDataManager instance;
   return instance;
}



