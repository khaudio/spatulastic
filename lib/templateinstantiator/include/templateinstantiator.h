#ifndef TEMPLATEINSTANTIATOR_H
#define TEMPLATEINSTANTIATOR_H

#include <iostream>

/*
#ifndef INSTANTIATE_TEMPLATE_INTS
#define INSTANTIATE_TEMPLATE_INTS(x) \
template class x<int8_t>; \
template class x<uint8_t>; \
template class x<int16_t>; \
template class x<uint16_t>; \
template class x<int32_t>; \
template class x<uint32_t>; \
template class x<int64_t>; \
template class x<uint64_t>; \
template class x<float>; \
template class x<double>; \
template class x<long double>; \
#ifdef SOMETHING
template class x<int_fast_8>; \
template class x<uint_fast_8>; \
template class x<int_fast_16>; \
template class x<uint_fast_16>; \
template class x<int_fast_32>; \
template class x<uint_fast_32>; \
template class x<int_fast_64>; \
template class x<uint_fast_64>; \
#endif
template class x<char>; \
template class x<char16_t>; \
template class x<char32_t>; \
#endif
*/

template <typename T>
class Foo
{
    Foo();
    ~Foo();
    void print(const char* message);
};

/* INSTANTIATE_TEMPLATE_INTS(Foo) */

#endif