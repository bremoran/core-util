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
#ifndef __FUNCTIONPOINTER_HPP__
#define __FUNCTIONPOINTER_HPP__

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <new>
#include <utility>
#include <cstdio>
#include <type_traits>

namespace mbed {
namespace util {


namespace impl{
template <typename FunctionType>
class FunctionPointerInterface;

class FunctionPointerStorage {
public:
    // Forward declaration of an unknown class
    class UnknownClass;
    // Forward declaration of an unknown member function to this an unknown class
    // this kind of declaration is authorized by the standard (see 8.3.3/2 of C++ 03 standard).
    // As a result, the compiler will allocate for UnknownFunctionMember_t the biggest size
    // and biggest alignment possible for member function.
    // This type can be used inside unions, it will help to provide the storage
    // with the proper size and alignment guarantees
    typedef void (UnknownClass::*UnknownFunctionMember_t)();

    union {

        void * _static_fp;
        struct {
            union {
                char _member[sizeof(UnknownFunctionMember_t)];
                UnknownFunctionMember_t _alignementAndSizeGuarantees;
            };
            void * _object;
        } _method;
        char _functor_storage[sizeof(UnknownFunctionMember_t) + sizeof(void *)];
        void * _external_functor;
    };
};

class FunctionPointerSize0 {
public:
    FunctionPointerStorage _fp;
    virtual void nullmethod()=0;
};
class FunctionPointerSize : public FunctionPointerSize0 {
public:
    void nullmethod() {}
};
template<class T>
T&& forward(typename std::remove_reference<T>::type& a) noexcept
{
    return static_cast<T&&>(a);
}
template<class T>
T&& forward(typename std::remove_reference<T>::type&& a) noexcept
{
    return static_cast<T&&>(a);
}

template<typename FunctionType>
class StaticPointer;

template<typename FunctionType, typename C>
class MethodPointer;

template<typename FunctionType, typename F>
class FunctorPointer;
}

template<typename FunctionType>
class FunctionPointer;


#include "impl/FP0.hpp"
#include "impl/FP1.hpp"
} // namespace util
} // namespace mbed

#endif
