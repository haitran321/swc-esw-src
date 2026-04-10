#ifndef ConfigDataManager_H
#define ConfigDataManager_H

#include <string>
#include <iostream>
#include "Uncopyable.h"
#include "Properties.h"

/**
 * Retrieves Configuration Parameters 
 */
class ConfigDataManager : public Properties, private Uncopyable
{
public:

    /**
     * Returns the singleton object for this class.
     *
     * @return Singleton object for this class
     */
    static ConfigDataManager &getInstance();

    /**
     * Loads SAP data from the SAP file located on RIMS
     *
     * @return Status of operation
     */
    STATUS load(const char* fileName = "/mnt/sd-mmcblk0p1/run/conf/config.txt");
    
    /**
     * Retrieves the SAP named <code>name</code>, placing its contents
     * into <code>value</code> after converting them to type <code>T</code>.
     * <code>T</code> must be able to extract itself from an <code>istrstream</code>
     * via <code>operator>></code>.
     * @return OK if successful, ERROR if not.  Appropriate alerts are generated for failures.
     */
    template <class T> STATUS get(const std::string &name, T &value) const;

protected:

    /**
     * Constructor
     */
    ConfigDataManager();
};

template <class T> STATUS ConfigDataManager::get(const std::string &name, T &value) const
{
    STATUS status = Properties::get(name, value);
    if (status != OK)
    {
        std::cout << "ERROR:  Failed to resolve Config " << name.c_str() << " from " << loadedFrom.c_str() << std::endl;
    }

    return status;
}

#endif /* #ifndef ConfigDataManager_H */
