#include <string>
#include <exception>
#include "exceptions.hpp"

namespace Motor{
    namespace Exception{
        Error::Error(std::string explanation = "") : explain(explanation){
            for (size_t i = 0; i < explain.size();){
                if (explain[i] == '\r'){
                    explain.erase(explain.begin() + i, explain.begin() + i + 1);
                    continue;
                }
                ++i;
            }
        }

        const char * Error::what() const{
            return explain.c_str();
        }

        NotFound::NotFound(std::string name = "") :
            Error("\nCould not find file \"" + name + "\""){

        }
    }
}

#endif
