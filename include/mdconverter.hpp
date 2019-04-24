#include <vector>
#include <string>
#include <iostream>

namespace mdc
{

template<template <typename> class C, typename T>
auto convert_collection(const C<T>& md_text) -> decltype(C<T>())
{
    for (auto&& s : md_text)
    {
        std::cout << s << std::endl;
    }

    return md_text;
}

}
