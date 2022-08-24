#include "progressbar.h"


template <typename T>
RelativeProgress<T>::RelativeProgress() :
_relativeState(0.0),
_relativeChunkSize(0.0)
{
}

template <typename T>
RelativeProgress<T>::~RelativeProgress()
{
}


template <typename T>
void RelativeProgress<T>::set(T state)
{
    this->_relativeState = state;
}

template <typename T>
void RelativeProgress<T>::set_chunk_size(T chunk)
{
    this->_relativeChunkSize = chunk;
}

template <typename T>
void RelativeProgress<T>::increment(T value)
{
    value = (value ? value : this->_relativeChunkSize);
    this->_relativeState += value;
}


template <typename T>
void RelativeProgress<T>::decrement(T value)
{
    value = (value ? value : this->_relativeChunkSize);
    this->_relativeState -= value;
}

template <typename T>
T RelativeProgress<T>::get()
{
    return this->_relativeState;
}

template <typename T>
T RelativeProgress<T>::get_percentage()
{
    return (this->_relativeState * 100);
}

template <typename T>
bool RelativeProgress<T>::is_complete()
{
    return (get() >= 1.0);
}

template <typename T>
AbsoluteProgress<T>::AbsoluteProgress() :
RelativeProgress<T>(),
_absoluteMaximum(0),
_absoluteState(0),
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
    return _absoluteMaximum;
}

template <typename T>
T AbsoluteProgress<T>::_calculate_percentage()
{
    this->_relativeState = (
            static_cast<T>(this->_absoluteState)
            / static_cast<T>(this->_absoluteMaximum)
        );
    return this->_relativeState;
}

template <typename T>
void AbsoluteProgress<T>::set_maximum(size_t maximum)
{
    this->_absoluteMaximum = maximum;
}

template <typename T>
void AbsoluteProgress<T>::set(size_t state)
{
    this->_absoluteState = state;
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
    this->_absoluteState += value;
    _calculate_percentage();
}

template <typename T>
void AbsoluteProgress<T>::decrement(size_t value)
{
    value = (value ? value : this->_absoluteChunkSize);
    this->_absoluteState -= value;
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
    int elapsed = std::round(RelativeProgress<T>::get() * static_cast<T>(width ));
    int remaining = (width) - elapsed;
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
    char* output = new char[width + 1];
    get_bar(output, width + 1);
    for (int i(0); i < width; ++i)
    {
        std::cout << output[i];
    }
    if (appendReturn)
    {
        std::cout << "\r";
    }
    delete output;
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

