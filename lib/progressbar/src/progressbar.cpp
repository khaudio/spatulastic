#include "progressbar.h"


template <typename T>
RelativeProgress<T>::RelativeProgress() :
_percentageComplete(0.0),
_chunkSize(0.0)
{
}

template <typename T>
RelativeProgress<T>::~RelativeProgress()
{
}


template <typename T>
void RelativeProgress<T>::set(T state)
{
    this->_percentageComplete = state;
}

template <typename T>
void RelativeProgress<T>::set_chunk_size(T chunk)
{
    this->_chunkSize = chunk;
}

template <typename T>
void RelativeProgress<T>::increment(T value)
{
    value = (value ? value : this->_chunkSize);
    this->_percentageComplete += value;
}


template <typename T>
void RelativeProgress<T>::decrement(T value)
{
    value = (value ? value : this->_chunkSize);
    this->_percentageComplete -= value;
}

template <typename T>
T RelativeProgress<T>::get()
{
    return this->_percentageComplete;
}

template <typename T>
bool RelativeProgress<T>::is_complete()
{
    T value = get();
    return (value >= 100.0);
}


template <typename T>
AbsoluteProgress<T>::AbsoluteProgress() :
RelativeProgress<T>(),
_maximum(0),
_state(0),
_absoluteChunkSize(0)
{
}

template <typename T>
AbsoluteProgress<T>::~AbsoluteProgress()
{
}

template <typename T>
bool AbsoluteProgress<T>::_initialized()
{
    return _maximum;
}

template <typename T>
T AbsoluteProgress<T>::_calculate_percentage()
{
    this->_percentageComplete = (static_cast<T>(this->_state) / static_cast<T>(this->_maximum));
    return this->_percentageComplete;
}

template <typename T>
void AbsoluteProgress<T>::set_maximum(size_t maximum)
{
    this->_maximum = maximum;
}

template <typename T>
void AbsoluteProgress<T>::set(size_t state)
{
    this->_state = state;
    _calculate_percentage();
}

template <typename T>
void AbsoluteProgress<T>::set_chunk_size(size_t chunkSize)
{
    this->_absoluteChunkSize = chunkSize;
}

template <typename T>
void AbsoluteProgress<T>::increment(size_t value)
{
    value = (value ? value : this->_absoluteChunkSize);
    this->_state += value;
    _calculate_percentage();
}

template <typename T>
void AbsoluteProgress<T>::decrement(size_t value)
{
    value = (value ? value : this->_absoluteChunkSize);
    this->_state -= value;
    _calculate_percentage();
}

template <typename T>
BasicProgressBar<T>::BasicProgressBar() :
AbsoluteProgress<T>(),
pendingChar('-'),
elapsedChar('#')
{
}

template <typename T>
BasicProgressBar<T>::~BasicProgressBar()
{
}

template <typename T>
void BasicProgressBar<T>::get_bar(char* output, int width)
{

    int elapsed = std::round(RelativeProgress<T>::get() * static_cast<T>(width - 1));
    int remaining = (width - 1) - elapsed;
    int index(0);

    while (index < elapsed)
    {
        output[index++] = this->elapsedChar;
    }
    while (index < remaining)
    {
        output[index++] = this->pendingChar;
    }

    output[width] = '\0';
}

template <typename T>
void BasicProgressBar<T>::print_bar(int width, bool appendReturn)
{
    char output[width];
    get_bar(output, width);
    for (int i(0); i < width; ++i)
    {
        std::cout << output[i];
    }
    if (appendReturn)
    {
        std::cout << "\r";
    }
}

template class RelativeProgress<float>;
template class RelativeProgress<double>;
template class RelativeProgress<long double>;

template class AbsoluteProgress<float>;
template class AbsoluteProgress<double>;
template class AbsoluteProgress<long double>;

template class BasicProgressBar<float>;
template class BasicProgressBar<double>;
template class BasicProgressBar<long double>;

