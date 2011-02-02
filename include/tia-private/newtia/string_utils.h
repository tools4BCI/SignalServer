#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "tia_exceptions.h"

#include <string>
#include <sstream>

namespace tia
{

//-----------------------------------------------------------------------------
/// removes whitespaces at the beginning and the end of the given string
inline std::string trim (std::string str)
{
    size_t start_index = 0;
    size_t end_index = str.size () - 1;

    while ((str[start_index] == ' ') && (start_index <= end_index))
        start_index++;

    while ((str[end_index] == ' ') && (end_index >= start_index))
        end_index--;

    return str.substr (start_index, end_index - start_index + 1);
}

//-----------------------------------------------------------------------------
inline unsigned toUnsigned (std::string const& str)
{
    std::istringstream iss (str);
    unsigned value;
    iss >> value;
    if (iss.fail())
        throw TiAException (std::string ("Failed to convert \"") + str + "\" into an unsigned int.");
    return value;
}

//-----------------------------------------------------------------------------
inline std::string toString (unsigned number)
{
    std::ostringstream oss;
    oss << std::dec << number;
    if (oss.fail())
        throw TiAException ("Failed to a number into a string.");
    return oss.str ();
}


}

#endif // STRING_UTILS_H
