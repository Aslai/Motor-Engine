#ifndef MOTOR_NODEF
#ifndef MOTOR_H_PLATFORM_HPP
#define MOTOR_H_PLATFORM_HPP

extern "C"{
    #include <errno.h>
    #include <string.h>
}
#include "util/exceptions.hpp"

namespace Motor{
    namespace Exception{
        class ErrNo : public Error{
        public:
            ErrNo( const std::string & info = "" );
        private:
            int err;
        };
    }
}

#endif
#endif
