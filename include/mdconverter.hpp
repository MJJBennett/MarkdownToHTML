#include <vector>
#include <string>
#include <optional>
#include <iostream>

namespace mdc
{

char header_number_to_char(const unsigned int i) 
{
    switch (i) 
    {
    case 1: return '1';
    case 2: return '2';
    case 3: return '3';
    case 4: return '4';
    case 5: return '5';
    default: return '6';
    }
}

template<typename T>
struct MDHandler
{
    virtual T header1_start() { return "<h1>"; }
    virtual T header1_end() { return "</h1>"; }
    virtual T header2_start() { return "<h1>"; }
    virtual T header2_end() { return "</h1>"; }
    virtual T header3_start() { return "<h1>"; }
    virtual T header3_end() { return "</h1>"; }
    virtual T header4_start() { return "<h1>"; }
    virtual T header4_end() { return "</h1>"; }
    virtual T header5_start() { return "<h1>"; }
    virtual T header5_end() { return "</h1>"; }
    virtual T header6_start() { return "<h1>"; }
    virtual T header6_end() { return "</h1>"; }
};

struct State
{
    
};

template<template <typename> class C, typename T, typename H>
auto convert_collection(const C<T>& md_text, const H& handler) -> decltype(C<T>())
{
    C<T> html_text;
    std::optional<State> state{};
    for (auto&& s : md_text)
    {
        const auto res = convert(s, handler, state);
        std::cout << "(" << res << ")" << std::endl;
        html_text.emplace_back(res);
    }

    return html_text;
}

template<typename T, typename H>
auto convert(const T& md_text, const H& handler, std::optional<State>& state) -> T
{
    unsigned short header = 0;
    T html_text;

    auto itr = md_text.begin();

    while (*itr == '#')
    {
        header++;
        itr++;
    }
    if (header)
    {
        if (*itr == ' ' && header < 7)
        {
            html_text += header_number_to_char(header);
            itr++;
        }
        else
        {
            for (int i = 0; i < header; i++) html_text += '#';
        }
    }

    return html_text;
}

}
