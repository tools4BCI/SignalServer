#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>

namespace tia
{

//-----------------------------------------------------------------------------
/// removes whitespaces at the beginning and the end of the given string
std::string trim (std::string str)
{
    size_t start_index = 0;
    size_t end_index = str.size () - 1;

    while ((str[start_index] == ' ') && (start_index <= end_index))
        start_index++;

    while ((str[end_index] == ' ') && (end_index >= start_index))
        end_index--;

    return str.substr (start_index, end_index - start_index + 1);
}

}

#endif // STRING_UTILS_H
