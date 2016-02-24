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

#ifndef __MBED_UTIL_CRITICAL_H__
#define __MBED_UTIL_CRITICAL_H__

#ifdef __cplusplus
    namespace mbed {
    namespace util {
    extern "C" {
#endif

#include <assert.h>

/*
 * critical_section_enter
 *
 * This function should be called to mark the start of a critical section of code.
 * NOTES:
 * 1) The use of this style of critical section is targetted at C based implementations.
 * 2) These critical sections can be nested.
 * 3) The interrupt enable state on entry to the first critical section (of a nested set, or single
 *    section) will be preserved on exit from the section.
 * 4) This implementation will currently only work on code running in privileged mode.
 */
void critical_section_enter();

/*
 * critical_section_exit
 *
 * This function should be called to mark the end of a critical section of code.
 * NOTES:
 * 1) The use of this style of critical section is targetted at C based implementations.
 * 2) These critical sections can be nested.
 * 3) The interrupt enable state on entry to the first critical section (of a nested set, or single
 *    section) will be preserved on exit from the section.
 * 4) This implementation will currently only work on code running in privileged mode.
 */
void critical_section_exit();

#ifdef __cplusplus
} // extern "C"
} // namspace util
} // namespace mbed
#endif


#endif // __MBED_UTIL_CRITICAL_H__
