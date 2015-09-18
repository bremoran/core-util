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

#include "mbed-util/irq_state.h"

#ifndef TARGET_LIKE_POSIX
#include "cmsis-core/core_generic.h"
#ifdef TARGET_NORDIC
#include "nrf_soc.h"
#endif /* #ifdef TARGET_NORDIC */
#else  /* #ifdef TARGET_LIKE_POSIX */
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#endif /* #ifdef TARGET_LIKE_POSIX */


namespace mbed {
namespace util {

static uint32_t IRQNestingDepth = 0;

irqstate_t pushDisableIRQState(){
    irqstate_t ret;
#ifdef TARGET_NORDIC
    sd_nvic_critical_region_enter(&ret);
#elif defined(TARGET_LIKE_POSIX)
    int rc;
    if (++IRQNestingDepth > 1) {
        rc = sigprocmask(SIG_BLOCK, NULL, &ret);
        assert(rc == 0);
        return ret;
    }

    sigset_t fullSet;
    rc = sigfillset(&fullSet);
    assert(rc == 0);
    rc = sigprocmask(SIG_BLOCK, &fullSet, &ret);
    assert(rc == 0);
#else
        ret = __get_PRIMASK();
        __disable_irq();
#endif
    return ret;
}

void popDisableIRQState(irqstate_t restore){
#ifdef TARGET_NORDIC
    sd_nvic_critical_region_exit(restore);
#elif defined(TARGET_LIKE_POSIX)
    assert(IRQNestingDepth > 0);
    if (--IRQNestingDepth == 0) {
        int rc = sigprocmask(SIG_SETMASK, &restore, NULL);
        assert(rc == 0);
    }
#else
    __set_PRIMASK(restore);
#endif
}

} // namespace util
} // namespace mbed
