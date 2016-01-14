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
#ifndef __FUNCTIONAL_HPP__
#define __FUNCTIONAL_HPP__

/**
 * \file functional.hpp
 * \brief A library that provides function containers, allocated by arbitrary allocators
 *
 * # Function
 * Function is the primary API for functional.hpp
 * Function derives from a smart pointer to a functor.
 * Function contains almost entirely inline APIs
 *
 * Functor is a callable class.
 *
 */

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <new>
#include <utility>
#include <cstdio>
#include <type_traits>

#include "core-util/ExtendablePoolAllocator.h"

namespace functional {
template <typename FunctionType>
class Function;

} // namespace functional

#include "detail/forward.hpp"
#include "detail/interface.hpp"
#include "detail/static.hpp"
#include "detail/member.hpp"
#include "detail/functor.hpp"
#include "detail/allocators.hpp"
#include "detail/capture.hpp"


namespace functional {
namespace detail {



} // namespace detail


template <typename ReturnType, typename... ArgTypes>
class Function <ReturnType(ArgTypes...)> {
public:
    // Function(Deleter): f(nullptr){}
    // template <typename F>
    // Function(const F & f) : FunctionPointer() {
    //     F * newf = allocator(sizeof(FunctorContainer<F>))
    //     new(newf) FunctorContainer<F>(f);
    //     *this = newf;
    // }
    // template <>
    Function() : ref(nullptr) {}
    Function(ReturnType (*f)(ArgTypes...)) {
        typedef detail::StaticContainer<ReturnType(ArgTypes...)> staticFP;
        staticFP * newf = reinterpret_cast<staticFP *>(detail::StaticFPAllocator.alloc());
        new(newf) staticFP(f);
        ref = newf;
        ref->inc();
    }
    template <class C>
    Function(C *o, ReturnType (C::*fp)(ArgTypes...)) {
        typedef detail::MemberContainer<C, ReturnType(ArgTypes...)> memberFP;
        memberFP * newf = reinterpret_cast<memberFP *>(detail::MemberFPAllocator.alloc());
        new(newf) memberFP(o, fp);
        ref = newf;
        ref->inc();
    }
    //TODO: use template metaprogramming to select one of three allocator pools
    template <typename F>
    Function(const F &f) {
        typedef detail::FunctorContainer<F, ReturnType(ArgTypes...), detail::FunctorFPAllocator> FunctorFP;
        FunctorFP * newf = reinterpret_cast<FunctorFP *>(detail::FunctorFPAllocator.alloc());
        new(newf) FunctorFP(f);
        ref = newf;
        ref->inc();
    }
    Function(const Function & f): ref(f.ref) {
        ref && ref->inc();
    }
    // template <typename... CapturedTypes>
    // Function(Function<ReturnType(CapturedTypes..., ArgTypes...)> & f, CapturedTypes&&... CapturedArgs) {
    //     typedef typename detail::CapturedArguments<ReturnType(ArgTypes...), detail::FunctorFPAllocator, CapturedTypes...> CaptureFP;
    //     std::tuple<CapturedTypes...> t(detail::forward<CapturedTypes>(CapturedArgs)...);
    //     CaptureFP * newf = reinterpret_cast<CaptureFP *>(detail::FunctorFPAllocator.alloc());
    //     new(newf) CaptureFP(t, f);
    //     ref = newf;
    //     ref->inc();
    // }
    template <typename... CapturedTypes, typename... ParentArgTypes>
    Function(std::tuple<CapturedTypes...>& t, Function<ReturnType(ParentArgTypes...)>& f) {
        typedef typename detail::CapturedArguments<ReturnType(ArgTypes...), detail::FunctorFPAllocator, CapturedTypes...> CaptureFP;
        static_assert(sizeof(CaptureFP) <= 40, "Size of bound arguments is too large" );
        CaptureFP * newf = reinterpret_cast<CaptureFP *>(detail::FunctorFPAllocator.alloc());
        new(newf) CaptureFP(t, f);
        ref = newf;
        ref->inc();
    }
    template<typename... CapturedTypes>
    typename detail::RemoveArgs<ReturnType(ArgTypes...), CapturedTypes...>::type bind(CapturedTypes... CapturedArgs) {
        std::tuple<CapturedTypes...> t(detail::forward<CapturedTypes>(CapturedArgs)...);
        typename detail::RemoveArgs<ReturnType(ArgTypes...), CapturedTypes...>::type f(t,*this);
        return f;
    }

    ~Function() {
        ref && ref->dec() && ref->getAllocator()->free(ref);
    }
    Function & operator = (const Function & rhs) {
        ref && ref->dec() && ref->getAllocator()->free(ref);
        ref = rhs.ref;
        ref && ref->inc();
    }
    //     MemberFunctionContainer<
    // }
    // template <>
    // Function(const Function<ReturnType(Args...)> & rhs) {
    //     *this = f;
    // }
    inline ReturnType operator () (ArgTypes&&... Args) {
        return (*ref)(detail::forward<ArgTypes>(Args)...);
    }
    // inline bool operator == (const Function<ReturnType(Args...)> & rhs) {
    //     return static_cast<FunctionPointer>(*this) == static_cast<FunctionPointer>(rhs);
    // }
    // inline Function& operator = (const Function<ReturnType(Args...)> & rhs) {
    //     static_cast<FunctionPointer>(*this) = static_cast<FunctionPointer>(rhs);
    //     return *this
    // }
protected:
    detail::FunctionInterface <ReturnType(ArgTypes...)> * ref;
};

} // namespace functional

#endif
