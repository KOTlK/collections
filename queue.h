#pragma once

#include "basic.h"
#include "allocator.h"
#include "assert.h"

#define QUEUE_INITIAL_LENGTH 256
#define QUEUE_REALLOC_STEP   128

template <typename T>
struct Queue {
    T*         data;
    u32        count;
    u32        length;
    u32        head;
    u32        tail;
    Allocator* allocator;

    Queue(u32 length = QUEUE_INITIAL_LENGTH, Allocator* allocator = &Allocator_Std) : count(0),
                                                                                      length(length),
                                                                                      head(0),
                                                                                      tail(0),
                                                                                      allocator(allocator) {
        data = (T*)allocator_alloc(allocator, sizeof(T) * length);
        Assert(data, "Cannot allocate memory for queue data.");
    }

    ~Queue() {
        if (allocator == &Allocator_Temp) return;

        allocator_free(allocator, data);
    }
};

template <typename T>
static inline
Queue<T>*
queue_make(u32 length = QUEUE_INITIAL_LENGTH, Allocator* allocator = &Allocator_Std);

template <typename T>
static inline
void
queue_realloc(Queue<T>* queue, u32 length);

template <typename T>
static inline
void
queue_free(Queue<T>* queue);

template <typename T>
static inline
void
queue_enqueue(Queue<T>* queue, T elem);

template <typename T>
static inline
T
queue_dequeue(Queue<T>* queue);

template <typename T>
static inline
void
queue_clear(Queue<T>* queue);

// Implementation

template <typename T>
static inline
Queue<T>*
queue_make(u32 length, Allocator* allocator) {
    auto queue = (Queue<T>*)allocator_alloc(allocator, sizeof(Queue<T>));
    Assert(queue, "Cannot allocate memory for queue.");
    auto data = (T*)allocator_alloc(allocator, sizeof(T) * length);
    Assert(data, "Cannot allocate memory for queue data");

    queue->data      = data;
    queue->count     = 0;
    queue->length    = length;
    queue->head      = 0;
    queue->tail      = 0;
    queue->allocator = allocator;

    return queue;
}

template <typename T>
static inline
void
queue_realloc(Queue<T>* queue, u32 length) {
    Assert(length > queue->length, "Cannot resize queue with less size.");
    if (queue->allocator == &Allocator_Temp) {
        T* new_data = (T*)allocator_alloc(queue->allocator, sizeof(T) * length);
        Assert(new_data, "Cannot allocate enough memory for new queue");

        u32 head = queue->head;
        u32 tail = queue->tail;

        queue->head = 0;
        queue->tail = 0;

        while (head != tail) {
            new_data[queue->tail++] = queue->data[head];
            head = (head + 1) % queue->length;
        }

        queue->data = new_data;
    } else {
        T* origin = queue->data;
        queue->data = (T*)allocator_realloc(queue->allocator, queue->data, sizeof(T) * length);
        Assert(queue->data, "Cannot allocate enough memory for new queue");

        if (queue->head > queue->tail || queue->head == queue->tail) {
            u32 start = queue->length;
            u32 end   = queue->tail;

            for (u32 i = 0; i < end; i++) {
                u32 index = (start + i) % length;
                queue->data[index] = origin[i];
            }

            queue->tail = (start + end) % length;
        }
    }

    queue->length = length;
}

template <typename T>
static inline
void
queue_free(Queue<T>* queue) {
    if (queue->allocator == &Allocator_Temp) return;

    allocator_free(queue->allocator, queue->data);
    allocator_free(queue->allocator, queue);
}

template <typename T>
static inline
void
queue_enqueue(Queue<T>* queue, T elem) {
    if (queue->count >= queue->length)
        queue_realloc(queue, queue->count + 1 + QUEUE_REALLOC_STEP);

    u32 index   = queue->tail;
    queue->tail = (queue->tail + 1) % queue->length;
    queue->count++;
    queue->data[index] = elem;
}

template <typename T>
static inline
T
queue_dequeue(Queue<T>* queue) {
    Assert(queue->count > 0, "Cannot dequeu if queue is empty.");
    T elem = queue->data[queue->head];
    queue->count--;
    queue->head = (queue->head + 1) % queue->length;

    if (queue->count == 0) {
        queue->head = 0;
        queue->tail = 0;
    }

    return elem;
}

template <typename T>
static inline
void
queue_clear(Queue<T>* queue) {
    queue->head  = 0;
    queue->tail  = 0;
    queue->count = 0;
}