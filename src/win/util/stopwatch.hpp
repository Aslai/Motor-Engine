#ifndef MOTOR_H_UTIL_STOPWATCH_HPP
#define MOTOR_H_UTIL_STOPWATCH_HPP

#include "platform.hpp"
#include <ctime>

class Stopwatch{
    LARGE_INTEGER start;
    public:
    Stopwatch();
    void Start();
    double Seconds();
    unsigned long long Nanoseconds();
};

#endif