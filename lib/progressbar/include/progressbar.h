#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <cstddef>
#include <cmath>
#include <iostream>
#include <iomanip>


template <typename T = double>
class RelativeProgress
{
protected:
    T
        _relativeState,
        _relativeChunkSize;
    
public:
    RelativeProgress();
    ~RelativeProgress();
    
    void set(T state);
    void set_chunk_size(T chunk);
    
    void increment(T value = 0.0);
    void decrement(T value = 0.0);
    
    T get();
    T get_percentage();
    bool is_complete();

};


template <typename T = double>
class AbsoluteProgress : public RelativeProgress<T>
{
protected:
public:
    size_t
        _absoluteMaximum,
        _absoluteState,
        _absoluteChunkSize;
    
    bool _initialized();
    T _calculate_percentage();
    
public:
    AbsoluteProgress();
    ~AbsoluteProgress();
    
    void set_maximum(size_t maximum);
    
    void set(size_t state);
    void set_chunk_size(size_t chunkSize);
    
    void increment(size_t value = 0);
    void decrement(size_t value = 0);
};


template <typename T = double>
class BasicProgressBar : public AbsoluteProgress<T>
{
protected:
    char pendingChar, elapsedChar;
    
public:
    BasicProgressBar();
    ~BasicProgressBar();
    
    void get_bar(char* message, int width = 80);
    void print_bar(int width = 80, bool appendReturn = true);
};

#endif
