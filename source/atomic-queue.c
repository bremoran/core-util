/*
 * SPDX-License-Identifier: Apache-2.0
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

#include "core-util/assert.h"
#include "core-util/atomic-queue.h"
#include <stddef.h>
#include "cmsis.h"

int atomic_cas_deref_uint32( uint32_t * volatile *ptr, uint32_t ** currentValue, uint32_t expectedValue, uint32_t *newValue, uintptr_t offset) {
    uint32_t *current;
    current = (uint32_t *)__LDREXW((volatile uint32_t *)ptr);
    if (currentValue != NULL) {
        *currentValue = current;
    }
    if (current == NULL) {
        return -1;
    } else if ( *(uint32_t *)((uintptr_t)current + offset) != expectedValue) {
        return 1;
    } else if(!__STREXW((uint32_t)newValue, (volatile uint32_t *)ptr)) {
        return 0;
    } else {
        return -1;
    }
}

void lfq_push_tail(struct lockfree_queue * q, struct lockfree_queue_element * e)
{
    CORE_UTIL_ASSERT_MSG(q != NULL, "null queue used");
    if (q == NULL) {
        return;
    }
    do {
        e->next = q->tail;
    } while (!__sync_bool_compare_and_swap(&q->tail, e->next, e));
}

struct lockfree_queue_element * lfq_pop_head(struct lockfree_queue * q)
{
    CORE_UTIL_ASSERT_MSG(q != NULL, "null queue used");
    if (q == NULL) {
        return NULL;
    }
    struct lockfree_queue_element * current;
    int fail = 1;
    while (fail) {
        // Set the element reference pointer to the tail pointer
        struct lockfree_queue_element * volatile * px = &q->tail;
        if (*px == NULL) {
            return NULL;
        }
        fail = 1;
        while (fail > 0) {
            fail = atomic_cas_deref_uint32((uint32_t * volatile *)px,
                            (uint32_t **)&current,
                            (uint32_t) NULL,
                            NULL,
                            offsetof(struct lockfree_queue_element, next));
            if (fail == 1) {
                px = &current->next;
            }
        }
    }
    return current;
}


int lfq_empty(struct lockfree_queue * q)
{
    return q->tail == NULL;
}

unsigned lfq_count(struct lockfree_queue *q)
{
    unsigned x;
    struct lockfree_queue_element * volatile e;
    if (lfq_empty(q)) {
        return 0;
    }
    e = q->tail;
    for (x = 1; e->next != NULL; x++, e = e->next);
    return x;
}
