#include <core/environment.hpp>
#include <tuple>
#include <iostream>
#include <functional>
#include <cassert>

int main(int argc, char **argv)
{
    plusar::environment env;

    using tuple2 = std::tuple<int, int>;

    int v = env.source<tuple2>([]() { return std::make_tuple(1, 2); })
       .map<int>([](tuple2 const &t) { return std::get<0>(t) + std::get<1>(t); })
       .take(10)
       .map<int>([](int v) { return v * v; })
       .reduce<int>(0, std::plus<>())
       .collect();

    assert(v == 90);
    return 0;
}
