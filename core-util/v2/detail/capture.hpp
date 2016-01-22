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
#ifndef FUNCTIONAL_DETAIL_CAPTURE_HPP
#define FUNCTIONAL_DETAIL_CAPTURE_HPP

#include <tuple>

#include "interface.hpp"
#include "allocators.hpp"

namespace functional {
namespace detail {


namespace index {
template<std::size_t ...>
struct sequence { };

template<std::size_t N, std::size_t ...S>
struct generator : generator<N-1, N-1, S...> { };

template<std::size_t ...S>
struct generator<0, S...> {
    typedef sequence<S...> type;
};
} // namespace index

template <typename FunctionType, ContainerAllocator & Allocator, typename... CapturedTypes>
class CaptureFirst;

template <typename ReturnType, typename... ArgTypes, ContainerAllocator & Allocator, typename... CapturedTypes>
class CaptureFirst <ReturnType(ArgTypes...), Allocator, CapturedTypes...>
        : public FunctionInterface <ReturnType(ArgTypes...)> {
public:
    CaptureFirst(const std::tuple<CapturedTypes...>& t, Function<ReturnType(CapturedTypes..., ArgTypes...)>& f):
        f(f), storage(t)
    {}

    ReturnType operator () (ArgTypes&&... Args) {
        return idxcall(typename index::generator<sizeof...(CapturedTypes)>::type(), forward<ArgTypes>(Args)...);
    }
    template <size_t... S>
    inline ReturnType idxcall(index::sequence<S...>,  ArgTypes&&... Args) {
        return f(forward<CapturedTypes>(std::get<S>(storage))..., forward<ArgTypes>(Args)...);
    }

    ContainerAllocator * get_allocator() {
        return & Allocator;
    }
protected:
    /*
     * Future Optimization Note: It is possible to reduce memory consumption and call
     * overhead by making CaptureFirst inherit from each of the FunctionInterface
     * types, rather than just from FunctionInterface.
     *
     * In this case, however, a smarter allocator would help
     */
    Function<ReturnType(CapturedTypes..., ArgTypes...)> & f;
    std::tuple<CapturedTypes...> storage;
};

template <typename FunctionType, ContainerAllocator & Allocator, typename... CapturedTypes>
class CaptureLast;

template <typename ReturnType, typename... ArgTypes, ContainerAllocator & Allocator, typename... CapturedTypes>
class CaptureLast <ReturnType(ArgTypes...), Allocator, CapturedTypes...>
        : public FunctionInterface <ReturnType(ArgTypes...)> {
public:
    CaptureLast(Function<ReturnType(ArgTypes..., CapturedTypes...)> & f, CapturedTypes&&... CapturedArgs) :
        f(f), storage(CapturedArgs...)
    {}

    ReturnType operator () (ArgTypes&&... Args) {
        return idxcall(typename index::generator<sizeof...(CapturedTypes)>::type(), forward<ArgTypes>(Args)...);
    }
    template <size_t... S>
    inline ReturnType idxcall(index::sequence<S...>,  ArgTypes&&... Args) {
        return f(forward<ArgTypes>(Args)..., forward<CapturedTypes>(std::get<S>(storage))...);
    }

    ContainerAllocator * get_allocator() {
        return & Allocator;
    }
protected:

    Function<ReturnType(ArgTypes..., CapturedTypes...)> & f;
    std::tuple<CapturedTypes...> storage;
};

template <typename FunctionType, typename... ToRemove> struct RemoveFirstArgs {};

template <typename ReturnType, typename... ArgTypes>
struct RemoveFirstArgs <ReturnType(ArgTypes...)> {
    typedef Function<ReturnType(ArgTypes...)> type;
};

template <typename ReturnType, typename RemovedArg, typename... ArgTypes, typename ToRemove0, typename... ToRemove>
struct RemoveFirstArgs <ReturnType(RemovedArg, ArgTypes...), ToRemove0, ToRemove...> {
    static_assert(std::is_same<RemovedArg, ToRemove0>::value, "Type mismatch in argument removal");
    typedef typename RemoveFirstArgs<ReturnType(ArgTypes...), ToRemove...>::type type;
};

template <typename FunctionType0, typename FunctionType1, typename... RemoveTypes> struct RemoveLastArgs;

template <typename ReturnType, typename... Types, typename... RemoveTypes>
struct RemoveLastArgs <ReturnType(Types...), ReturnType(), RemoveTypes...> {
    using type = void;
};

template <typename ReturnType, typename... Types0, typename Transfer1, typename... Types1, typename... RemoveTypes>
struct RemoveLastArgs <ReturnType(Types0...), ReturnType(Transfer1, Types1...), RemoveTypes...> {
    using type = typename std::conditional<
        std::is_same<std::tuple<Types1...>,std::tuple<RemoveTypes...> >::value,
        Function<ReturnType(Types0..., Transfer1)>,
        typename RemoveLastArgs<ReturnType(Types0..., Transfer1), ReturnType(Types1...), RemoveTypes...>::type
    >::type;
};

template <typename ReturnType, typename... ArgTypes, typename... ParentTypes, typename... CapturedTypes>
Function<ReturnType(ArgTypes...)> bind_last(Function<ReturnType(ArgTypes...)> &&, Function<ReturnType(ParentTypes...)>& f, CapturedTypes... CapturedArgs) {
    using CaptureFP = CaptureLast<ReturnType(ArgTypes...), FunctorFPAllocator, CapturedTypes...>;
    static_assert(sizeof(CaptureFP) <= FUNCTOR_SIZE, "Size of bound arguments is too large" );
    CaptureFP * newf = reinterpret_cast<CaptureFP *>(detail::FunctorFPAllocator.alloc());
    CORE_UTIL_ASSERT_MSG(newf, "Function container memory allocation failed");
    new(newf) CaptureFP(f,forward<CapturedTypes>(CapturedArgs)...);
    return Function<ReturnType(ArgTypes...)>(static_cast<FunctionInterface<ReturnType(ArgTypes...)>*>(newf));
}


} // namespace detail
} // namespace functional
#endif // FUNCTIONAL_DETAIL_CAPTURE_HPP
