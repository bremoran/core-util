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

#ifndef CORE_UTIL_ATOMIC_QUEUE_H
#define CORE_UTIL_ATOMIC_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

struct lockfree_queue_element {
    struct lockfree_queue_element * volatile next;
    void * data;
};

struct lockfree_queue {
    struct lockfree_queue_element * volatile tail;
};

/**
 * \brief Add an element to the tail of the queue
 *
 * Since the queue only maintains a tail pointer, this simply inserts the new element before the tail pointer
 *
 * @param[in,out] q the queue structure to operate on
 * @param[in] e The element to add to the queue
 */
void lfq_push_tail(struct lockfree_queue * q, struct lockfree_queue_element * e);
/**
 * \brief Get an element from the head of the queue
 *
 * This function iterates over the queue and removes an element from the head when it finds the head. This is slower
 * than maintaining a head pointer, but it is necessary to ensure that a pop is completely atomic.
 *
 * @param[in,out] q The queue to pop from
 * @return The popped element or NULL if the queue was empty
 */
struct lockfree_queue_element * lfq_pop_head(struct lockfree_queue * q);
/**
 * Check if there are any elements in the queue
 *
 * Note that there is no guarantee that a queue which is not empty when this API is called will not be become empty
 * before lfq_pop_head is called
 *
 * @retval non-zero when the queue is empty
 * @retval 0 when the queue is not empty
 */
int lfq_empty(struct lockfree_queue * q);
/**
 * Iterates over the queue and counts the elements in the queue
 *
 * The value returned by this function may be invalid by the time it returns. Do not depend on this value except in
 * a critical section.
 *
 * @return the number of elements in the queue
 */
unsigned lfq_count(struct lockfree_queue * q);
#ifdef __cplusplus
}
#endif

#endif // CORE_UTIL_ATOMIC_QUEUE_H
