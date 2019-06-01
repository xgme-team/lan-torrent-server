#ifndef BASE64_HPP
#define BASE64_HPP

#include <string>


std::string b64_encode(const std::string &bin, bool url = false,
                       bool omit_padding = false);
std::string b64_decode(const std::string &str);

#endif // BASE64_HPP
