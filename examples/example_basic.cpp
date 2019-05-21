#include <plusar/stream.hpp>
#include <tuple>
#include <iostream>
#include <functional>
#include <cassert>

int main(int argc, char **argv)
{
    int v = plusar::make_stream([]() { return std::make_optional(std::make_tuple(1, 2)); })
       .skip(10)
       .map([](auto const &t) { return std::get<0>(t) + std::get<1>(t); })
       .take(10)
       .map([](int v) { return v * v; })
       .reduce(0, std::plus<>())
       .collect();

    assert(v == 90);

    std::cout << "make_stream([]() { return plusar::make_optional(std::make_tuple(1, 2)); })\n"
    "   .skip(10)\n"
    "   .map([](auto const &t) { return std::get<0>(t) + std::get<1>(t); })\n"
    "   .take(10)\n"
    "   .map([](int v) { return v * v; })\n"
    "   .reduce(0, std::plus<>())\n"
    "   .collect() == " << v << std::endl;

    return 0;
}
