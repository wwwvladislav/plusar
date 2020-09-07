#pragma once
#include <type_traits>
#include <optional>
#include <cstddef>
#include <utility>

namespace plusar
{
    template<typename Fn>
    class stream
    {
        Fn _fn;

        stream() = delete;
        stream & operator = (stream const &other) = delete;
        stream & operator = (stream &&other) = delete;

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

        constexpr type collect() const
        {
            return next().value();
        }

        constexpr std::optional<type> next() const
        {
            return _fn();
        }
    };

    namespace internal
    {
        struct mutable_idx
        {
            mutable size_t value = 0;
        };

        template<typename T>
        class mutable_optional
        {
            mutable std::optional<T> _value = std::nullopt;

        public:
            void operator=(std::optional<T> const &rhs) const
            {
                if (rhs.has_value())
                    _value.emplace(rhs.value());
                else
                    _value.reset();
            }

            std::optional<T> const & operator->() const
            {
                return _value;
            }

            operator bool() const
            {
                return _value.has_value();
            }
        };
    }

    template <typename Fn>
    constexpr auto make_stream(Fn && fn)
    {
        return stream<std::decay_t<Fn>>(std::forward<Fn>(fn));
    }

    template <typename T, size_t N>
    constexpr auto make_stream(T const (&arr)[N])
    {
        using internal::mutable_idx;

        auto fn = [arr, n = mutable_idx{}]()
        {
            return n.value >= N
                        ? std::nullopt
                        : std::make_optional(arr[n.value++]);
        };
        return stream<decltype(fn)>(std::move(fn));
    }

    template<typename Fn>
    template<typename FnPredicate>
    constexpr auto stream<Fn>::filter(FnPredicate && pred) const
    {
        return make_stream([src = *this, pred = std::forward<FnPredicate>(pred)]() -> std::optional<type>
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
        return make_stream([src = *this, fn = std::forward<FnR>(fn)]()
        {
            auto sv = src.next();
            return sv ? std::make_optional(fn(*sv)) : std::nullopt;
        });
    }

    template<typename Fn>
    template<typename FnR, typename R>
    constexpr auto stream<Fn>::reduce(R && v, FnR && fn) const
    {
        return make_stream([src = *this, v = std::forward<R>(v), fn = std::forward<FnR>(fn)]()
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
        using namespace internal;

        return make_stream([src = *this, current = mutable_optional<type>{}] () -> std::optional<typename type::type>
        {
            if (!current)
                current = src.next();

            for(;current; current = src.next())
            {
                std::optional<typename type::type> const &v = current->next();
                if (v)
                    return v;
            }

            return std::nullopt;
        });
    }

    template<typename Fn>
    constexpr auto stream<Fn>::take(size_t limit) const
    {
        using internal::mutable_idx;

        return make_stream([src = *this, n = mutable_idx{}, limit] () -> std::optional<type>
        {
            if (n.value >= limit)
                return std::nullopt;
            ++n.value;
            return src.next();
        });
    }

    template<typename Fn>
    constexpr auto stream<Fn>::skip(size_t limit) const
    {
        using internal::mutable_idx;

        return make_stream([src = *this, n = mutable_idx{}, limit] () -> std::optional<type>
        {
            while(n.value < limit)
            {
                auto v = src.next();
                if (!v)
                    return v;
                n.value++;
            }
            return src.next();
        });
    }

    template<typename Fn>
    template<typename FnStream, typename FnZip>
    constexpr auto stream<Fn>::zip(stream<FnStream> && other, FnZip && fn) const
    {
        return make_stream([src = *this, other = std::forward<stream<FnStream>>(other), fn = std::forward<FnZip>(fn)] ()
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

        return make_stream([src = skip(start), step] () -> std::optional<type>
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
}
