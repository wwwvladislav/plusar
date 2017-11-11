#ifndef _environment_hpp_54456_
#define _environment_hpp_54456_
#include "stream.hpp"

namespace plusar
{
    class environment
    {
    public:
        template<typename T, typename Fn>
        stream<T, Fn> source(Fn && source_fn);
    };

    template<typename T, typename Fn>
    stream<T, Fn> environment::source(Fn && source_fn)
    {
        return stream<T, Fn>(std::forward<Fn>(source_fn));
    }
}

#endif
