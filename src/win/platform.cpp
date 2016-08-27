#include "platform.hpp"
#include "util/unicode.hpp"

namespace Motor{
    #ifdef UNICODE
    std::string TStrToUTF8( const std::wstring str & ){
        return Unicode::ToUTF8( str );
    }
    std::wstring TStrFromUTF8( const std::string str & ){
        return Unicode::ToUTF16( str );
    }
    #else
    std::string TStrToUTF8( const std::string str & ){
        return str;
    }
    std::string TStrFromUTF8( const std::string str & ){
        return str;
    }
    #endif
}
