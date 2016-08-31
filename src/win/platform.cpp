#include "platform.hpp"
#include "util/unicode.hpp"

namespace Motor{
    #ifdef UNICODE
	std::string TStrToUTF8( const TString & str ){
        return Unicode::ToUTF8( str );
    }
	TString TStrFromUTF8( const std::string & str ){
        return Unicode::ToUTF16( str );
    }
    #else
	std::string TStrToUTF8(const TString & str){
        return str;
    }
	TString TStrFromUTF8(const std::string & str){
        return str;
    }
    #endif
}
