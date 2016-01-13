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

namespace functional {
namespace detail {

class ContainerAllocator : public mbed::util::ExtendablePoolAllocator {
public:
    ContainerAllocator(size_t initial_elements, size_t new_pool_elements, size_t element_size,
        UAllocTraits_t alloc_traits, unsigned alignment = MBED_UTIL_POOL_ALLOC_DEFAULT_ALIGN)
    {
        this->init(initial_elements, new_pool_elements, element_size, alloc_traits, alignment);
    }
    template <typename FunctionType>
    bool free(FunctionInterface<FunctionType> * ptr) {
        ptr->~FunctionInterface<FunctionType>();
        mbed::util::ExtendablePoolAllocator::free(ptr);
        return true;
    }
};

extern ContainerAllocator StaticFPAllocator;
extern ContainerAllocator MemberFPAllocator;
extern ContainerAllocator FunctorFPAllocator;


} // detail
} // functional
#endif // FUNCTIONAL_DETAIL_ALLOCATORS_HPP
