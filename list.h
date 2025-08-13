#pragma once

#include "basic.h"
#include "allocator.h"
#include "assert.h"

#define LIST_DEFAULT_LENGTH 256
#define LIST_REALLOC_STEP 128

template <typename T>
struct List {
    T*         data;
    u32        count;
    u32        length;
    Allocator* allocator;

    T* begin() { return data; }
    T* end()   { return &data[count]; }

    const T* begin() const { return data; }
    const T* end()   const { return &data[count]; }

    T& operator[](u32 i)             { return data[i]; }
    const T& operator[](u32 i) const { return data[i]; }
};

template <typename T>
static inline
List<T>*
list_make(u32 length = LIST_DEFAULT_LENGTH, Allocator* allocator = &Std_Allocator);

template <typename T>
static inline
void
list_realloc(List<T> *list, u32 length);

template <typename T>
static inline
void
list_free(List<T> *list);

template <typename T>
static inline
void
list_add(List<T> *list, T element);

template <typename T>
static inline
void
list_remove(List<T> *list, T element);

template <typename T>
static inline
void
list_remove_swap_back(List<T> *list, T element);

template <typename T>
static inline
void
list_remove_at(List<T> *list, u32 index);

template <typename T>
static inline
void
list_remove_at_swap_back(List<T> *list, u32 index);

template <typename T>
static inline
void
list_set(List<T> *list, u32 index, T element);

template <typename T>
static inline
T
list_get(List<T> *list, u32 index);

template <typename T>
static inline
T*
list_get_ptr(List<T> *list, u32 index);

template <typename T>
static inline
void
list_quick_sort(List<T> *list);

// Implementation
template <typename T>
static inline
List<T>*
list_make(u32 length, Allocator* allocator) {
    auto list = (List<T>*)allocator->alloc(allocator, sizeof(List<T>));
    Assert(list != null, "Cannot allocate list.");
    auto data = (T*)allocator->alloc(allocator, sizeof(T) * length);
    Assert(list != null, "Cannot allocate list data.");

    list->data      = data;
    list->count     = 0;
    list->length    = length;
    list->allocator = allocator;

    return list;
}

template <typename T>
static inline
void
list_realloc(List<T> *list, u32 length) {
    list->data   = (T*)list->allocator->realloc(list->allocator, list->data, length);
    list->length = length;
}

template <typename T>
static inline
void
list_free(List<T> *list) {
    list->allocator->free(list->allocator, list->data);
    list->allocator->free(list->allocator, list);
}

template <typename T>
static inline
void
list_add(List<T> *list, T element) {
    if (list->length == list->count) {
        list_realloc(list, list->length + LIST_REALLOC_STEP);
    }

    list->data[list->count++] = element;
}

template <typename T>
static inline
void
list_remove(List<T> *list, T element) {
    u32 i = 0;
    for(; i < list->count; i++) {
        if (list->data[i] == element) {
            list->count--;
            break;
        }
    }

    for (; i < list->count; i++) {
        list->data[i] = list->data[i + 1];
    }
}

template <typename T>
static inline
void
list_remove_swap_back(List<T> *list, T element) {
    for(u32 i = 0; i < list->count; i++) {
        if (list->data[i] == element) {
            list->data[i] = list->data[--list->count];
            break;
        }
    }
}

template <typename T>
static inline
void
list_remove_at(List<T> *list, u32 index) {
    Assert(index < list->count, "Index outside the bounds of the list");
    list->count--;
    for (u32 i = index; i < list->count; i++) {
        list->data[i] = list->data[i + 1];
    }
}

template <typename T>
static inline
void
list_remove_at_swap_back(List<T> *list, u32 index) {
    Assert(index < list->count, "Index outside the bounds of the list");
    list->data[index] = list->data[--list->count];
}

template <typename T>
static inline
void
list_set(List<T> *list, u32 index, T element) {
    Assert(index < list->count, "Index outside the bounds of the list");
    list->data[index] = element;
}

template <typename T>
static inline
T
list_get(List<T> *list, u32 index) {
    Assert(index < list->count, "Index outside the bounds of the list");
    return list->data[index];
}

template <typename T>
static inline
T*
list_get_ptr(List<T> *list, u32 index) {
    Assert(index < list->count, "Index outside the bounds of the list");
    return &list->data[index];
}

template <typename T>
static inline
void
swap(T *a, T *b) {
    T temp = *a;
    *a = *b;
    *b = temp;
}

template <typename T>
static inline
void
quick_sort(T *arr, u32 low, u32 high) {
    if (low < high) {
        auto pivot = arr[high];
        auto i     = low - 1;

        for (u32 j = low; j < high; j++) {
            if (arr[j] < pivot) {
                i += 1;
                swap(&arr[i], &arr[j]);
            }
        }

        swap(&arr[i + 1], &arr[high]);
        pivot = i + 1;

        quick_sort(arr, low, pivot - 1);
        quick_sort(arr, pivot + 1, high);
    }
}

template <typename T>
static inline
void
list_quick_sort(List<T> *list) {
    quick_sort(list->data, 0, list->count);
}