/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef CORE_UTIL_V2_DETAIL_POLYFILL_HPP
#define CORE_UTIL_V2_DETAIL_POLYFILL_HPP

namespace polyfill {
// From ARM Compiler armcc User Guide {
// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0472k/chr1407404265784.html
template< class T > struct remove_reference      {typedef T type;};
template< class T > struct remove_reference<T&>  {typedef T type;};
template< class T > struct remove_reference<T&&> {typedef T type;};

template<class T>
typename remove_reference<T>::type&&
move(T&& a) noexcept
{
	typedef typename remove_reference<T>::type&& RvalRef;
	return static_cast<RvalRef>(a);
}
template<class T>
T&&
forward(typename remove_reference<T>::type& a) noexcept
{
	return static_cast<T&&>(a);
}
/// } From ARM Compiler armcc User Guide


template<class T>
T&&
forward(typename remove_reference<T>::type&& a) noexcept
{
    return static_cast<T&&>(a);
}

template <typename T0, typename T1>
struct is_same {
    static constexpr bool value = false;
};

template <typename T>
struct is_same<T,T> {
    static constexpr bool value = true;
};

template <bool B, typename T0, typename T1>
struct conditional;

template <typename T0, typename T1>
struct conditional<true, T0, T1> {
    using type = T0;
};

template <typename T0, typename T1>
struct conditional<false, T0, T1> {
    using type = T1;
};

template <bool, typename T = void>
struct enable_if_impl {};

template <typename T>
struct enable_if_impl<true, T> {
    typedef T type;
};

template <bool B, typename T = void>
struct enable_if : public enable_if_impl<B, T> {};

template <typename T0, typename T1>
struct align_gt {
    static constexpr bool value = __alignof__(T0) > __alignof__(T1);
};
template <typename T0, typename T1>
struct align_le {
    static constexpr bool value = __alignof__(T0) <= __alignof__(T1);
};

template <typename T0, typename T1, typename T = void>
struct enable_if_align_gt : enable_if<align_gt<T0,T1>::value, T> {};
template <typename T0, typename T1, typename T = void>
struct enable_if_align_le : enable_if<align_le<T0,T1>::value, T> {};



template <unsigned I, typename T0, typename... T>
struct tuple_element {
    using type = typename tuple_element<I-1,T...>::type;
};

template <typename T0, typename... T>
struct tuple_element<0,T0,T...> {
    using type = T0;
};

template <typename... T>
struct tuple_impl;

template <typename T0>
struct tuple_impl<T0> {
    T0 t0;
    tuple_impl(const T0 &t0) : t0(forward<const T0 &>(t0)) {}
    template <unsigned I>
    typename tuple_element<I,T0>::type & get() {
        static_assert(I == 0, "tuple range exceeded");
        return t0;
    }
};

template <typename T0, typename... T>
struct tuple_impl<T0, T...> {
    T0 t0;
    tuple_impl<T...> t;

    tuple_impl(const T0 &t0, const T&... t) :
        t0(forward<const T0 &>(t0)), t(forward<const T &>(t)...) {}
    template <unsigned I>
    typename enable_if<I != 0, typename tuple_element<I, T0, T...>::type>::type & get() {
        return t.get<I-1>();
    }
    template <unsigned I>
    typename enable_if<I == 0, typename tuple_element<I, T0, T...>::type>::type & get() {
        return t0;
    }
};



template <typename... T>
struct tuple : public tuple_impl<T...> {
    tuple(const T&... t) : tuple_impl<T...>(forward<const T &>(t)...) {}
    tuple(){}
};

template <unsigned I, typename... T>
constexpr typename tuple_element<I, T...>::type & get(tuple<T...> & t) {
    return t.get<I>();
}

} // namespace polyfill

#endif // CORE_UTIL_V2_DETAIL_POLYFILL_HPP
