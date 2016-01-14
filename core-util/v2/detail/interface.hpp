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
#ifndef FUNCTIONAL_V2_DETAIL_INTERFACE_HPP
#define FUNCTIONAL_V2_DETAIL_INTERFACE_HPP

#include "cmsis.h"
namespace functional {
namespace detail {
template <typename FunctionType>
class FunctionInterface;
} // namespace detail
} // namespace functional

#include "allocators.hpp"

namespace functional {
namespace detail {
template <typename ReturnType, typename... ArgTypes>
class FunctionInterface <ReturnType(ArgTypes...)> {
public:
    FunctionInterface() : refcnt(0) {}
    virtual ReturnType operator () (ArgTypes&&... Args) = 0;
    bool inc() {
        uint32_t tmp;
        do {
            tmp = __LDREXW(&refcnt) + 1;
        } while (__STREXW(tmp, &refcnt));
        return true;
    }
    bool dec() {
        uint32_t tmp;
        do {
            tmp = __LDREXW(&refcnt) - 1;
        } while (__STREXW(tmp, &refcnt));
        return tmp == 0;
    }
    virtual ContainerAllocator *getAllocator() = 0;
protected:
    volatile uint32_t refcnt;
};

} // namespace detail
} // namespace functional

#endif // FUNCTIONAL_V2_DETAIL_INTERFACE_HPP
