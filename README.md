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
#include <plusar/stream.hpp>
#include <tuple>
#include <iostream>
#include <functional>

int main(int argc, char **argv)
{
    int v = plusar::make_stream([]() { return plusar::make_optional(std::make_tuple(1, 2)); })
       .map([](auto const &t) { return std::get<0>(t) + std::get<1>(t); })
       .take(10)
       .map([](int v) { return v * v; })
       .reduce(0, std::plus<>())
       .collect();

    std::cout << v << std::endl;    // Output 90
    return 0;
}
```

## Building Plusar from Source
```
mkdir build
cd build
cmake ..
make
```
