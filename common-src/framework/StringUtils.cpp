/**
* $Id: StringUtils.cpp 5100 2009-12-04 20:54:54Z nei18232 $
*/

#include "StringUtils.h"

string trim(const string& str, const string delimiters)
{
    string r = str;
    r.erase(str.find_last_not_of(delimiters) + 1);
    r.erase(0, r.find_first_not_of(delimiters));
    return r;
}

void tokenize(const string& str, vector<string>& tokens, const string& delimiters)
{
    // Skip delimiters at beginning
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter"
    string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

string truncateString(const string &text, const char endHere)
{
    string str = "";

    unsigned int pos = text.find_last_of(endHere);

    if (pos != string::npos)
    {
        pos++;
        
        str = text;

        str.resize(pos);
    }

    return str;
}

unsigned int truncateStringInPlace(string &text, const char endHere)
{
    unsigned int pos = text.find_last_of(endHere);

    if (pos != string::npos)
    {
        pos++;

        text.resize(pos);
    }

    return pos;
} 
