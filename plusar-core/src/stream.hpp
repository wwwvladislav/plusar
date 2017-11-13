#ifndef _stream_hpp_54456_
#define _stream_hpp_54456_
#include "type_traits.hpp"
#include "optional.hpp"

namespace plusar
{
    template<typename T, typename Fn>
    class stream
    {
        Fn _fn;
        mutable size_t _count = 0;

        stream() = delete;
        stream & operator = (stream const &) = delete;
        stream & operator = (stream &&) = delete;

    public:
        stream(stream const &) noexcept(std::is_nothrow_copy_constructible<Fn>());
        stream(stream &&) noexcept(conjunction<
                                                std::is_nothrow_move_constructible<Fn>,
                                                std::is_nothrow_copy_constructible<Fn>
                                            >());
        ~stream() = default;

        stream(Fn && fn) noexcept(conjunction<
                                    std::is_nothrow_move_constructible<Fn>,
                                    std::is_nothrow_copy_constructible<Fn>
                                  >());

        template<
            typename... Args,
            std::enable_if_t<std::is_constructible<Fn, Args&&...>::value, int>...
        >
        constexpr explicit stream(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible<Fn, Args...>());

        template<typename FnR>
        constexpr auto map(FnR && fn) const &;
        template<typename FnR>
        constexpr auto map(FnR && fn) &&;

        template<typename FnR, typename R = typename std::result_of_t<FnR()>>
        constexpr auto reduce(R && v, FnR && fn) const &;
        template<typename FnR, typename R = typename std::result_of_t<FnR()>>
        constexpr auto reduce(R && v, FnR && fn) &&;

        constexpr auto take(size_t limit) const &;
        constexpr auto take(size_t limit) &&;

        template<class OutputIt>
        constexpr void collect(OutputIt it) const &;
        template<class OutputIt>
        constexpr void collect(OutputIt it) &&;

        constexpr T collect() const &;
        constexpr T collect() &&;

        constexpr optional<T> next() const &;
        constexpr optional<T> next() &&;

        constexpr size_t count() const & noexcept;
        constexpr size_t count() && noexcept;
    };

    template <typename Fn, typename T = typename std::result_of_t<Fn()>::value_type>
    constexpr auto make_stream(Fn && fn)
    {
        return stream<T, std::decay_t<Fn>>(std::forward<Fn>(fn));
    }

    template<typename T, typename Fn>
    stream<T, Fn>::stream(stream const &other) noexcept(std::is_nothrow_copy_constructible<Fn>()):
        _fn(other._fn),
        _count(other._count)
    {}

    template<typename T, typename Fn>
    stream<T, Fn>::stream(stream &&other) noexcept(conjunction<
                                                std::is_nothrow_move_constructible<Fn>,
                                                std::is_nothrow_copy_constructible<Fn>
                                            >()):
        _fn(std::forward<Fn>(other._fn)),
        _count(other._count)
    {}

    template<typename T, typename Fn>
    stream<T, Fn>::stream(Fn && fn) noexcept(conjunction<
                                                std::is_nothrow_move_constructible<Fn>,
                                                std::is_nothrow_copy_constructible<Fn>
                                            >()):
        _fn(std::forward<Fn>(fn))
    {}

    template<
        typename T,
        typename Fn
    >
    template<
        typename... Args,
        std::enable_if_t<std::is_constructible<Fn, Args&&...>::value, int>...
    >
    constexpr stream<T, Fn>::stream(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible<Fn, Args...>()):
        _fn(std::forward<Args>(args)...)
    {}

    template<typename T, typename Fn>
    template<typename FnR>
    constexpr auto stream<T, Fn>::map(FnR && fn) const &
    {
        stream self(*this);
        return make_stream([self = std::move(self), fn = std::forward<FnR>(fn)]()
        {
            optional<T> sv = self.next();
            return sv ? make_optional(fn(*sv)) : nullopt;
        });
    }

    template<typename T, typename Fn>
    template<typename FnR>
    constexpr auto stream<T, Fn>::map(FnR && fn) &&
    {
        stream self(*this);
        return make_stream([self = std::move(self), fn = std::forward<FnR>(fn)]()
        {
            optional<T> sv = self.next();
            return sv ? make_optional(fn(*sv)) : nullopt;
        });
    }

    template<typename T, typename Fn>
    template<typename FnR, typename R>
    constexpr auto stream<T, Fn>::reduce(R && v, FnR && fn) const &
    {
        stream self(*this);
        return make_stream([self = std::move(self), v = std::forward<R>(v), fn = std::forward<FnR>(fn)]()
        {
            R res = v;
            for(optional<T> v = self.next(); v; v = self.next())
                res = fn(res, *v);
            return make_optional(res);
        });
    }

    template<typename T, typename Fn>
    template<typename FnR, typename R>
    constexpr auto stream<T, Fn>::reduce(R && v, FnR && fn) &&
    {
        stream self(*this);
        return make_stream([self = std::move(self), v = std::forward<R>(v), fn = std::forward<FnR>(fn)]()
        {
            R res = v;
            for(optional<T> v = self.next(); v; v = self.next())
                res = fn(res, *v);
            return make_optional(res);
        });
    }

    template<typename T, typename Fn>
    constexpr auto stream<T, Fn>::take(size_t limit) const &
    {
        stream self(*this);
        return make_stream([self = std::move(self), limit] () -> optional<T>
        {
            if (self.count() >= limit)
                return nullopt;
            return self.next();
        });
    }

    template<typename T, typename Fn>
    constexpr auto stream<T, Fn>::take(size_t limit) &&
    {
        stream self(*this);
        return make_stream([self = std::move(self), limit] () -> optional<T>
        {
            if (self.count() >= limit)
                return nullopt;
            return self.next();
        });
    }

    template<typename T, typename Fn>
    template<class OutputIt>
    constexpr void stream<T, Fn>::collect(OutputIt it) const &
    {
        for(optional<T> v = next(); v; v = next())
            ++it = *v;
    }

    template<typename T, typename Fn>
    template<class OutputIt>
    constexpr void stream<T, Fn>::collect(OutputIt it) &&
    {
        for(optional<T> v = next(); v; v = next())
            ++it = *v;
    }

    template<typename T, typename Fn>
    constexpr T stream<T, Fn>::collect() const &
    {
        return *next();
    }

    template<typename T, typename Fn>
    constexpr T stream<T, Fn>::collect() &&
    {
        return *next();
    }

    template<typename T, typename Fn>
    constexpr optional<T> stream<T, Fn>::next() const &
    {
        optional<T> v = _fn();
        if (v) ++_count;
        return v;
    }

    template<typename T, typename Fn>
    constexpr optional<T> stream<T, Fn>::next() &&
    {
        optional<T> v = _fn();
        if (v) ++_count;
        return v;
    }

    template<typename T, typename Fn>
    constexpr size_t stream<T, Fn>::count() const & noexcept    { return _count; }
    template<typename T, typename Fn>
    constexpr size_t stream<T, Fn>::count() && noexcept         { return _count; }
}

#endif
