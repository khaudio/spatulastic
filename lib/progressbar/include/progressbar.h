#include <cstddef>
#include <cmath>


template <typename T = double>
class RelativeProgress
{
protected:
    T
        _percentageComplete,
        _chunkSize;
    
public:
    RelativeProgress();
    ~RelativeProgress();
    
    void set(T state);
    void set_chunk_size(T chunk);
    
    void increment(T value = 0.0);
    void decrement(T value = 0.0);
    
    T get();
    bool is_complete();

};


template <typename T = double>
class AbsoluteProgress : public RelativeProgress<T>
{
protected:
    size_t _maximum, _state, _absoluteChunkSize;
    
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
    
    void print(char* message, int width = 80);
};
