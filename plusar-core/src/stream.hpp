#ifndef _stream_hpp_54456_
#define _stream_hpp_54456_
#include <experimental/optional>
#include <type_traits>

namespace plusar
{
    template<typename T, typename Fn, typename = typename std::enable_if<std::is_same<Fn, std::decay_t<Fn>>::value, bool>::type>
    class stream;

    template<typename T, typename Fn, typename = typename std::enable_if<std::is_same<Fn, std::decay_t<Fn>>::value, bool>::type>
    constexpr stream<T, std::decay_t<Fn>> make_stream(Fn && fn);

    template<typename T, typename Fn, typename>
    class stream
    {
        template<typename OptT>
        using optional = std::experimental::optional<OptT>;
        static constexpr auto nullopt = std::experimental::nullopt;

        Fn _fn;
        mutable size_t _count = 0;

        stream() = delete;
        stream & operator = (stream const &) = delete;
        stream & operator = (stream &&) = delete;

    public:
        stream(stream &&) = default;
        stream(stream const &) = default;
        ~stream() = default;

        stream(Fn && fn):
            _fn(fn)
        {}

        template<typename R, typename FnR>
        constexpr auto map(FnR && fn) const
        {
            return make_stream<R>([this, fn]()
            {
                optional<T> sv = next();
                return sv ? optional<R>(fn(*sv)) : nullopt;
            });
        }


        template<typename R, typename FnR>
        constexpr auto reduce(R const &v, FnR && fn) const
        {
            return make_stream<R>([this, v, fn]()
            {
                R res = v;
                for(optional<T> v = next(); v; v = next())
                    res = fn(res, *v);
                return optional<R>(res);
            });
        }

        constexpr auto take(size_t limit) const
        {
            return make_stream<T>([this, limit] () -> optional<T>
            {
                if (_count >= limit)
                    return nullopt;
                return next();
            });
        }

        template<class OutputIt>
        constexpr void collect(OutputIt it) const
        {
            for(optional<T> v = next(); v; v = next())
                ++it = *v;
        }

        constexpr T collect() const
        {
            return *next();
        }

        constexpr optional<T> next() const
        {
            optional<T> v = _fn();
            if (v) ++_count;
            return v;
        }
    };

    template<typename T, typename Fn, typename = typename std::enable_if<std::is_same<Fn, std::decay_t<Fn>>::value, bool>::type>
    constexpr stream<T, std::decay_t<Fn>> make_stream(Fn && fn)
    {
        return stream<T, std::decay_t<Fn>>(std::forward<std::decay_t<Fn>>(fn));
    }
}

#endif
