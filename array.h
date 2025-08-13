#pragma once

#include "basic.h"
#include "allocator.h"
#include "assert.h"

template <typename T>
struct Array {
    T*         data;
    u64        length;
    Allocator* allocator;

    T* begin() { return data; }
    T* end()   { return &data[length - 1]; }

    const T* begin() const { return data; }
    const T* end()   const { return &data[length - 1]; }

    T& operator[](u64 i) {
        Assert(i < count, "Index outside the bounds of the array");
        return data[i];
    }

    const T& operator[](u64 i) const {
        Assert(i < count, "Index outside the bounds of the array");
        return data[i];
    }

    Array(u64 length, Allocator* allocator = &Allocator_Std) : length(length),
                                                               allocator(allocator) {
        data = (T*)allocator_alloc(allocator, sizeof(T) * length);
        Assert(data, "Cannot allocate data for the array.");
    }

    ~Array() {
        if (allocator == &Allocator_Temp) return;

        allocator_free(allocator, data);
    }
};

template <typename T>
static inline
Array<T>*
array_make(u64 length, Allocator* allocator = &Allocator_Std);

template <typename T>
static inline
void
array_realloc(Array<T>* array, u64 length);

template <typename T>
static inline
void
array_free(Array<T>* array);

template <typename T>
static inline
T
array_get(Array<T>* array, u64 index);

template <typename T>
static inline
T*
array_get_ptr(Array<T>* array, u64 index);

template <typename T>
static inline
void
array_set(Array<T>* array, u64 index, T elem);

template <typename T>
static inline
Array<T>*
array_make(u64 length, Allocator* allocator) {
    auto array = (Array<T>*)allocator_alloc(allocator, sizeof(Array<T>));
    Assert(array, "Cannot allocate array.");
    auto data = (T*)allocator_alloc(allocator, sizeof(T) * length);
    Assert(data, "Cannot allocate data for the array.");

    array->data      = data;
    array->length    = length;
    array->allocator = allocator;

    return array;
}

template <typename T>
static inline
void
array_realloc(Array<T>* array, u64 length) {
    Assert(length > array->length, "Cannot resize array with less size.");
    if (array->allocator == &Allocator_Temp) {
        T* new_data = (T*)allocator_alloc(array->allocator, sizeof(T) * length);
        Assert(new_data, "Cannot allocate enough memory for new array");

        for (u64 i = 0; i < array->length; i++) {
            new_data[i] = array->data[i];
        }

        array->data = new_data;
    } else {
        array->data = (T*)allocator_realloc(array->allocator, array->data, sizeof(T) * length);
    }

    Assert(array->data, "Cannot realloc array");
    array->length = length;
}

template <typename T>
static inline
void
array_free(Array<T>* array) {
    if (array->allocator == &Allocator_Temp) return;

    allocator_free(array->allocator, array->data);
    allocator_free(array->allocator, array);
}

template <typename T>
static inline
T
array_get(Array<T>* array, u64 index) {
    Assert(index < array->length - 1, "Index outside bounds of the array.");
    return array->data[index];
}

template <typename T>
static inline
T*
array_get_ptr(Array<T>* array, u64 index) {
    Assert(index < array->length - 1, "Index outside bounds of the array.");
    return &array->data[index];
}

template <typename T>
static inline
void
array_set(Array<T>* array, u64 index, T elem) {
    Assert(index < array->length - 1, "Index outside bounds of the array.");
    array->data[index] = elem;
}