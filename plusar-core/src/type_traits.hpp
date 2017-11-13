#ifndef _type_traits_hpp_54456_
#define _type_traits_hpp_54456_
#include <type_traits>

namespace plusar
{
    struct in_place_t {};
    static const constexpr in_place_t in_place {};

    template<typename...>
    struct conjunction;

    template<>
    struct conjunction<>: public std::true_type
    {};

    template<typename Expr>
    struct conjunction<Expr>: public Expr
    {};

    template<typename Expr1, typename Expr2>
    struct conjunction<Expr1, Expr2>: public std::conditional_t<Expr1::value, Expr2, Expr1>
    {};

    template<typename Expr1, typename Expr2, typename... Exprn>
    struct conjunction<Expr1, Expr2, Exprn...>: public std::conditional_t<Expr1::value, conjunction<Expr2, Exprn...>, Expr1>
    {};

    template<typename...>
    struct disjunction;

    template<>
    struct disjunction<>: public std::false_type
    {};

    template<typename Expr1>
    struct disjunction<Expr1>: public Expr1
    {};

    template<typename Expr1, typename Expr2>
    struct disjunction<Expr1, Expr2>: public std::conditional_t<Expr1::value, Expr1, Expr2>
    {};

    template<typename Expr1, typename Expr2, typename... Exprn>
    struct disjunction<Expr1, Expr2, Exprn...>: public std::conditional<Expr1::value, Expr1, disjunction<Expr2, Exprn...>>
    {};

    template<typename Expr>
    struct negation: public std::integral_constant<bool, !Expr::value>
    {};
}

#endif
