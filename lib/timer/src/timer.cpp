#include "timer.h"

Timer::Timer() :
_counter(0),
_duration(0),
_sum(0),
_mean(0)
{
    _start();
}

Timer::Timer(const Timer& obj) :
_counter(obj._counter),
_duration(obj._duration),
_sum(obj._sum),
_mean(obj._mean)
{
    _start();
}

Timer::~Timer()
{
}

inline void Timer::_start()
{
    this->_startPoint = std::chrono::high_resolution_clock::now();
}

inline void Timer::_stop()
{
    this->_stopPoint = std::chrono::high_resolution_clock::now();
    this->_duration = this->_stopPoint - this->_startPoint;
    this->_sum += this->_duration;
    ++this->_counter;
}

inline void Timer::_avg()
{
    this->_mean = this->_sum / this->_counter;
}

void Timer::start()
{
    #if _DEBUG
    std::cout << "Starting timer" << std::endl;
    #endif
    _start();
}

void Timer::stop()
{
    #if _DEBUG
    std::cout << "Stopping timer" << std::endl;
    #endif
    _stop();
    _avg();
}

void Timer::tick()
{
    #if _DEBUG
    std::cout << "Ticking timer" << std::endl;
    #endif
    _stop();
    _avg();
    _start();
}

void Timer::reset()
{
    #if _DEBUG
    std::cout << "Resetting timer" << std::endl;
    #endif
    this->_counter = 0;
    this->_sum = std::chrono::high_resolution_clock::duration(0);
    this->_duration = std::chrono::high_resolution_clock::duration(0);
    this->_mean = std::chrono::high_resolution_clock::duration(0);
}

std::chrono::high_resolution_clock::duration Timer::get()
{
    return this->_mean;
}

double Timer::get_ns()
{
    return static_cast<double>(this->_mean.count());
}

double Timer::get_ms()
{
    return static_cast<double>(this->_mean.count()) / 1e6;
}

double Timer::get_s()
{
    return static_cast<double>(this->_mean.count()) / 1e9;
}

void Timer::print_ns(const char* action)
{
    std::cout << ((action == nullptr) ? "Timer" : action);
    std::cout << ((this->_counter > 1) ? " averaged " : " took ");
    std::cout << std::fixed << std::setprecision(4);
    std::cout << get_ns() << " ns" << std::endl;
}

void Timer::print_ms(const char* action)
{
    std::cout << ((action == nullptr) ? "Timer" : action);
    std::cout << ((this->_counter > 1) ? " averaged " : " took ");
    std::cout << std::fixed << std::setprecision(4);
    std::cout << get_ms() << " ms" << std::endl;
}

void Timer::print_s(const char* action)
{
    std::cout << ((action == nullptr) ? "Timer" : action);
    std::cout << ((this->_counter > 1) ? " averaged " : " took ");
    std::cout << std::fixed << std::setprecision(4);
    std::cout << get_s() << " s" << std::endl;
}



