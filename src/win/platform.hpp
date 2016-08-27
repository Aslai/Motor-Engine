#ifndef MOTOR_H_PLATFORM_HPP
#define MOTOR_H_PLATFORM_HPP

#include <windows.h>
#include <string>

namespace Motor{
    #ifdef UNICODE
    std::string TStrToUTF8( const std::wstring str & );
    std::wstring TStrFromUTF8( const std::string str & );
    #else
    std::string TStrToUTF8( const std::string str & );
    std::string TStrFromUTF8( const std::string str & );
    #endif
}

#endif
