#ifndef _optional_hpp_54456_
#define _optional_hpp_54456_
#include <stdexcept>
#include <utility>
#include "type_traits.hpp"
#include "memory.hpp"

namespace plusar
{
    struct nullopt_t {};
    static const constexpr nullopt_t nullopt;

    class bad_optional_access : public std::logic_error
    {
    public:
        bad_optional_access() : logic_error("bad optional access") { }

        explicit bad_optional_access(const char* what) : logic_error(what) { }

        virtual ~bad_optional_access() noexcept = default;
    };

    inline void throw_bad_optional_access(const char* what)
    {
        throw bad_optional_access(what);
    }

    template<typename T>
    class optional;

    template<typename T, typename U>
    using is_convertible_from =
        disjunction<
            std::is_constructible<T, const optional<U>&>,
            std::is_constructible<T, optional<U>&>,
            std::is_constructible<T, const optional<U>&&>,
            std::is_constructible<T, optional<U>&&>,
            std::is_convertible<const optional<U>&, T>,
            std::is_convertible<optional<U>&, T>,
            std::is_convertible<const optional<U>&&, T>,
            std::is_convertible<optional<U>&&, T>
        >;

    template<typename T, typename U>
        using is_assignable_from =
        disjunction<
            std::is_assignable<T&, const optional<U>&>,
            std::is_assignable<T&, optional<U>&>,
            std::is_assignable<T&, const optional<U>&&>,
            std::is_assignable<T&, optional<U>&&>
        >;

    template<typename T>
    class optional_payload
    {
        using type = std::remove_const_t<T>;
        struct empty_byte {};

        union {
            empty_byte _empty;
            type       _payload;
        };
        bool _is_empty = true;

    public:
        constexpr optional_payload() noexcept :
            _empty{}
        {}

        constexpr optional_payload(nullopt_t) noexcept :
            _empty{}
        {}

        // in-place constructor
        template<typename... Args>
        constexpr explicit optional_payload(in_place_t, Args&&... args):
            _payload(std::forward<Args>(args)...),
            _is_empty(false)
        {}

        // in-place constructor
        template<
            typename U,
            typename... Args,
            std::enable_if_t<std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value, int>...
        >
        constexpr explicit optional_payload(in_place_t, std::initializer_list<U> il, Args&&... args):
            _payload(il, std::forward<Args>(args)...),
            _is_empty(false)
        {}

        // copy constructor
        optional_payload(optional_payload const & other)
        {
            if (!other._is_empty)
                construct(other.get());
        }

        // move constructor
        optional_payload(optional_payload && other) noexcept(std::is_nothrow_move_constructible<T>())
        {
            if (!other._is_empty)
                construct(std::move(other.get()));
        }

        // copy assignment
        optional_payload & operator = (optional_payload const & other)
        {
            if (!_is_empty && !other._is_empty)
                get() = other.get();
            else
            {
                if (!other._is_empty)
                    construct(other.get());
                else
                    reset();
            }

            return *this;
        }

        // move assignment
        optional_payload & operator = (optional_payload && other) noexcept(conjunction<
                                                                            std::is_nothrow_move_constructible<T>,
                                                                            std::is_nothrow_move_assignable<T>
                                                                           >())
        {
            if (!_is_empty && !other._is_empty)
                get() = std::move(other.get());
            else
            {
                if (!other._is_empty)
                    construct(std::move(other.get()));
                else
                    reset();
            }
            return *this;
        }

        ~optional_payload()
        {}

        constexpr T & get() noexcept                { return _payload; }
        constexpr T const & get() const noexcept    { return _payload; }
        constexpr bool is_empty() const noexcept    { return _is_empty; }

        template<typename... Args>
        void construct(Args&&... args) noexcept(std::is_nothrow_constructible<type, Args...>())
        {
            ::new (plusar::addressof(_payload)) type(std::forward<Args>(args)...);
            _is_empty = false;
        }

        void destruct()
        {
            _is_empty = true;
            _payload.~type();
        }

        void reset()
        {
            if (!_is_empty)
                destruct();
        }
    };

    // Base class for trivially destructible objects
    template<typename T, bool = std::is_trivially_destructible<T>::value>
    class optional_base : public optional_payload<T>
    {
        using base = optional_payload<T>;

