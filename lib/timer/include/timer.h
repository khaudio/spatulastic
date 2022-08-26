#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>
#include <iomanip>

class Timer
{
protected:
    uint_fast32_t _counter;
    std::chrono::high_resolution_clock::duration _duration, _sum, _mean;
    std::chrono::high_resolution_clock::time_point _startPoint, _stopPoint;

    void _start();
    void _stop();
    void _avg();

public:
    Timer();
    ~Timer();
    Timer(const Timer& obj);

    void start();
    void stop();
    void tick();
    void reset();

    std::chrono::high_resolution_clock::duration get();
    double get_ns();
    double get_ms();
    double get_s();

    void print_ns(const char* action = nullptr);
    void print_ms(const char* action = nullptr);
    void print_s(const char* action = nullptr);
};

#endif
