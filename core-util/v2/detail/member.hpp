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
#ifndef FUNCTIONAL_DETAIL_MEMBER_HPP
#define FUNCTIONAL_DETAIL_MEMBER_HPP

#include "interface.hpp"
#include "allocators.hpp"
#include "polyfill.hpp"

namespace functional {
namespace detail {

template <class C, typename FunctionType>
class MemberContainer;

template <class C, typename ReturnType, typename... ArgTypes>
class MemberContainer <C, ReturnType(ArgTypes...)> : public FunctionInterface <ReturnType(ArgTypes...)> {
public:
    typedef ReturnType (C::*MemberFPType)(ArgTypes...);
    MemberContainer() : obj(nullptr), fp (nullptr) {}
    MemberContainer(C * obj, MemberFPType fp) : obj(obj), fp(fp) {}
    virtual ReturnType operator () (ArgTypes&&... Args) {
        return (obj->*fp)(polyfill::forward<ArgTypes>(Args)...);
    }
    ContainerAllocator * get_allocator() {
        return & MemberFPAllocator;
    }

    // virtual void deallocate(FunctionInterface<ReturnType(ArgTypes...)> *ptr) {
    //     ptr->~FunctionInterface();
    //     StaticFPAllocator.free(ptr);
    // }

protected:
    // Forward declaration of an unknown class
    class UnknownClass;
    // Forward declaration of an unknown member function to this an unknown class
    // this kind of declaration is authorized by the standard (see 8.3.3/2 of C++ 03 standard).
    // As a result, the compiler will allocate for UnknownFunctionMember_t the biggest size
    // and biggest alignment possible for member function.
    // This type can be used inside unions, it will help to provide the storage
    // with the proper size and alignment guarantees
    typedef void (UnknownClass::*UnknownFunctionMember_t)();
    C * obj;
    union {
        MemberFPType fp;
        UnknownFunctionMember_t _alignementAndSizeGuarantees;
    };
};
} // namespace detail
} // namespace functional
#endif // FUNCTIONAL_DETAIL_MEMBER_HPP
