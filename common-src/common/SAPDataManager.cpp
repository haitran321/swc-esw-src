#include "SAPDataManager.h"

SAPDataManager::SAPDataManager() :
Properties()
{
}

STATUS SAPDataManager::load(const char* fileName)
{
    return Properties::load(fileName);
}

SAPDataManager &SAPDataManager::getInstance()
{
   static SAPDataManager instance;
   return instance;
}



