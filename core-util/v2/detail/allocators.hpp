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
#ifndef FUNCTIONAL_DETAIL_ALLOCATORS_HPP
#define FUNCTIONAL_DETAIL_ALLOCATORS_HPP

#include "interface.hpp"

#include "core-util/ExtendablePoolAllocator.h"
#include "ualloc/ualloc.h"

#include <cstdint>
namespace functional {
namespace detail {

class ContainerAllocator : public mbed::util::ExtendablePoolAllocator {
public:
    template <typename FunctionType>
    void free(FunctionInterface<FunctionType> * ptr) {
        ptr->~FunctionInterface<FunctionType>();
        mbed::util::ExtendablePoolAllocator::free(ptr);
    }
    ContainerAllocator(std::size_t initial_elements, std::size_t new_pool_elements, std::size_t element_size) :
        mbed::util::ExtendablePoolAllocator()
    {
        this->init(initial_elements, new_pool_elements, element_size, UAllocTraits_t{.flags = UALLOC_TRAITS_NEVER_FREE},
            MBED_UTIL_POOL_ALLOC_DEFAULT_ALIGN);
    }
};

template <std::size_t ALLOC_SIZE>
class ContainerAllocatorWrapper {
public:
    static const std::size_t size = ALLOC_SIZE;
    static ContainerAllocator & instance();
};

namespace alloc_size {
class BaseClass {
public:
    virtual void base_function(){}
};
class VirtualClass : public BaseClass {
    virtual void base_function(){}
};
class UnknownClass;

#ifndef YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_FUNCTOR_SIZE
/*
 * This size was chosen to allow a Function to bind a DNS response.
 * 4 bytes for a reference count
 * 4 bytes for the vtable pointers
 * 4 bytes for the base Function
 * 128/8 bytes for an IPv6 address
 * 4 bytes for a char* pointer.
 * 4 bytes of padding to round out to 8-byte alignment.
 */
#define YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_FUNCTOR_SIZE \
    ((sizeof(std::uint32_t) + sizeof(VirtualClass) + sizeof(UnknownClass *) + 128/8 + sizeof(char *) + 7) & ~7)
#endif

const std::size_t staticfp = sizeof(std::uint32_t) + sizeof(VirtualClass) + sizeof(void(*)());
const std::size_t memberfp = sizeof(std::uint32_t) + sizeof(VirtualClass) + sizeof(UnknownClass *) + sizeof(void (UnknownClass::*)());
const std::size_t functorfp = YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_FUNCTOR_SIZE;
}

using StaticFPAllocator  = ContainerAllocatorWrapper<alloc_size::staticfp>;
using MemberFPAllocator  = ContainerAllocatorWrapper<alloc_size::memberfp>;
using FunctorFPAllocator = ContainerAllocatorWrapper<alloc_size::functorfp>;

} // detail
} // functional
#endif // FUNCTIONAL_DETAIL_ALLOCATORS_HPP
