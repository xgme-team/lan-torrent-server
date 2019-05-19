#include <cstring>

#include <errorhandling.hpp>


const char *basic_error::what() const noexcept
{
    return mWhat;
}

basic_error::basic_error(const char *what) : mWhat(what) {}
assertion_error::assertion_error(const char *what) : basic_error(what) {}
os_error::os_error(const char *what) : basic_error(what) {}
os_file_error::os_file_error(const char *what) : os_error(what) {}

const char *crop_ampersand_and_stdnamespace(const char *str) noexcept
{
    if (str[0] == '&')
        str += 1;
    if (std::strncmp(str, "std::", 5) == 0)
        str += 5;
    return str;
}
