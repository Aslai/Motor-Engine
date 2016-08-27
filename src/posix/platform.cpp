#include "platform.hpp"

namespace Motor{
    namespace Exception{
        ErrNo::ErrNo( const std::string & info){
            err = errno;
            char buffer[3000];
            explain = info;
            const char * str = strerror_r( errno, buffer, 3000 );
            if( str ){
                explain += " - ";
                explain += str;
            }
        }
    }
}
