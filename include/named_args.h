#ifndef NAMED_ARGS_H
#define NAMED_ARGS_H

#include <optional>
#include <type_traits>

#define NAMED_ARG_CAT_IMPL(A, B) A ## B
#define NAMED_ARG_CAT(A, B) NAMED_ARG_CAT_IMPL(A, B)

#define NAMED_ARG_CHOOSEN32(_32,_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,_0, N, ...) N
#define NAMED_ARG_COUNT32(...) NAMED_ARG_CHOOSEN32(0, ##__VA_ARGS__, 32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)

#define NAMED_ARG_MAP3_IMPL_0(FUNC, ...)
#define NAMED_ARG_MAP3_IMPL_3( FUNC, A, B, C, ...) FUNC(A, B, C)
#define NAMED_ARG_MAP3_IMPL_6( FUNC, A, B, C, ...) FUNC(A, B, C) NAMED_ARG_MAP3_IMPL_3 (FUNC, __VA_ARGS__)
#define NAMED_ARG_MAP3_IMPL_9( FUNC, A, B, C, ...) FUNC(A, B, C) NAMED_ARG_MAP3_IMPL_6 (FUNC, __VA_ARGS__)
#define NAMED_ARG_MAP3_IMPL_12(FUNC, A, B, C, ...) FUNC(A, B, C) NAMED_ARG_MAP3_IMPL_9 (FUNC, __VA_ARGS__)
#define NAMED_ARG_MAP3_IMPL_15(FUNC, A, B, C, ...) FUNC(A, B, C) NAMED_ARG_MAP3_IMPL_12(FUNC, __VA_ARGS__)
#define NAMED_ARG_MAP3_IMPL_18(FUNC, A, B, C, ...) FUNC(A, B, C) NAMED_ARG_MAP3_IMPL_15(FUNC, __VA_ARGS__)
#define NAMED_ARG_MAP3_IMPL_21(FUNC, A, B, C, ...) FUNC(A, B, C) NAMED_ARG_MAP3_IMPL_18(FUNC, __VA_ARGS__)
#define NAMED_ARG_MAP3_IMPL_24(FUNC, A, B, C, ...) FUNC(A, B, C) NAMED_ARG_MAP3_IMPL_21(FUNC, __VA_ARGS__)
#define NAMED_ARG_MAP3_IMPL_27(FUNC, A, B, C, ...) FUNC(A, B, C) NAMED_ARG_MAP3_IMPL_24(FUNC, __VA_ARGS__)
#define NAMED_ARG_MAP3_IMPL_30(FUNC, A, B, C, ...) FUNC(A, B, C) NAMED_ARG_MAP3_IMPL_27(FUNC, __VA_ARGS__)

#define NAMED_ARG_MAP3(FUNC, ...) NAMED_ARG_CAT(NAMED_ARG_MAP3_IMPL_, NAMED_ARG_COUNT32(__VA_ARGS__))(FUNC, __VA_ARGS__)

namespace named_args {

namespace impl {

struct required_tag {};
struct optional_tag
{
    template<typename T>
    constexpr operator std::optional<T>() const
    { return std::nullopt; }
};

template<typename Tag, typename T>
struct arg
{
    using tag = Tag;
    T v;
};

template<typename Tag>
struct named_arg
{
    template<typename T>
    constexpr auto operator=(T&& v) const
    {
        return arg<Tag,
                   std::conditional_t<std::is_lvalue_reference_v<T>,
                                      std::add_lvalue_reference_t<T>,
                                      std::decay_t<T>>>
        {v};
    }
};

template<typename Tag, typename T> constexpr auto get_arg_impl(const T& default_) { return default_; }

template<typename Tag, typename T, typename Tag0, typename T0, typename ...TagN, typename ...TN>
constexpr auto get_arg_impl(const T& default_, const arg<Tag0, T0>& arg0, const arg<TagN, TN>& ... argn)
    -> std::conditional_t<std::is_same_v<Tag, Tag0>, decltype(arg0.v), decltype(get_arg_impl<Tag>(default_, argn...))>
{
    if constexpr (std::is_same_v<Tag, Tag0>)
        return arg0.v;
    else
        return get_arg_impl<Tag>(default_, argn...);
}

template<typename Tag, typename T, typename ...Tags, typename ...TN>
constexpr auto get_arg(const T& default_, const arg<Tags, TN>& ... args)
    -> decltype(get_arg_impl<Tag>(default_, args...))
{
    return get_arg_impl<Tag>(default_, args...);
}

} // namespace named_args::impl

constexpr impl::required_tag required = {};
constexpr impl::optional_tag optional = {};

} // namespace named_args

#define named_arg(NAME)                                     \
    namespace named_args::impl::tags { struct NAME {}; }    \
    namespace named_args::names { constexpr impl::named_arg<impl::tags::NAME> NAME = {}; }

#define named_arg_list_IMPL1(_0, NAME, _1) || std::is_same_v<typename std::decay_t<decltype(args)>::tag, named_args::impl::tags::NAME>

#define named_arg_list_IMPL2(TYPE, NAME, DEFAULT)                                                                   \
    using NAME##_t = std::conditional_t<                                                                            \
            std::is_same_v<std::decay_t<decltype(DEFAULT)>, ::named_args::impl::optional_tag>,                      \
            std::optional<TYPE>,                                                                                    \
            TYPE>;                                                                                                  \
    {                                                                                                               \
        using namespace named_args;                                                                                 \
        using namespace named_args::impl;                                                                           \
                                                                                                                    \
        using NAME##_arg_t = decltype(get_arg<tags::NAME>(DEFAULT, args...));                                       \
                                                                                                                    \
        constexpr bool is_optional = std::is_same_v<std::decay_t<decltype(DEFAULT)>, optional_tag>;                 \
        constexpr bool is_required = std::is_same_v<std::decay_t<decltype(DEFAULT)>, required_tag>;                 \
                                                                                                                    \
        constexpr bool has_arg = (std::is_same_v<typename std::decay_t<decltype(args)>::tag, tags::NAME> || ...);   \
        constexpr bool can_impl_conv = std::is_convertible_v<NAME##_arg_t, NAME##_t>;                               \
                                                                                                                    \
        if constexpr (is_required && !has_arg)                                                                      \
            static_assert(false, "missing required arg: '" #NAME "'");                                              \
        else if constexpr (!can_impl_conv)                                                                          \
            static_assert(false, "cannot convert the type of arg '" #NAME "' to '" #TYPE "'");                      \
    }                                                                                                               \
    NAME##_t NAME = ::named_args::impl::get_arg<::named_args::impl::tags::NAME>(DEFAULT, args...);

#define named_arg_list(...)                                                                                         \
    {                                                                                                               \
        constexpr bool no_extra_arg = (true && ... && (false NAMED_ARG_MAP3(named_arg_list_IMPL1, __VA_ARGS__)));   \
        static_assert(no_extra_arg, "additional arg in the input.");                                                \
    }                                                                                                               \
    NAMED_ARG_MAP3(named_arg_list_IMPL2, __VA_ARGS__)

#endif // !NAMED_ARGS_H
