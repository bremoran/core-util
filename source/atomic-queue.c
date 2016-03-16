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

volatile int trip = 0;

int tripped() {
    return trip;
}

struct lockfree_queue_element * lfq_pop_head(struct lockfree_queue * q)
{
    struct lockfree_queue_element * rx;
    CORE_UTIL_ASSERT_MSG(q != NULL, "null queue used");
    if (q == NULL) {
        return NULL;
    }
    while (1) {
        // Set the element reference pointer to the tail pointer
        struct lockfree_queue_element * volatile * px = &q->tail;
        if (*px == NULL) {
            return NULL;
        }
        while (1) {
            rx = *px;
            if (rx == NULL) {
                break;
            }
            if (rx->next == NULL) {
                break;
            }
            px = &rx->next;
        }
        if (rx == NULL) {
            continue;
        }
        if(__sync_bool_compare_and_swap(px, rx, NULL)){
            break;
        }
    }
    return rx;
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
