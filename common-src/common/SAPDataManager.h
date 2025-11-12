#ifndef SAPDataManager_H
#define SAPDataManager_H

#include <string>
#include "Uncopyable.h"
#include "Properties.h"

/**
 * Retrieves Configuration Parameters 
 */
class SAPDataManager : public Properties, private Uncopyable
{
public:

    /**
     * Returns the singleton object for this class.
     *
     * @return Singleton object for this class
     */
    static SAPDataManager &getInstance();

    /**
     * Loads SAP data from the SAP file located on RIMS
     *
     * @return Status of operation
     */
    STATUS load(const char* fileName = "/mnt/sd-mmcblk0p1/conf/sap.txt");
    
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
    SAPDataManager();
};

template <class T> STATUS SAPDataManager::get(const std::string &name, T &value) const
{
    STATUS status = Properties::get(name, value);
    if (status != OK)
    {
        printf("ERROR:  Failed to resolve SAP [%s] from %s\n",
                    name.c_str(), loadedFrom.c_str());
    }

    return status;
}

#endif /* #ifndef SAPDataManager_H */
