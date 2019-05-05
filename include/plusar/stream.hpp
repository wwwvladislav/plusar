#pragma once
#include "type_traits.hpp"
#include "optional.hpp"
#include <initializer_list>

namespace plusar
{
    template<typename Fn, typename T = typename std::result_of_t<Fn()>::value_type>
    class stream
    {
        Fn _fn;

        stream() = delete;
        stream & operator = (stream const &) = delete;
        stream & operator = (stream &&) = delete;

    public:
        stream(stream const &) noexcept(std::is_nothrow_copy_constructible<Fn>());
        stream(stream &&) noexcept(std::is_nothrow_move_constructible<Fn>::value
                                   &&std::is_nothrow_copy_constructible<Fn>::value);
        ~stream() = default;

        stream(Fn && fn) noexcept(std::is_nothrow_move_constructible<Fn>::value
                                  && std::is_nothrow_copy_constructible<Fn>::value);

        template<
            typename... Args,
            std::enable_if_t<std::is_constructible<Fn, Args&&...>::value, int>...
        >
        constexpr explicit stream(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible<Fn, Args...>());

        template<typename FnPredicate>
        constexpr auto filter(FnPredicate && pred) const &;
        template<typename FnPredicate>
        constexpr auto filter(FnPredicate && pred) &&;

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

        constexpr optional<T> next() &;
        constexpr optional<T> next() &&;
    };

    template <typename Fn, typename T = typename std::result_of_t<Fn()>::value_type>
    constexpr auto make_stream(Fn && fn)
    {
        return stream<std::decay_t<Fn>, T>(std::forward<Fn>(fn));
    }

    template <typename T>
    constexpr auto make_stream(std::initializer_list<T> il)
    {
        auto fn = [il = std::move(il), it = il.begin()]() mutable
        {
            return it == il.end() ? nullopt : make_optional(*(it++));
        };
        return stream<decltype(fn), T>(std::move(fn));
    }

    template<typename Fn, typename T>
    stream<Fn, T>::stream(stream const &other) noexcept(std::is_nothrow_copy_constructible<Fn>()):
        _fn(other._fn)
    {}

    template<typename Fn, typename T>
    stream<Fn, T>::stream(stream &&other) noexcept(std::is_nothrow_move_constructible<Fn>::value
                                                   && std::is_nothrow_copy_constructible<Fn>::value):
        _fn(std::forward<Fn>(other._fn))
    {}

    template<typename Fn, typename T>
    stream<Fn, T>::stream(Fn && fn) noexcept(std::is_nothrow_move_constructible<Fn>::value
                                             && std::is_nothrow_copy_constructible<Fn>::value):
        _fn(std::forward<Fn>(fn))
    {}

    template<
        typename Fn,
        typename T
    >
    template<
        typename... Args,
        std::enable_if_t<std::is_constructible<Fn, Args&&...>::value, int>...
    >
    constexpr stream<Fn, T>::stream(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible<Fn, Args...>()):
        _fn(std::forward<Args>(args)...)
    {}

    template<typename Fn, typename T>
    template<typename FnPredicate>
    constexpr auto stream<Fn, T>::filter(FnPredicate && pred) const &
    {
        stream self(*this);
        return make_stream([self = std::move(self), pred = std::forward<FnPredicate>(pred)]() mutable
        {
            for(optional<T> sv = self.next(); sv; sv = self.next())
                if(pred(*sv))
                    return sv; 
            return optional<T>(nullopt);
        });
    }

    template<typename Fn, typename T>
    template<typename FnPredicate>
    constexpr auto stream<Fn, T>::filter(FnPredicate && pred) &&
    {
        stream self(*this);
        return make_stream([self = std::move(self), pred = std::forward<FnPredicate>(pred)]() mutable
        {
            for(optional<T> sv = self.next(); sv; sv = self.next())
                if(pred(*sv))
                    return sv; 
            return optional<T>(nullopt);
        });
    }

    template<typename Fn, typename T>
    template<typename FnR>
    constexpr auto stream<Fn, T>::map(FnR && fn) const &
    {
        stream self(*this);
        return make_stream([self = std::move(self), fn = std::forward<FnR>(fn)]() mutable
        {
            optional<T> sv = self.next();
            return sv ? make_optional(fn(*sv)) : nullopt;
        });
    }

    template<typename Fn, typename T>
    template<typename FnR>
    constexpr auto stream<Fn, T>::map(FnR && fn) &&
    {
        stream self(*this);
        return make_stream([self = std::move(self), fn = std::forward<FnR>(fn)]() mutable
        {
            optional<T> sv = self.next();
            return sv ? make_optional(fn(*sv)) : nullopt;
        });
    }

    template<typename Fn, typename T>
    template<typename FnR, typename R>
    constexpr auto stream<Fn, T>::reduce(R && v, FnR && fn) const &
    {
        stream self(*this);
        return make_stream([self = std::move(self), v = std::forward<R>(v), fn = std::forward<FnR>(fn)]() mutable
        {
            R res = v;
            for(optional<T> v = self.next(); v; v = self.next())
                res = fn(res, *v);
            return make_optional(res);
        });
    }

    template<typename Fn, typename T>
    template<typename FnR, typename R>
    constexpr auto stream<Fn, T>::reduce(R && v, FnR && fn) &&
    {
        stream self(*this);
        return make_stream([self = std::move(self), v = std::forward<R>(v), fn = std::forward<FnR>(fn)]() mutable
        {
            R res = v;
            for(optional<T> v = self.next(); v; v = self.next())
                res = fn(res, *v);
            return make_optional(res);
        });
    }

    template<typename Fn, typename T>
    constexpr auto stream<Fn, T>::take(size_t limit) const &
    {
        stream self(*this);
        return make_stream([self = std::move(self), limit] () mutable -> optional<T>
        {
            if (!limit)
                return nullopt;
            --limit;
            return self.next();
        });
    }

    template<typename Fn, typename T>
    constexpr auto stream<Fn, T>::take(size_t limit) &&
    {
        stream self(*this);
        return make_stream([self = std::move(self), limit] () mutable -> optional<T>
        {
            if (!limit)
                return nullopt;
            --limit;
            return self.next();
        });
    }

    template<typename Fn, typename T>
    template<class OutputIt>
    constexpr void stream<Fn, T>::collect(OutputIt it) const &
    {
        for(optional<T> v = next(); v; v = next())
            ++it = *v;
    }

    template<typename Fn, typename T>
    template<class OutputIt>
    constexpr void stream<Fn, T>::collect(OutputIt it) &&
    {
        for(optional<T> v = next(); v; v = next())
            ++it = *v;
    }

    template<typename Fn, typename T>
    constexpr T stream<Fn, T>::collect() const &
    {
        return *next();
    }

    template<typename Fn, typename T>
    constexpr T stream<Fn, T>::collect() &&
    {
        return *next();
    }

    template<typename Fn, typename T>
    constexpr optional<T> stream<Fn, T>::next() &
    {
        return _fn();
    }

    template<typename Fn, typename T>
    constexpr optional<T> stream<Fn, T>::next() &&
    {
        return _fn();
    }
}
