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

#ifndef __MBED_UTIL_IRQ_STATE_H__
#define __MBED_UTIL_IRQ_STATE_H__

#include <stdint.h>

namespace mbed {
namespace util {

#ifdef TARGET_NORDIC
    typedef uint8_t  irqstate_t;
#elif defined(TARGET_LIKE_POSIX)
    typedef sigset_t irqstate_t;
#else
    typedef uint32_t irqstate_t;
#endif
/// @name IRQ State Management
irqstate_t pushDisableIRQState();
void popDisableIRQState(irqstate_t);

} // namespace util
} // namespace mbed

#endif //__MBED_UTIL_IRQ_STATE_H__
