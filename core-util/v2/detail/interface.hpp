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

#include "core-util/atomic_ops.h"
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
    virtual ContainerAllocator & get_allocator() = 0;
    inline uint32_t inc()
    {
        return mbed::util::atomic_incr(&refcnt, static_cast<uint32_t>(1));
    }
    inline uint32_t dec()
    {
        return mbed::util::atomic_decr(&refcnt, static_cast<uint32_t>(1));
    }
protected:
    uint32_t refcnt;
};

} // namespace detail
} // namespace functional

#endif // FUNCTIONAL_V2_DETAIL_INTERFACE_HPP
