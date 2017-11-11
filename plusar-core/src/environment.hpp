#ifndef _environment_hpp_54456_
#define _environment_hpp_54456_
#include "stream.hpp"

namespace plusar
{
    class environment
    {
    public:
        template<typename T, typename Fn>
        stream<T, std::decay_t<Fn>> source(Fn && source_fn);
    };

    template<typename T, typename Fn>
    stream<T, std::decay_t<Fn>> environment::source(Fn && source_fn)
    {
        return make_stream<T, std::decay_t<Fn>>(std::forward<std::decay_t<Fn>>(source_fn));
    }
}

#endif
