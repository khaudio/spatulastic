#include "templateinstantiator.h"

template <typename T>
Foo<T>::Foo()
{
}

template <typename T>
Foo<T>::~Foo()
{
}

template <typename T>
void Foo<T>::print(const char* message)
{
    std::cout << "Template type sizeof(T) is " << sizeof(T) << std::endl;
    std::cout << message << std::endl;
}

/* INSTANTIATE_TEMPLATE_INTS(Foo) */