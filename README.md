# Plusar

Plusar is an open source stream processing framework.

[![language][badge.language]][language]
[![license][badge.license]][license]

[badge.language]: https://img.shields.io/badge/language-C%2B%2B17-yellow.svg
[badge.license]: https://img.shields.io/badge/license-MIT-blue.svg

[language]: https://en.wikipedia.org/wiki/C%2B%2B17
[license]: https://github.com/wwwVladislav/plusar/blob/master/LICENSE.md

### Features
* The realtime stream processing capabilities.
* Fast as speed of light, rapid like a pulsars jet.
* Different types of stream processing algorithms (map, reduce, etc).

### Simple Example
```cpp
#include <plusar/stream.hpp>
#include <tuple>
#include <iostream>
#include <functional>

int main(int argc, char **argv)
{
    int v = plusar::make_stream([]() { return std::make_optional(std::make_tuple(1, 2)); })
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
Plusar is header only library. So it isn't required to build it. Just copy inlude directory to your own project.
Build files are required only for tests and sample project.
```
mkdir build
cd build
cmake ..
make
```
