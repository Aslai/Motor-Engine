#ifndef MOTOR_H_UTIL_UNICODE_HPP
#define MOTOR_H_UTIL_UNICODE_HPP

#include <string>

namespace Motor{
    namespace Unicode{
        std::string ToUTF8(const wchar_t * const & str);
        std::string ToUTF8(const wchar_t * const & str, size_t length);
        std::string ToUTF8(std::wstring const & str);
        std::wstring ToUTF16(std::string const & str);
        std::string ToUTF8(std::wstring const & str);
    }
}

#endif
