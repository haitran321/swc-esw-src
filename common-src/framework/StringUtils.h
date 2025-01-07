/**
* $Id: StringUtils.h 5100 2009-12-04 20:54:54Z nei18232 $
* 
* Collection of string utility functions
*/

#include <string>
#include <vector>

using std::string;
using std::vector;

/**
 * Trim leading and trailing <code>delimiters</code> from a string.<br/>
 * Defaults to trimming all whitespace.
 */
string trim(const string& str, const string delimeters = " \t\n\r");

/**
 * Tokenize <code>str</code> into <code>tokens</code> as separated by <code>delimiters</code>.<br/>
 * Defaults to tokenizing items delimited by whitespace. 
 */
void tokenize(const string& str, vector<string>& tokens, const string& delimiters = " \t\n\r");


/**
* Truncate everything after endHere and return a new string. If no truncation 
* occurred return an empty string (size == 0).
*/
string truncateString(const string &text, const char endHere);

/**
* Truncate everything after endHere. The string is modified in place. 
* Return the new size of the string or std::string::npos on failure.
*/
unsigned int truncateStringInPlace(std::string &text, const char endHere);

