#ifndef _environment_hpp_54456_
#define _environment_hpp_54456_
#include "stream.hpp"

namespace plusar
{
    class environment
    {
    public:
        template<typename T, typename Fn>
        stream<T, std::decay_t<Fn>> source(Fn && fn);
    };

    template<typename T, typename Fn>
    stream<T, std::decay_t<Fn>> environment::source(Fn && fn)
    {
        return make_stream(std::forward<Fn>(fn));
    }
}

#endif
