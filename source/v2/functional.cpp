/*
 * PackageLicenseDeclared: Apache-2.0
 * Copyright (c) 2015 ARM Limited
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

#include "core-util/v2/functional.hpp"

#ifndef YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_DEFAULT_INITIAL
#define YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_DEFAULT_INITIAL 4
#endif

#ifndef YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_DEFAULT_GROWBY
#define YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_DEFAULT_GROWBY 4
#endif


#ifndef YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_STATICFP_INITIAL
#define YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_STATICFP_INITIAL YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_DEFAULT_INITIAL
#endif

#ifndef YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_STATICFP_GROWBY
#define YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_STATICFP_GROWBY YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_DEFAULT_GROWBY
#endif

#ifndef YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_MEMBERFP_INITIAL
#define YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_MEMBERFP_INITIAL YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_DEFAULT_INITIAL
#endif

#ifndef YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_MEMBERFP_GROWBY
#define YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_MEMBERFP_GROWBY YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_DEFAULT_GROWBY
#endif

#ifndef YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_FUNCTORFP_INITIAL
#define YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_FUNCTORFP_INITIAL YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_DEFAULT_INITIAL
#endif

#ifndef YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_FUNCTORFP_GROWBY
#define YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_FUNCTORFP_GROWBY YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_DEFAULT_GROWBY
#endif

namespace functional {
namespace detail {

template <std::size_t ALLOC_SIZE>
ContainerAllocator & ContainerAllocatorWrapper<ALLOC_SIZE>::instance()
{
    static ContainerAllocator instance(
        YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_DEFAULT_INITIAL,
        YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_DEFAULT_GROWBY,
        ALLOC_SIZE
    );
    return instance;
}

template <>
ContainerAllocator & ContainerAllocatorWrapper<alloc_size::staticfp>::instance()
{
    static_assert(alloc_size::staticfp == sizeof(StaticContainer<void()>), "alloc_size::staticfp mismatch");
    static ContainerAllocator instance(
        YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_STATICFP_INITIAL,
        YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_STATICFP_GROWBY,
        alloc_size::staticfp
    );
    return instance;
}

class UnknownClass;

template <>
ContainerAllocator & ContainerAllocatorWrapper<alloc_size::memberfp>::instance()
{
    static_assert(alloc_size::memberfp == sizeof(MemberContainer<UnknownClass,void()>), "alloc_size::staticfp mismatch");
    static ContainerAllocator instance(
        YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_MEMBERFP_INITIAL,
        YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_MEMBERFP_GROWBY,
        alloc_size::memberfp
    );
    return instance;
}

template <>
ContainerAllocator & ContainerAllocatorWrapper<alloc_size::functorfp>::instance()
{
    static ContainerAllocator instance(
        YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_MEMBERFP_INITIAL,
        YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_MEMBERFP_GROWBY,
        alloc_size::functorfp
    );
    return instance;
}

} // namespace detail
} // namespace functional
