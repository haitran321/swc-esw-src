/**
 * $Id: Properties.cpp 6125 2010-07-12 22:07:50Z ste38548 $
 */
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <iostream>
#include "Properties.h"
#include "StringUtils.h"

using std::string;
using std::stringstream;
using std::ifstream;
using std::boolalpha;
using std::map;

STATUS Properties::load(const char* fileName)
{
    loadedFrom = fileName; // stash where we were loaded from for diagnostics

    string line;
    string section = "";
    int lineNo = 0;

    ifstream infile(fileName);
    if (infile.fail())
    {
        // Attempt a second time to open the file...
        infile.open(fileName);
        if (infile.fail())
        {
            printf("Could not open configuration file [%s] (attempted to open "
                   "twice).\n", fileName);
            return ERROR;
        }
    }

    // Read configuration file
    while (!infile.eof())
    {
        // get next line
        getline(infile, line);
        lineNo++;

        // Process line, skipping blank lines and lines that have
        // a '#' in first column
        if (line.length() > 1 && line[0] != '#')
        {
            if (line.length() > 8 && line.substr(0, 8) == "include ")
            {
                // an include directive, so include the specified file
                if (include(line.substr(9), fileName) != OK)
                    break;
                continue;
            }

            // extract keys and values
            string::size_type pos = line.find("=");
            if (pos == string::npos)
            {
                // found a line w/o an equals, so treat it as the start of a section
                section = trim(line);

                size_t extends = section.find(" extends ");
                if (extends != string::npos)
                {
                    // has an extends keyword, so pre-populate this section with base's keys/values
                    string base = trim(section.substr(extends + 9));
                    section = trim(section.substr(0, extends));
                    extend(section, base);
                }

                continue;
            }

            string key = line.substr(0, pos);
            key = trim(key);
            string value = line.substr(pos + 1);

            // if the line starts with spaces then treat the previous
            // entry as the start of a section and all of the indented
            // key/value pairs are part of that section
            if ((line[0] == ' ' || line[0] == '\t') && section != "")
            {
                if (key[0] != '.')
                    key = "." + key;
                key = section + key;
            }
            else
            {
                section = "";
            }

            // store the key/value pair
            props[key] = expand(trim(value));
        }
    }

    infile.close();

    return OK;
}


/**
 * Utility function that returns <code>true</code> if <code>filename</code> exists
 */
bool fexists(const char *filename)
{
    struct stat buffer;
    return stat(filename, &buffer);
}

STATUS Properties::include(const string& include, const char* from)
{
    string original = loadedFrom;
    STATUS status;

    if (fexists(include.c_str()))
    {
        // the filename specified exists, so load it
        status = load(include.c_str());
    }
    else
    {
        // else try to load it from the directory that "from" was loaded from
        const char* lastSlash = strrchr(from, '/');
        if (lastSlash == NULL)
        {
            lastSlash = strrchr(from, '\\');
        }

        if (lastSlash != NULL)
        {
            string path(from, (lastSlash - from) + 1);
            path.append(include);
            status = load(path.c_str());
        }
        else
        {
            std::cout << "Could not open configuration file " << include.c_str() << " referenced in " << from << std::endl;
            status = ERROR;
        }
    }

    loadedFrom = original;
    return status;
}

const string Properties::expand(const string& value)
{
    // search for opening ${
    size_t opening = value.find("${");
    if (opening != string::npos)
    {
        // search for closing }
        size_t closing = value.find('}', opening);
        if (closing != string::npos)
        {
            // take name of property between ${ and } and replace it with its contents
            const string propName = trim(value.substr(opening + 2, closing - opening - 2));
            string propValue;
            if (get(propName, propValue) != ERROR)
            {
                string expanded = value;
                expanded.replace(opening, closing + 1, propValue);
                return expanded;
            }
        }
    }

    return value;
}

void Properties::extend(const string& section, string base)
{
    // match base only and not something that happens to start with it
    base.append(".");

    // iterate through the properties to find any properties in base
    PropertiesMap::const_iterator iter;
    for (iter = props.begin(); iter != props.end(); ++iter)
    {
        const string& key = (*iter).first;
        if (key.substr(0, base.length()) == base)
        {
            // found a property in base, so copy its value to the same property name in section
            string clonedKey = section + "." + key.substr(base.length());
            props[clonedKey] = (*iter).second;
        }
    }
}

STATUS Properties::get(const string& key, string& value) const
{
    // pull key out of the map
    PropertiesMap::const_iterator iter = props.find(key);
    if (iter == props.end())
    {
        // not in map.  bail
        return ERROR;
    }

    value = iter->second;

    return OK;
}

STATUS Properties::get(const string& key, bool& value) const
{
    // pull key out of the map
    PropertiesMap::const_iterator iter = props.find(key);
    if (iter == props.end())
    {
        // not in map.  bail
        return ERROR;
    }

    // parse the associated value, plugging the results into 'value'
    // after translating '1' and '0'
    stringstream stream(iter->second);
    stream >> value;
    if (stream.fail())
    {
        // clear/reset all string status bits
        stream.clear(); 
        
        // failed parsing as '1' or '0', so try again looking for 'true' or 'false'
        stream.str(iter->second);
        stream >> boolalpha >> value;
        
        if (stream.fail())
        {
            // failed to parse.  bail
            return ERROR;
        }
    }

    return OK;
}

Properties::PropertiesMap::const_iterator Properties::begin()
{
    return props.begin();
}

Properties::PropertiesMap::const_iterator Properties::end()
{
    return props.end();
}