    public:
        using base::base;

        ~optional_base()
        {
            this->reset();
        }
    };

    // Base class for not trivially destructible objects
    template<typename T>
    class optional_base<T, false> : public optional_payload<T>
    {
        using base = optional_payload<T>;

    public:
        using base::base;
    };

    template<typename T>
    class optional: private optional_base<T>
    {
        static_assert(conjunction<
                        negation<std::is_same<std::remove_cv_t<T>, nullopt_t>>,
                        negation<std::is_same<std::remove_cv_t<T>, in_place_t>>,
                        negation<std::is_reference<T>>
                      >(), "Invalid instantiation of optional<T>");

        using base = optional_base<T>;

    public:
        using base::base;
        using value_type = T;

        constexpr optional() = default;

        // converting constructors
        template <
            typename U = T,
            std::enable_if_t<
                conjunction<
                    negation<std::is_same<optional<T>, std::decay_t<U>>>,
                    std::is_constructible<T, U&&>
                >::value,
                bool
            > = true
        >
        constexpr optional(U && val):
            base(in_place, std::forward<U>(val))
        {}

        template <
            typename U,
            std::enable_if_t<
                conjunction<
                    negation<std::is_same<T, U>>,
                    std::is_constructible<T, const U&>,
                    negation<is_convertible_from<T, U>>
                >::value,
                bool
            > = true
        >
        constexpr optional(const optional<U>& val)
        {
            if (val) emplace(*val);
        }

        template <
            typename U,
            std::enable_if_t<
                conjunction<
                    negation<std::is_same<T, U>>,
                    std::is_constructible<T, U&&>,
                    negation<is_convertible_from<T, U>>
                >::value,
                bool
            > = true
        >
        constexpr optional(optional<U>&& val)
        {
            if (val) emplace(std::move(*val));
        }

        // nullopt assignment
        optional & operator = (nullopt_t) noexcept
        {
            base::reset();
            return *this;
        }

        template<typename U = T>
        std::enable_if_t<
            conjunction<
                negation<std::is_same<optional<T>, std::decay_t<U>>>,
                std::is_constructible<T, U>,
                negation<conjunction<
                            std::is_scalar<T>,
                            std::is_same<T, std::decay_t<U>>
                        >
                >,
                std::is_assignable<T&, U>
            >::value,
            optional &
        >
        operator = (U && rhs)
        {
            if (!base::is_empty())
                base::get() = std::forward<U>(rhs);
            else
                base::construct(std::forward<U>(rhs));
            return *this;
        }

        template<typename U>
        std::enable_if_t<
            conjunction<
                negation<std::is_same<T, U>>,
                std::is_constructible<T, const U&>,
                std::is_assignable<T&, U>,
                negation<is_convertible_from<T, U>>,
                negation<is_assignable_from<T, U>>
            >::value,
            optional &
        >
        operator = (const optional<U>& rhs)
        {
            if (rhs)
            {
                if (!base::is_empty())
                    base::get() = *rhs;
                else
                    base::construct(*rhs);
            }
            else
                base::reset();
            return *this;
        }

        template<typename U>
            std::enable_if_t<
                conjunction<
                    negation<std::is_same<T, U>>,
                    std::is_constructible<T, U>,
                    std::is_assignable<T&, U>,
                    negation<is_convertible_from<T, U>>,
                    negation<is_assignable_from<T, U>>
                >::value,
                optional &
        >
        operator = (optional<U> && rhs)
        {
            if (rhs)
            {
                if (!base::is_empty())
                    base::get() = std::move(*rhs);
                else
                    base::construct(std::move(*rhs));
            }
            else
                base::reset();

          return *this;
        }

        template<typename... Args>
        std::enable_if_t<
            std::is_constructible<T, Args&&...>::value
        >
        emplace(Args&&... args)
        {
            base::reset();
            base::construct(std::forward<Args>(args)...);
        }

        template<typename U, typename... Args>
        std::enable_if_t<
            std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value
        >
        emplace(std::initializer_list<U> il, Args&&... args)
        {
            base::reset();
            base::construct(il, std::forward<Args>(args)...);
        }

