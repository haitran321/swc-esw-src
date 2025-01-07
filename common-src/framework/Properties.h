/**
 * $Id: Properties.h 5100 2009-12-04 20:54:54Z nei18232 $
 *
 * <code>Properties</code> represents a persistent set of
 * properties loaded from a file.  The file is expected to
 * be in traditional <code>key=value</code> format.
 *
 * The load logic was "stolen" from Dennis Alverson's SAPDataManager.
 */
#ifndef Properties_H
#define Properties_H

#include <string>
#include <map>
#include <strstream>
#include "DefineUtils.h"

class Properties
{
public:
    /**
     * Constructor
     */
    Properties() : loadedFrom("")
    {
    };

    /**
     * Destructor
     */
    virtual ~Properties()
    {
    };

    /**
     * Load the contents of the specified file.
     *
     * @return Status of operation.  Details of failures written to stdout.
     */
    STATUS load(const char* fileName);

    /**
     * Retrieves the configuration item named <code>name</code>, placing its contents
     * into <code>value</code> after converting them to type <code>T</code>.
     * <code>T</code> must be able to extract itself from an <code>istrstream</code>
     * via <code>operator>></code>.
     *
     * @return OK if successful, ERROR if not.  Error reporting is left to the caller.
     */
    template <class T> STATUS get(const std::string &key, T &value) const
    {
        // pull key out of the map
        std::map<std::string, std::string>::const_iterator iter = props.find(key);
        if (iter == props.end())
        {
            // not in map.  bail
            return ERROR;
        }

        // parse the associated value, plugging the results into 'value'
        std::istrstream stream(iter->second.c_str());
        stream >> value;
        if (stream.fail())
        {
            // failed to parse.  bail
            return ERROR;
        }

        return OK;
    }

    /**
     * Specialization (optimization) of get() for strings
     */
    STATUS get(const std::string &key, std::string &value) const;

    /**
     * Specialization of get() for booleans (translates "true"/"false" values)
     */
    STATUS get(const std::string &key, bool &value) const;

    /**
     * Type of the internal map
     */
    typedef std::map<std::string, std::string> PropertiesMap;

    /**
     * Allow those that need properties w/o knowing their names
     * get at the beginning of the map.
     */
    PropertiesMap::const_iterator begin();

    /**
     * Allow those that need properties w/o knowing their names
     * get at the end of the map.
     */
    PropertiesMap::const_iterator end();

    /**
     * For diagnostic purposed only.<p/>
     * Returns the path from which this properties was initially loaded from.
     */
    const std::string& getLoadedFrom() const
    {
        return loadedFrom;
    }

protected:
    /**
     * An include directive was found in <code>from</code> that specified
     * to include the data from <code>include</code>.
     */
    STATUS include(const std::string& include, const char* from);

    /**
     * Expands value to include contents of any keys referenced by name with ${key} notation.
     */
    const std::string expand(const std::string& value);

    /**
     * Section <code>section</code> extends (gets copies of all of the properties of)
     * <code>base</code>.
     */
    void extend(const std::string& section, std::string base);

    /**
     * file that we were loaded from.  for diagnostic purposes.
     */
    std::string loadedFrom;

    /**
     * Collection of the string-based properties backing this <code>Properties</code> instance
     */
    PropertiesMap props;
};
#endif  // Properties_H
