/* mbed Microcontroller Library
 * Copyright (c) 2006-2016 ARM Limited
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
 * \brief A library that provides a ```std::function```-like interface for containing reference to callables.
 *
 * The primary API for containing and referring to callables is ```Function```. APIs in the ```detail``` namespace
 * should not be invoked directly.
 *
 * In order to maintain compatibility with existing code, ```Function``` is intended to be duck-type compatible with
 * the ```FunctionPointer``` family of classes, but it is intentionally not named the same way. A family of
 * compatibility classes is provided to support conversion from ```FunctionPointer``` code to ```Function```.
 * These compatibility classes will be deprecated in an upcoming release.
 */

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <new>
#include <utility>
#include "core-util/atomic_ops.h"
#include "core-util/assert.h"

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
/**
 * Superficially, ```Function``` is intented to appear similar to ```std::function```. However, ```Function``` has a
 * number of key differences. ```Function``` uses reference counting to limit the copying of large callable objects,
 * such as lambdas with large capture lists or ```Function```s with large or many arguments. ```Function``` also uses
 * pool allocation so that objects can be created in interrupt context without using ```malloc()```. These choices are
 * to overcome two specific limitations of ```std::function```
 *
 * 1. Copy-constructing a functor requires copy-constructing its member objects. In the case of lambdas, this means
 * copy-constructing the lambda captures. lambda captures are not guaranteed to have reentrant copy constructors, so
 * lambdas cannot be copy-constructed in interrupt context. Therefore, functors must be created in dynamic memory and
 * only pointers to functors can be used.
 * 2. Due to 1. creating a functor requires dynamic memory allocation, however malloc is not permitted in interrupt
 * context. As a result functors must be pool allocated.
 * 3. In order to simplify memory management of functors and due to 1. and 2., functors are reference-counted.
 */
template <typename ReturnType, typename... ArgTypes>
class Function <ReturnType(ArgTypes...)> {
public:
    /**
     * The empty constructor.
     * Since ```Function``` only contains a single pointer, only a null assignment is necessary.
     */
    Function() : ref(nullptr) {}
    /**
     * Construct a Function from a FunctionInterface.
     *
     * This is an API that should only be used by Function and its helpers. It was specifically added to enable
     * bind_first and bind_last.
     *
     * @param[in] f
     */
    Function(detail::FunctionInterface<ReturnType(ArgTypes...)> *f) {
        ref = f;
        if (ref) {
            ref->inc();
        }
    }
    /**
     * Move constructor
     * This constructor steals the reference from the rvalue-reference Function
     * without incrementing the reference count.
     */
    Function(Function &&f): ref(f.ref) {}

    Function(ReturnType (*f)(ArgTypes...)) {
        typedef detail::StaticContainer<ReturnType(ArgTypes...)> staticFP;
        staticFP * newf = reinterpret_cast<staticFP *>(detail::StaticFPAllocator.alloc());
        CORE_UTIL_ASSERT_MSG(newf, "Function container memory allocation failed");
        new(newf) staticFP(f);
        ref = newf;
        ref->inc();
    }
    void attach(ReturnType (*f)(ArgTypes...)) {
        Function<ReturnType(ArgTypes...)> func(f);
        *this = func;
    }
    template <class C>
    Function(C *o, ReturnType (C::*fp)(ArgTypes...)) {
        typedef detail::MemberContainer<C, ReturnType(ArgTypes...)> memberFP;
        memberFP * newf = reinterpret_cast<memberFP *>(detail::MemberFPAllocator.alloc());
        CORE_UTIL_ASSERT_MSG(newf, "Function container memory allocation failed");
        new(newf) memberFP(o, fp);
        ref = newf;
        ref->inc();
    }
    template <class C>
    void attach(C *o, ReturnType (C::*fp)(ArgTypes...)) {
        Function<ReturnType(ArgTypes...)> func(o,fp);
        *this = func;
    }


    //Optimization Note: use template metaprogramming to select one of three allocator pools
    template <typename F>
    Function(const F &f) {
        typedef detail::FunctorContainer<F, ReturnType(ArgTypes...), detail::FunctorFPAllocator> FunctorFP;
        FunctorFP * newf = reinterpret_cast<FunctorFP *>(detail::FunctorFPAllocator.alloc());
        CORE_UTIL_ASSERT_MSG(newf, "Function container memory allocation failed");
        new(newf) FunctorFP(f);
        ref = newf;
        ref->inc();
    }
    template <typename F>
    void attach(const F & f) {
        Function<ReturnType(ArgTypes...)> func(f);
        *this = func;
    }
    Function(const Function & f): ref(f.ref) {
        if(ref) {
            ref->inc();
        }
    }

    template <typename... CapturedTypes, typename... ParentArgTypes>
    Function(std::tuple<CapturedTypes...>& t, Function<ReturnType(ParentArgTypes...)>& f) {
        typedef typename detail::CaptureFirst<ReturnType(ArgTypes...), detail::FunctorFPAllocator, CapturedTypes...> CaptureFP;
        static_assert(sizeof(CaptureFP) <= FUNCTOR_SIZE, "Size of bound arguments is too large" );
        CaptureFP * newf = reinterpret_cast<CaptureFP *>(detail::FunctorFPAllocator.alloc());
        CORE_UTIL_ASSERT_MSG(newf, "Function container memory allocation failed");
        new(newf) CaptureFP(t, f);
        ref = newf;
        ref->inc();
    }

