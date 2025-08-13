#pragma once

#define null NULL

#include <stdint.h>

#define s8  int8_t
#define u8  uint8_t
#define s16 int16_t
#define u16 uint16_t
#define s32 int32_t
#define u32 uint32_t
#define s64 int64_t
#define u64 uint64_t

#include "allocator.h"
#include "std_allocator.h"
#include "arena.h"

static Allocator Std_Allocator = {
    .alloc   = std_alloc,
    .realloc = std_realloc,
    .free    = std_free,
    .context = null
};

static Allocator Temp_Allocator = {
    .alloc   = arena_alloc,
    .realloc = arena_realloc,
    .free    = arena_free,
    .context = arena_make()
};

static inline
void
free_temp_allocator() {
    Temp_Allocator.free(&Temp_Allocator, null);
}