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

#include <type_traits>
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
class CapturedArguments;

template <typename ReturnType, typename... ArgTypes, ContainerAllocator & Allocator, typename... CapturedTypes>
class CapturedArguments <ReturnType(ArgTypes...), Allocator, CapturedTypes...>
        : public FunctionInterface <ReturnType(ArgTypes...)> {
public:
    // CapturedArguments(Function<ReturnType(CapturedTypes..., ArgTypes...)> & f, CapturedTypes&&... CapturedArgs) :
    //     f(f), storage(CapturedArgs...)
    // {}
    CapturedArguments(const std::tuple<CapturedTypes...>& t, Function<ReturnType(CapturedTypes..., ArgTypes...)>& f):
        f(f), storage(t)
    {}

    virtual ReturnType operator () (ArgTypes&&... Args) {
        return idxcall(typename index::generator<sizeof...(CapturedTypes)>::type(), forward<ArgTypes>(Args)...);
    }
    template <size_t... S>
    inline ReturnType idxcall(index::sequence<S...>,  ArgTypes&&... Args) {
        return f(forward<CapturedTypes>(std::get<S>(storage))..., forward<ArgTypes>(Args)...);
    }

    virtual ContainerAllocator * getAllocator() {
        return & Allocator;
    }
protected:
    /*
     * Future Optimization Note: It is possible to reduce memory consumption and call
     * overhead by making CapturedArguments inherit from each of the FunctionInterface
     * types, rather than just from FunctionInterface.
     *
     * In this case, however, a smarter allocator would help
     */
    Function<ReturnType(CapturedTypes..., ArgTypes...)> & f;
    std::tuple<CapturedTypes...> storage;
};

template <typename FunctionType, typename... ToRemove> struct RemoveArgs {};

template <typename ReturnType, typename... ArgTypes>
struct RemoveArgs <ReturnType(ArgTypes...)> {
    typedef Function<ReturnType(ArgTypes...)> type;
};

template <typename ReturnType, typename RemovedArg, typename... ArgTypes, typename ToRemove0, typename... ToRemove>
struct RemoveArgs <ReturnType(RemovedArg, ArgTypes...), ToRemove0, ToRemove...> {
    static_assert(std::is_same<RemovedArg, ToRemove0>::value, "Type mismatch in argument removal");
    typedef typename RemoveArgs<ReturnType(ArgTypes...), ToRemove...>::type type;
};

} // namespace detail
} // namespace functional
#endif // FUNCTIONAL_DETAIL_CAPTURE_HPP
