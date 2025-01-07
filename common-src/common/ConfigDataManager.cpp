/**
* $Id: ConfigDataManager.cpp 5102 2009-12-04 20:57:39Z nei18232 $
*/

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



