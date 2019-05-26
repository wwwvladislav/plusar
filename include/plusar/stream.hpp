#pragma once
#include <type_traits>
#include <optional>
#include <cstddef>
#include <new>
#include <memory>

namespace plusar
{
    template<typename Fn>
    class stream
    {
        Fn _fn;

        stream() = delete;

    public:
        using type = typename std::result_of_t<Fn()>::value_type;

    public:
        stream(stream const &other) noexcept(std::is_nothrow_copy_constructible<Fn>()):
            _fn(other._fn)
        {}

        stream(stream &&other) noexcept(std::is_nothrow_move_constructible<Fn>::value
                                        &&std::is_nothrow_copy_constructible<Fn>::value):
            _fn(std::forward<Fn>(other._fn))
        {}

        ~stream() = default;

        stream(Fn && fn) noexcept(std::is_nothrow_move_constructible<Fn>::value
                                  && std::is_nothrow_copy_constructible<Fn>::value):
            _fn(std::forward<Fn>(fn))
        {}

        template<
            typename... Args,
            std::enable_if_t<std::is_constructible<Fn, Args&&...>::value, int>...
        >
        constexpr explicit stream(Args&&... args) noexcept(std::is_nothrow_constructible<Fn, Args...>()):
            _fn(std::forward<Args>(args)...)
        {}

        constexpr stream & operator = (stream const &other) noexcept(std::is_nothrow_copy_constructible<Fn>())
        {
            _fn.~Fn();
            ::new((void*)std::addressof(_fn))Fn(std::forward<Fn>(other._fn));
            return *this;
        }

        constexpr stream & operator = (stream &&other) noexcept(std::is_nothrow_move_constructible<Fn>::value
                                                                && std::is_nothrow_copy_constructible<Fn>::value)
        {
            _fn.~Fn();
            ::new((void*)std::addressof(_fn))Fn(std::forward<Fn>(other._fn));
            return *this;
        }

        template<typename FnPredicate>
        constexpr auto filter(FnPredicate && pred) const;

        template<typename FnR>
        constexpr auto map(FnR && fn) const;

        template<typename FnR, typename R = typename std::result_of_t<FnR()>>
        constexpr auto reduce(R && v, FnR && fn) const;

        constexpr auto flatten() const;

        constexpr auto take(size_t limit) const;

        constexpr auto skip(size_t limit) const;

        template<typename FnStream, typename FnZip>
        constexpr auto zip(stream<FnStream> && other, FnZip && fn) const;

        constexpr auto slice(size_t start, size_t end, size_t step = 1) const;

        constexpr auto slice_to_end(size_t start, size_t step = 1) const;

        template<class OutputIt>
        constexpr void collect(OutputIt it) const;
        template<class OutputIt>
        constexpr void collect(OutputIt it);

        constexpr type collect() const
        {
            return next().value();
        }

        constexpr type collect()
        {
            return next().value();
        }

        constexpr std::optional<type> next() const
        {
            return _fn();
        }

        constexpr std::optional<type> next()
        {
            return _fn();
        }
    };

    template <typename Fn>
    constexpr auto make_stream(Fn && fn)
    {
        return stream<std::decay_t<Fn>>(std::forward<Fn>(fn));
    }

    template <typename T, size_t N>
    constexpr auto make_stream(T const (&arr)[N])
    {
        auto fn = [arr, n = 0u]() mutable
        {
            return n >= N ? std::nullopt : std::make_optional(arr[n++]);
        };
        return stream<decltype(fn)>(std::move(fn));
    }

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
    constexpr auto stream<Fn>::flatten() const
    {
        return make_stream([src = *this, current = std::optional<type>()] () mutable -> std::optional<typename type::type>
        {
            if (!current)
                current = src.next();

            for(;current; current = src.next())
            {
                auto v = current->next();
                if (v)
                    return v;
            }

            return std::nullopt;
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
    template<typename FnStream, typename FnZip>
    constexpr auto stream<Fn>::zip(stream<FnStream> && other, FnZip && fn) const
    {
        return make_stream([src = *this, other = std::forward<stream<FnStream>>(other), fn = std::forward<FnZip>(fn)] () mutable
        {
            auto a = src.next();
            auto b = other.next();
            return a && b ? std::make_optional(fn(*a, *b)) : std::nullopt;
        });
    }

    template<typename Fn>
    constexpr auto stream<Fn>::slice(size_t start, size_t end, size_t step) const
    {
        if (!step)
            step = 1;
        if (end < start)
            end = start;
        return slice_to_end(start, step)
                    .take((end - start + step - 1) / step);
    }

    template<typename Fn>
    constexpr auto stream<Fn>::slice_to_end(size_t start, size_t step) const
    {
        if (!step)
            step = 1;

        return make_stream([src = skip(start), step] () mutable -> std::optional<type>
        {
            auto v = src.next();
            if (!v)
                return v;

            for(size_t i = 1; i < step; ++i)
            {
                auto t = src.next();
                if (!t)
                    break;
            }

            return v;
        });
    }

    template<typename Fn>
    template<class OutputIt>
    constexpr void stream<Fn>::collect(OutputIt it) const
    {
        for(auto v = next(); v; v = next())
            ++it = v.value();
    }

    template<typename Fn>
    template<class OutputIt>
    constexpr void stream<Fn>::collect(OutputIt it)
    {
        for(auto v = next(); v; v = next())
            ++it = v.value();
    }
}
