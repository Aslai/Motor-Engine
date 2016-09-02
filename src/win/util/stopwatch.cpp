#include "util/stopwatch.hpp"
#include <time.h>

Stopwatch::Stopwatch(){

}
void Stopwatch::Start(){
    QueryPerformanceCounter( &start );
}
double Stopwatch::Seconds(){
    LARGE_INTEGER end;
    LARGE_INTEGER freq;
    QueryPerformanceCounter( &end );
    QueryPerformanceFrequency( &freq );
    double ret = (double) (end.QuadPart - start.QuadPart);
    ret /= freq.QuadPart;
    return ret;
}
unsigned long long Stopwatch::Nanoseconds(){
    return (unsigned long long) Seconds() * 1000000000ull;
}