        void swap(optional& val) noexcept(std::is_nothrow_move_constructible<T>()
                                          && noexcept(std::swap(std::declval<T&>(), std::declval<T&>())))
        {
            if (!base::is_empty() && !val.is_empty())
                std::swap(base::get(), val.get());
            else if (!base::is_empty())
            {
                val.construct(std::move(base::get()));
                base::destruct();
            }
            else if (!val.is_empty())
            {
                base::construct(std::move(val.get()));
                val.destruct();
            }
        }

        constexpr const T* operator->() const               { return addressof(base::get()); }
        T * operator->()                                    { return addressof(base::get()); }
        constexpr T const& operator*() const&               { return value(); }
        constexpr T& operator*() &                          { return value(); }
        constexpr T&& operator * () &&                      { return std::move(value()); }
        constexpr T const && operator * () const&&          { return std::move(value()); }
        constexpr explicit operator bool() const noexcept   { return !base::is_empty(); }

        constexpr T const & value() const&
        {
            return !base::is_empty()
                    ? base::get()
                    : (throw_bad_optional_access("Attempt to access value of a disengaged optional object"), base::get());
        }

        constexpr T & value() &
        {
            return !base::is_empty()
                    ? base::get()
                    : (throw_bad_optional_access("Attempt to access value of a disengaged optional object"), base::get());
        }

        constexpr T && value()&&
        {
            return !base::is_empty()
                    ?  std::move(base::get())
                    : (throw_bad_optional_access("Attempt to access value of a disengaged optional object"), std::move(base::get()));
        }

        constexpr T const && value() const&&
        {
            return !base::is_empty()
                        ?  std::move(base::get())
                        : (throw_bad_optional_access("Attempt to access value of a disengaged optional object"), std::move(base::get()));
        }

        template<typename U>
        constexpr T value_or(U && or_val) const&
        {
            static_assert(conjunction<
                            std::is_copy_constructible<T>,
                            std::is_convertible<U&&, T>>(),
                          "Cannot return value");

            return !base::is_empty()
                    ? base::get()
                    : static_cast<T>(std::forward<U>(or_val));
        }

        template<typename U>
        T value_or(U && or_val) &&
        {
            static_assert(conjunction<
                            std::is_move_constructible<T>,
                            std::is_convertible<U&&, T>>(),
                          "Cannot return value");

            return !base::is_empty()
                    ? std::move(base::get())
                    : static_cast<T>(std::forward<U>(or_val));
        }
    };

    template<typename T>
    constexpr bool operator == (const optional<T>& lhs, const optional<T>& rhs)
    {
        return static_cast<bool>(lhs) == static_cast<bool>(rhs)
                && (!lhs || *lhs == *rhs);
    }

    template<typename T>
    constexpr bool operator != (const optional<T>& lhs, const optional<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template<typename T>
    constexpr bool operator == (const optional<T>& lhs, nullopt_t) noexcept
    {
        return !lhs;
    }

    template<typename T>
    constexpr bool operator == (nullopt_t, const optional<T>& rhs) noexcept
    {
        return !rhs;
    }

    template<typename T>
    constexpr bool operator != (const optional<T>& lhs, nullopt_t) noexcept
    {
        return static_cast<bool>(lhs);
    }

    template<typename T>
    constexpr bool operator != (nullopt_t, const optional<T>& rhs) noexcept
    {
        return static_cast<bool>(rhs);
    }

    template<typename T>
    constexpr bool operator == (const optional<T>& lhs, const T& rhs)
    {
        return lhs && *lhs == rhs;
    }

    template<typename T>
    constexpr bool operator == (const T& lhs, const optional<T>& rhs)
    {
        return rhs && lhs == *rhs;
    }

    template<typename T>
    constexpr bool operator != (const optional<T>& lhs, T const& rhs)
    {
        return !lhs || !(*lhs == rhs);
    }

    template<typename T>
    constexpr bool operator != (const T& lhs, const optional<T>& rhs)
    {
        return !rhs || !(lhs == *rhs);
    }

    template<typename T>
    inline void swap(optional<T>& lhs, optional<T>& rhs) noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }

    template<typename T>
    constexpr optional<std::decay_t<T>> make_optional( T&& val)
    {
        return optional<std::decay_t<T>> { std::forward<T>(val) };
    }
}

#endif