    // Optimization note: This should be changed to use the same constructor as bind_last.
    template<typename... CapturedTypes>
    typename detail::RemoveFirstArgs<ReturnType(ArgTypes...), CapturedTypes...>::type bind_first(
        CapturedTypes... CapturedArgs)
    {
        std::tuple<CapturedTypes...> t(detail::forward<CapturedTypes>(CapturedArgs)...);
        typename detail::RemoveFirstArgs<ReturnType(ArgTypes...), CapturedTypes...>::type f(t,*this);
        return f;
    }

    template<typename... CapturedTypes>
    typename detail::RemoveLastArgs<ReturnType(),ReturnType(ArgTypes...),CapturedTypes...>::type bind_last(CapturedTypes... CapturedArgs)
    {
        using ReturnFP = typename detail::RemoveLastArgs<ReturnType(),ReturnType(ArgTypes...),CapturedTypes...>::type;
        static_assert(std::is_same<ReturnFP, functional::Function<void(int,int)> >::value, "oops");
        ReturnFP f(detail::bind_last(ReturnFP(), *this, detail::forward<CapturedTypes>(CapturedArgs)...));
        return f;
    }

    ~Function()
    {
        if (ref) {
            if (ref->dec() == 0) {
                ref->get_allocator()->free(ref);
            }
        }
    }
    Function & operator = (const Function & rhs)
    {
        if (ref) {
            if (ref->dec() == 0) {
                ref->get_allocator()->free(ref);
            }
        }
        ref = rhs.ref;
        if(ref) {
            ref->inc();
        }
    }
    inline ReturnType operator () (ArgTypes&&... Args) {
        return (*ref)(detail::forward<ArgTypes>(Args)...);
    }
    inline ReturnType call(ArgTypes&&... Args) {
        return (*ref)(detail::forward<ArgTypes>(Args)...);
    }
    void * get_ref() {
        if (ref)
            ref->inc();
        return reinterpret_cast<void *>(ref);
    }
    /* Note that this function does not use rvalue references beceause it is for C API interoperability */
    static ReturnType call_from_void(void *vref, ArgTypes... Args) {
        detail::FunctionInterface<ReturnType(ArgTypes...)> * sref =
            reinterpret_cast<detail::FunctionInterface<ReturnType(ArgTypes...)> *>(vref);
        CORE_UTIL_ASSERT_MSG(sref != NULL, "Cannot call a null Function object");
        return (*sref)(Args...);
    }
    static ReturnType call_from_void_rref(void *vref, ArgTypes&&... Args) {
        detail::FunctionInterface<ReturnType(ArgTypes...)> * sref =
            reinterpret_cast<detail::FunctionInterface<ReturnType(ArgTypes...)> *>(vref);
        CORE_UTIL_ASSERT_MSG(sref != NULL, "Cannot call a null Function object");
        return (*sref)(detail::forward<ArgTypes>(Args)...);
    }
    static ReturnType call_from_void_dec(void *vref, ArgTypes... Args) {
        detail::FunctionInterface<ReturnType(ArgTypes...)> * sref =
            reinterpret_cast<detail::FunctionInterface<ReturnType(ArgTypes...)> *>(vref);
        CORE_UTIL_ASSERT_MSG(sref != NULL, "Cannot call a null Function object");
        ReturnType r((*sref)(Args...));
        sref->dec();
        return r;
    }
    static ReturnType call_from_void_dec_rref(void *vref, ArgTypes&&... Args) {
        detail::FunctionInterface<ReturnType(ArgTypes...)> * sref =
            reinterpret_cast<detail::FunctionInterface<ReturnType(ArgTypes...)> *>(vref);
        CORE_UTIL_ASSERT_MSG(sref != NULL, "Cannot call a null Function object");
        ReturnType r((*sref)(detail::forward<ArgTypes>(Args)...));
        sref->dec();
        return r;
    }

protected:
    detail::FunctionInterface <ReturnType(ArgTypes...)> * ref;
};

namespace v0_compatibility {
/* This namespace contains the class definitions for duck-type compatibility
 * with v0 FunctionPointers
 */

template <typename ReturnType>
class FunctionPointer0 : public Function<ReturnType()> {};

template <typename ReturnType, typename A0>
class FunctionPointer1 : public Function<ReturnType(A0)> {};

template <typename ReturnType, typename A0, typename A1>
class FunctionPointer2 : public Function<ReturnType(A0, A1)> {};

template <typename ReturnType, typename A0, typename A1, typename A2>
class FunctionPointer3 : public Function<ReturnType(A0, A1, A2)> {};

typedef FunctionPointer0<void> FunctionPointer;
} // namespace v0_compatibility

} // namespace functional

#endif
