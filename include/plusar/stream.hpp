#pragma once
#include <type_traits>
#include <optional>
#include <initializer_list>
#include <cstddef>

namespace plusar
{
    template<typename Fn>
    class stream
    {
        Fn _fn;

        stream() = delete;
        stream & operator = (stream const &) = delete;
        stream & operator = (stream &&) = delete;

    public:
        using type = typename std::result_of_t<Fn()>::value_type;

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
        constexpr explicit stream(Args&&... args) noexcept(std::is_nothrow_constructible<Fn, Args...>());

        template<typename FnPredicate>
        constexpr auto filter(FnPredicate && pred) const;

        template<typename FnR>
        constexpr auto map(FnR && fn) const;

        template<typename FnR, typename R = typename std::result_of_t<FnR()>>
        constexpr auto reduce(R && v, FnR && fn) const;

        constexpr auto take(size_t limit) const;

        constexpr auto skip(size_t limit) const;

        template<class OutputIt>
        constexpr void collect(OutputIt it) const;
        template<class OutputIt>
        constexpr void collect(OutputIt it);

        constexpr type collect() const;
        constexpr type collect();

        constexpr std::optional<type> next() const;
        constexpr std::optional<type> next();
    };

    template <typename Fn>
    constexpr auto make_stream(Fn && fn)
    {
        return stream<std::decay_t<Fn>>(std::forward<Fn>(fn));
    }

    template <typename T>
    constexpr auto make_stream(std::initializer_list<T> il)
    {
        auto fn = [il = std::move(il), it = il.begin()]() mutable
        {
            return it == il.end() ? std::nullopt : std::make_optional(*(it++));
        };
        return stream<decltype(fn)>(std::move(fn));
    }

    template<typename Fn>
    stream<Fn>::stream(stream const &other) noexcept(std::is_nothrow_copy_constructible<Fn>()):
        _fn(other._fn)
    {}

    template<typename Fn>
    stream<Fn>::stream(stream &&other) noexcept(std::is_nothrow_move_constructible<Fn>::value
                                                   && std::is_nothrow_copy_constructible<Fn>::value):
        _fn(std::forward<Fn>(other._fn))
    {}

    template<typename Fn>
    stream<Fn>::stream(Fn && fn) noexcept(std::is_nothrow_move_constructible<Fn>::value
                                          && std::is_nothrow_copy_constructible<Fn>::value):
        _fn(std::forward<Fn>(fn))
    {}

    template<typename Fn>
    template<
        typename... Args,
        std::enable_if_t<std::is_constructible<Fn, Args&&...>::value, int>...
    >
    constexpr stream<Fn>::stream(Args&&... args) noexcept(std::is_nothrow_constructible<Fn, Args...>()):
        _fn(std::forward<Args>(args)...)
    {}

    template<typename Fn>
    template<typename FnPredicate>
    constexpr auto stream<Fn>::filter(FnPredicate && pred) const
    {
        return make_stream([src = *this, pred = std::forward<FnPredicate>(pred)]() mutable -> std::optional<type>
        {
            for(auto sv = src.next(); sv; sv = src.next())
                if(pred(*sv))
                    return sv;
            return std::nullopt;
        });
    }

    template<typename Fn>
    template<typename FnR>
    constexpr auto stream<Fn>::map(FnR && fn) const
    {
        return make_stream([src = *this, fn = std::forward<FnR>(fn)]() mutable
        {
            auto sv = src.next();
            return sv ? std::make_optional(fn(*sv)) : std::nullopt;
        });
    }

    template<typename Fn>
    template<typename FnR, typename R>
    constexpr auto stream<Fn>::reduce(R && v, FnR && fn) const
    {
        return make_stream([src = *this, v = std::forward<R>(v), fn = std::forward<FnR>(fn)]() mutable
        {
            R res = v;
            for(auto v = src.next(); v; v = src.next())
                res = fn(res, *v);
            return std::make_optional(res);
        });
    }

    template<typename Fn>
    constexpr auto stream<Fn>::take(size_t limit) const
    {
        return make_stream([src = *this, limit] () mutable -> std::optional<type>
        {
            if (!limit)
                return std::nullopt;
            --limit;
            return src.next();
        });
    }

    template<typename Fn>
    constexpr auto stream<Fn>::skip(size_t limit) const
    {
        return make_stream([src = *this, limit] () mutable -> std::optional<type>
        {
            while(limit)
            {
                auto v = src.next();
                if (!v)
                    return v;
                --limit;
            }
            return src.next();
        });
    }

    template<typename Fn>
    template<class OutputIt>
    constexpr void stream<Fn>::collect(OutputIt it) const
    {
        for(auto v = next(); v; v = next())
            ++it = *v;
    }

    template<typename Fn>
    template<class OutputIt>
    constexpr void stream<Fn>::collect(OutputIt it)
    {
        for(auto v = next(); v; v = next())
            ++it = *v;
    }

    template<typename Fn>
    constexpr typename stream<Fn>::type stream<Fn>::collect() const
    {
        return *next();
    }

    template<typename Fn>
    constexpr typename stream<Fn>::type stream<Fn>::collect()
    {
        return *next();
    }

    template<typename Fn>
    constexpr std::optional<typename stream<Fn>::type> stream<Fn>::next() const
    {
        return _fn();
    }

    template<typename Fn>
    constexpr std::optional<typename stream<Fn>::type> stream<Fn>::next()
    {
        return _fn();
    }
}
