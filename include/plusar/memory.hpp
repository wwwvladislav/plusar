#pragma once

namespace plusar
{
    template<typename T>
    constexpr T* addressof(T& arg)
    {
        return reinterpret_cast<T*>(
                   &const_cast<char&>(
                      reinterpret_cast<const volatile char&>(arg)));
    }
}
