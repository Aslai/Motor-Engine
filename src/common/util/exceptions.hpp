#ifndef MOTOR_H_UTIL_EXCEPTIONS_HPP
#define MOTOR_H_UTIL_EXCEPTIONS_HPP

#include <string>
#include <exception>


namespace Motor{
    namespace Exception{
       	class Error : public std::exception {
        public:
            Error(std::string explanation = "");
            virtual const char * what() const noexcept;

        protected:
            std::string explain;
        };

        class NotFound : public Error {
        public:
            NotFound(std::string name = "");
        };
    }
}

#endif
