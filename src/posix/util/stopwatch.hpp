#ifndef MOTOR_H_UTIL_STOPWATCH_HPP
#define MOTOR_H_UTIL_STOPWATCH_HPP

#include <ctime>
class Stopwatch{
    struct timespec start;
    public:
    Stopwatch();
    void Start();
    double Seconds();
    unsigned long long Nanoseconds();
};

#endif