#include <cstring>

#include <errorhandling.hpp>


const char *crop_ampersand_and_stdnamespace(const char *str) noexcept
{
    if (str[0] == '&')
        str += 1;
    if (std::strncmp(str, "std::", 5) == 0)
        str += 5;
    return str;
}
