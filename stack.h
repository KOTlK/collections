#pragma once

#include "basic.h"
#include "allocator.h"
#include "assert.h"

#define STACK_INITIAL_LENGTH 256
#define STACK_REALLOC_STEP   128

template <typename T>
struct Stack {
    T*         data;
    u32        count;
    u32        length;
    Allocator* allocator;

    Stack(u32 length = STACK_INITIAL_LENGTH, Allocator* allocator = &Allocator_Std) : count(0),
                                                                                      length(length),
                                                                                      allocator(allocator) {
        data = (T*)allocator_alloc(allocator, sizeof(T) * length);
        Assert(data, "Cannot allocate memory for stack data.");
    }

    ~Stack() {
        if (allocator == &Allocator_Temp) return;

        allocator_free(allocator, data);
    }
};

template <typename T>
static inline
Stack<T>*
stack_make(u32 length = STACK_INITIAL_LENGTH, Allocator* allocator = &Allocator_Std);

template <typename T>
static inline
void
stack_realloc(Stack<T>* stack, u32 length);

template <typename T>
static inline
void
stack_free(Stack<T>* stack);

template <typename T>
static inline
void
stack_push(Stack<T>* stack, T element);

template <typename T>
static inline
T
stack_cuck(Stack<T>* stack);

template <typename T>
static inline
T
stack_pop(Stack<T>* stack);

template <typename T>
static inline
void
stack_clear(Stack<T>* stack);

// Implementation
template <typename T>
static inline
Stack<T>*
stack_make(u32 length, Allocator* allocator) {
    auto stack = (Stack<T>*)allocator_alloc(allocator, sizeof(Stack<T>));
    Assert(stack, "Cannot allocate memory for stack.");
    auto data = (T*)allocator_alloc(allocator, sizeof(T) * length);
    Assert(data, "Cannot allocate memory for stack data.");

    stack->data      = data;
    stack->count     = 0;
    stack->length    = length;
    stack->allocator = allocator;

    return stack;
}

template <typename T>
static inline
void
stack_realloc(Stack<T>* stack, u32 length) {
    Assert(length > stack->length, "Cannot resize stack with less size.");
    if (stack->allocator == &Allocator_Temp) {
        T* new_data = (T*)allocator_alloc(stack->allocator, sizeof(T) * length);
        Assert(new_data, "Cannot allocate enough memory for new stack");

        for (u32 i = 0; i < stack->count; i++) {
            new_data[i] = stack->data[i];
        }

        stack->data = new_data;
    } else {
        stack->data = (T*)allocator_realloc(stack->allocator, stack->data, sizeof(T) * length);
    }

    Assert(stack->data, "Cannot allocate enough memory for new stack");
    stack->length = length;
}

template <typename T>
static inline
void
stack_free(Stack<T>* stack) {
    allocator_free(stack->allocator, stack->data);
    allocator_free(stack->allocator, stack);
}

template <typename T>
static inline
void
stack_push(Stack<T>* stack, T element) {
    if (stack->count >= stack->length) {
        stack_realloc(stack, stack->count + 1 + STACK_REALLOC_STEP);
    }

    stack->data[stack->count++] = element;
}

template <typename T>
static inline
T
stack_cuck(Stack<T>* stack) {
    Assert(stack->count > 0, "You are trying to cuck element, but stack is empty");
    return stack->data[stack->count - 1];
}

template <typename T>
static inline
T
stack_pop(Stack<T>* stack) {
    Assert(stack->count > 0, "You are trying to pop element, but stack is empty");
    return stack->data[--stack->count];
}

template <typename T>
static inline
void
stack_clear(Stack<T>* stack) {
    stack->count = 0;
}