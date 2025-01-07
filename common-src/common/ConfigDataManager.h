/** 
 *  $Id: ConfigDataManager.h 5102 2009-12-04 20:57:39Z nei18232 $
 *  
 *  ConfigDataManager exists to obtain Configuration
 *  (non-SAP) data values from the FTP Server.  This class
 *  simply extends the Properties class and only
 *  implements its own Singleton and load() to set the filename.
 *  
 */

#ifndef ConfigDataManager_H
#define ConfigDataManager_H

#include <string>
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
    STATUS load(const char* fileName = "/mnt/sd-mmcblk0p1/conf/config.txt");
    
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
        printf("ERROR:  Failed to resolve Config [%s] from %s\n",
                    name.c_str(), loadedFrom.c_str());
    }

    return status;
}

#endif /* #ifndef ConfigDataManager_H */
