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
#ifndef FUNCTIONAL_DETAIL_FUNCTOR_HPP
#define FUNCTIONAL_DETAIL_FUNCTOR_HPP

#include "interface.hpp"
#include "allocators.hpp"

namespace functional {
namespace detail {


template <typename F, typename FunctionType, ContainerAllocator & Allocator>
class FunctorContainer;

template <typename F, typename ReturnType, typename... ArgTypes, ContainerAllocator & Allocator>
class FunctorContainer <F, ReturnType(ArgTypes...), Allocator> : public FunctionInterface  <ReturnType(ArgTypes...)> {
public:
    FunctorContainer(const F & f) : f(f) {}
    FunctorContainer(const FunctorContainer & f) : f(f.f) {}

    virtual ReturnType operator () (ArgTypes&&... Args) {
        return f(detail::forward<ArgTypes>(Args)...);
    }
    virtual ContainerAllocator * get_allocator() {
        return &Allocator;
    }

    // virtual void deallocate(FunctionInterface<ReturnType(ArgTypes...)> *ptr){
    //     (void)ptr;
    //     this->~FunctionInterface();
    //     Allocator.free(this);
    // }
protected:
    F f;
};


} // namespace detail
} // namespace functional
#endif // FUNCTIONAL_DETAIL_FUNCTOR_HPP
