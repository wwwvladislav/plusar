#ifndef _stream_hpp_54456_
#define _stream_hpp_54456_
#include <experimental/optional>
#include <type_traits>

namespace plusar
{
    template<typename T, typename Fn>
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
        stream(Fn && fn);
        ~stream() = default;

        template<typename R, typename FnR>
        constexpr auto map(FnR && fn) const;

        template<typename R, typename FnR>
        constexpr auto reduce(R const &v, FnR && fn) const;

        constexpr auto take(size_t limit) const;

        template<class OutputIt>
        constexpr void collect(OutputIt it) const;

        constexpr T collect() const;

        constexpr optional<T> next() const;
    };

    template<typename T, typename Fn>
    constexpr stream<T, Fn> make_stream(Fn && fn)
    {
        return stream<T, typename std::decay<Fn>::type>(std::forward<typename std::decay<Fn>::type>(fn));
    }

    template<typename T, typename Fn>
    stream<T, Fn>::stream(Fn && fn):
        _fn(fn)
    {}

    template<typename T, typename Fn>
    template<typename R, typename FnR>
    constexpr auto stream<T, Fn>::map(FnR && fn) const
    {
        return make_stream<R>([this, fn]()
        {
            optional<T> sv = next();
            return sv ? optional<R>(fn(*sv)) : nullopt;
        });
    }

    template<typename T, typename Fn>
    template<typename R, typename FnR>
    constexpr auto stream<T, Fn>::reduce(R const &s, FnR && fn) const
    {
        return make_stream<R>([this, s, fn]()
        {
            R res = s;
            for(optional<T> v = next(); v; v = next())
                res = fn(res, *v);
            return optional<R>(res);
        });
    }

    template<typename T, typename Fn>
    constexpr auto stream<T, Fn>::take(size_t limit) const
    {
        return make_stream<T>([this, limit] () -> optional<T>
        {
            if (_count >= limit)
                return nullopt;
            return next();
        });
    }

    template<typename T, typename Fn>
    template<class OutputIt>
    constexpr void stream<T, Fn>::collect(OutputIt it) const
    {
        for(optional<T> v = next(); v; v = next())
            ++it = *v;
    }

    template<typename T, typename Fn>
    constexpr T stream<T, Fn>::collect() const
    {
        return *next();
    }

    template<typename T, typename Fn>
    constexpr stream<T, Fn>::optional<T> stream<T, Fn>::next() const
    {
        optional<T> v = _fn();
        if (v) ++_count;
        return v;
    }
}

#endif
