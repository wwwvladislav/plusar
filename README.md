# Plusar

Plusar is an open source stream processing framework.

[![license][badge.license]][license]

[badge.license]: https://img.shields.io/badge/license-MIT-blue.svg

[license]: https://github.com/wwwVladislav/plusar/blob/master/LICENSE.md

### Features
* The realtime stream processing capabilities.
* Fast as speed of light, rapid like a pulsars jet.
* TODO: Good scaling on different cores of CPU.
* TODO: Different types of windows (time, count) for analysis.

### Simple Example
```cpp
#include <core/environment.hpp>
#include <tuple>
#include <iostream>
#include <functional>

int main(int argc, char **argv)
{
    plusar::environment env;

    using tuple2 = std::tuple<int, int>;

    int v = env.source<tuple2>([]() { return plusar::optional<tuple2>(std::make_tuple(1, 2)); })
               .map<int>([](tuple2 const &t) { return std::get<0>(t) + std::get<1>(t); })
               .take(10)
               .map<int>([](int v) { return v * v; })
               .reduce<int>(0, std::plus<>())
               .collect();

    std::cout << v << std::endl;    // Output 90
    return 0;
}
```

## Building Plusar from Source
```
git submodule init
git submodule update
mkdir build
cd build
cmake ..
make
```