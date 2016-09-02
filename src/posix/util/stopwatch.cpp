#include "util/stopwatch.hpp"
#include <time.h>

Stopwatch::Stopwatch(){

}
void Stopwatch::Start(){
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
}
double Stopwatch::Seconds(){
    struct timespec tr, tn;
    clock_getres(CLOCK_THREAD_CPUTIME_ID, &tr);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tn);
    double s = (tn.tv_sec - start.tv_sec);
    s *= 1000000000;
    s += tn.tv_nsec;
    s -= start.tv_nsec;
    s /= 1000000000;
    return s;
}
unsigned long long Stopwatch::Nanoseconds(){
    struct timespec tr, tn;
    clock_getres(CLOCK_THREAD_CPUTIME_ID, &tr);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tn);
    unsigned long long s = (tn.tv_sec - start.tv_sec);
    s *= 1000000000;
    s += tn.tv_nsec;
    s -= start.tv_nsec;
    return s;
}
