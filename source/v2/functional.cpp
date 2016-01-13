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

#if YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_STATICFP_INITIAL
#define STATICFP_INITIAL YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_STATICFP_INITIAL
#else
#define STATICFP_INITIAL 8
#endif

#if YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_STATICFP_GROWBY
#define STATICFP_GROWBY YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_STATICFP_GROWBY
#else
#define STATICFP_GROWBY STATICFP_INITIAL
#endif

#if YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_MEMBERFP_INITIAL
#define MEMBERFP_INITIAL YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_MEMBERFP_INITIAL
#else
#define MEMBERFP_INITIAL 8
#endif

#if YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_MEMBERFP_GROWBY
#define MEMBERFP_GROWBY YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_MEMBERFP_GROWBY
#else
#define MEMBERFP_GROWBY MEMBERFP_INITIAL
#endif

#if YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_FUNCTORFP_INITIAL
#define FUNCTORFP_INITIAL YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_FUNCTORFP_INITIAL
#else
#define FUNCTORFP_INITIAL 8
#endif

#if YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_FUNCTORFP_GROWBY
#define FUNCTORFP_GROWBY YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_FUNCTORFP_GROWBY
#else
#define FUNCTORFP_GROWBY FUNCTORFP_INITIAL
#endif

#if YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_FUNCTOR_SIZE
#define FUNCTOR_SIZE YOTTA_CONFIG_CORE_UTIL_FUNCTIONAL_FUNCTOR_SIZE
#else
#define FUNCTOR_SIZE (sizeof(MemberContainer<UnknownClass,void()>) + sizeof(void*) * 3)
#endif

namespace functional {
namespace detail {
const UAllocTraits_t NeverFreeTrait = {.flags = UALLOC_TRAITS_NEVER_FREE};

class UnknownClass;

ContainerAllocator StaticFPAllocator(STATICFP_INITIAL,
    STATICFP_GROWBY, sizeof(StaticContainer<void()>), NeverFreeTrait);

ContainerAllocator MemberFPAllocator(MEMBERFP_INITIAL,
    MEMBERFP_GROWBY, sizeof(MemberContainer<UnknownClass,void()>), NeverFreeTrait);

ContainerAllocator FunctorFPAllocator(FUNCTORFP_INITIAL,
    FUNCTORFP_GROWBY, sizeof(MemberContainer<UnknownClass,void()>), NeverFreeTrait);

} // namespace detail
} // namespace functional
