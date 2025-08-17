#pragma once

#include "basic.h"
#include "allocator.h"
#include "assert.h"
#include <memory.h>
#include "hash_functions.h"

#define HASH_TABLE_INITIAL_LENGTH  256
#define HASH_TABLE_REALLOC_STEP    128
#define HASH_TABLE_MAX_LOAD_FACTOR 70

template <typename Value>
struct Hash_Table_Slot {
    Value value;
    u32   hash;
    bool  tombstone;
};

/*
    Fuck you, c++. I wanted to make hash function as a template parameter, but it looks ugly,
    unreadable and need 2 lines to write fucking template<bullshit>
    So now it's a function, you need to define somewhere in code before including this file.
    The name of the function should be exactly "get_hash" and the signature:
    u32 get_hash(T key);
    Oversimplified hash functions for int types are already defined in "hash_functions.h".
*/
template <typename Key, typename Value>
struct Hash_Table {
    Hash_Table_Slot<Value>* data;
    u32                     count;
    u32                     length;
    Allocator*              allocator;

    Hash_Table(u32 length = HASH_TABLE_INITIAL_LENGTH, Allocator* allocator = &Allocator_Std) :
                                                                   count(0),
                                                                   length(length),
                                                                   allocator(allocator) {
        data = (Hash_Table_Slot<Value>*)allocator_alloc(allocator, sizeof(Hash_Table_Slot<Value>) * length);
        Assert(data, "Cannot allocate memory for hash_table data.");

        memset(data, 0, sizeof(Hash_Table_Slot<Value>) * length);
    }

    ~Hash_Table() {
        if (allocator == &Allocator_Temp) return;

        allocator_free(allocator, data);
    }
};

template <typename Key, typename Value>
static inline
Hash_Table<Key, Value>*
hash_table_make(u32 length = HASH_TABLE_INITIAL_LENGTH, Allocator* allocator = &Allocator_Std);

template <typename Key, typename Value>
static inline
void
hash_table_realloc(Hash_Table<Key, Value>* hash_table, u32 length);

template <typename Key, typename Value>
static inline
void
hash_table_free(Hash_Table<Key, Value>* hash_table);

template <typename Key, typename Value>
static inline
void
hash_table_add(Hash_Table<Key, Value>* hash_table, Key key, Value value);

template <typename Key, typename Value>
static inline
void
hash_table_set(Hash_Table<Key, Value>* hash_table, Key key, Value value);

template <typename Key, typename Value>
static inline
bool
hash_table_add_or_set(Hash_Table<Key, Value>* hash_table, Key key, Value value); // Adds or sets element. If element with the same key already been added, returns true, otherwise return false.

template <typename Key, typename Value>
static inline
void
hash_table_remove(Hash_Table<Key, Value>* hash_table, Key key);

template <typename Key, typename Value>
static inline
bool
hash_table_remove_if_contains(Hash_Table<Key, Value>* hash_table, Key key); // Removes element from hash table if it exist. Returns true if element was removed, false if not.

template <typename Key, typename Value>
static inline
bool
hash_table_contains(Hash_Table<Key, Value>* hash_table, Key key);

template <typename Key, typename Value>
static inline
Value
hash_table_get(Hash_Table<Key, Value>* hash_table, Key key);

static inline
u32
hash_table_double_hash(u32 hash, u32 length, u32 iteration = 0);

// Implementation
template <typename Key, typename Value>
static inline
Hash_Table<Key, Value>*
hash_table_make(u32 length, Allocator* allocator) {
    auto hash_table = (Hash_Table<Key, Value>*)allocator_alloc(allocator, sizeof(Hash_Table<Key, Value>));
    Assert(hash_table, "Cannot allocate memory for hash_table.");
    auto data = (Hash_Table_Slot<Value>*)allocator_alloc(allocator, sizeof(Hash_Table_Slot<Value>) * length);
    Assert(data, "Cannot allocate memory for hash_table data.");

    memset(data, 0, sizeof(Hash_Table_Slot<Value>) * length);

    hash_table->data      = data;
    hash_table->count     = 0;
    hash_table->length    = length;
    hash_table->allocator = allocator;

    return hash_table;
}

template <typename Key, typename Value>
static inline
void
hash_table_realloc(Hash_Table<Key, Value>* hash_table, u32 length) {
    Assert(length > hash_table->length, "Cannot resize hash table with less size.");

    auto new_data = (Hash_Table_Slot<Value>*)allocator_alloc(hash_table->allocator, sizeof(Hash_Table_Slot<Value>) * length);
    Assert(new_data, "Cannot allocate enough memory for new hash table data");

    memset(new_data, 0, sizeof(Hash_Table_Slot<Value>) * length);

    for (u32 i = 0; i < hash_table->length; i++) {
        if (hash_table->data[i].hash != 0) {
            u32 iteration = 0;
            u32 index     = 0;

            while (true) {
                index = hash_table_double_hash(hash_table->data[i].hash, length, iteration++);

                if (new_data[index].hash == 0) break;
            }

            new_data[index].hash      = hash_table->data[i].hash;
            new_data[index].value     = hash_table->data[i].value;
            new_data[index].tombstone = false;
        }
    }

    if (hash_table->allocator != &Allocator_Temp) {
        allocator_free(hash_table->allocator, hash_table->data);
    }

    hash_table->data   = new_data;
    hash_table->length = length;
}

template <typename Key, typename Value>
static inline
void
hash_table_free(Hash_Table<Key, Value>* hash_table) {
    // nothing to free if using Allocator_Temp
    if (hash_table->allocator == &Allocator_Temp) return;

    allocator_free(hash_table->allocator, hash_table->data);
    allocator_free(hash_table->allocator, hash_table);
}

template <typename Key, typename Value>
static inline
void
hash_table_add(Hash_Table<Key, Value>* hash_table, Key key, Value value) {
    u32 hash      = get_hash(key);
    Assert(hash != 0, "Hash cannot be 0, fix your hash function.");
    u32 iteration = 0;
    u32 index     = 0;

    while (true) {
        index = hash_table_double_hash(hash, hash_table->length, iteration++);

        if (hash_table->data[index].tombstone)    continue;
        if (hash_table->data[index].hash == hash) break;
        if (hash_table->data[index].hash == 0)    break;
    }

    Assert(hash_table->data[index].hash != hash, "An item with the same key has already been added.");

    auto slot = Hash_Table_Slot<Value> {0};
    slot.hash  = hash;
    slot.value = value;

    hash_table->data[index] = slot;
    hash_table->count++;

    u32 load_factor = hash_table->count * 100 / hash_table->length;

    if (load_factor >= HASH_TABLE_MAX_LOAD_FACTOR) {
        hash_table_realloc(hash_table, hash_table->length + HASH_TABLE_REALLOC_STEP);
    }
}

template <typename Key, typename Value>
static inline
void
hash_table_set(Hash_Table<Key, Value>* hash_table, Key key, Value value) {
    u32 hash      = get_hash(key);
    u32 iteration = 0;
    u32 index     = 0;

    while (true) {
        index = hash_table_double_hash(hash, hash_table->length, iteration++);

        if (hash_table->data[index].tombstone)    continue;
        if (hash_table->data[index].hash == hash) break;
        if (hash_table->data[index].hash == 0)    break;
    }

    Assert(hash_table->data[index].hash == hash, "The key is not presented in the hash table.");

    auto slot  = Hash_Table_Slot<Value> {0};
    slot.hash  = hash;
    slot.value = value;

    hash_table->data[index] = slot;
}

template <typename Key, typename Value>
static inline
bool
hash_table_add_or_set(Hash_Table<Key, Value>* hash_table, Key key, Value value) {
    u32 hash      = get_hash(key);
    u32 iteration = 0;
    u32 index     = 0;

    while (true) {
        index = hash_table_double_hash(hash, hash_table->length, iteration++);

        if (hash_table->data[index].tombstone)    continue;
        if (hash_table->data[index].hash == hash) break;
        if (hash_table->data[index].hash == 0)    break;
    }

    bool has = false;

    if (hash_table->data[index].hash == hash) {
        has = true;
    }

    auto slot = Hash_Table_Slot<Value> {0};
    slot.hash  = hash;
    slot.value = value;
    hash_table->data[index] = slot;

    if (!has) {
        hash_table->count++;
        u32 load_factor = hash_table->count * 100 / hash_table->length;

        if (load_factor >= HASH_TABLE_MAX_LOAD_FACTOR) {
            hash_table_realloc(hash_table, hash_table->length + HASH_TABLE_REALLOC_STEP);
        }
    }

    return has;
}

template <typename Key, typename Value>
static inline
void
hash_table_remove(Hash_Table<Key, Value>* hash_table, Key key) {
    u32 hash      = get_hash(key);
    u32 iteration = 0;
    u32 index     = 0;

    while (true) {
        index = hash_table_double_hash(hash, hash_table->length, iteration++);

        if (hash_table->data[index].tombstone)    continue;
        if (hash_table->data[index].hash == hash) break;
        if (hash_table->data[index].hash == 0)    break;
    }

    Assert(hash_table->data[index].hash == hash, "The key was not presented in the hash table.");

    hash_table->data[index].tombstone = true;
    hash_table->data[index].hash      = 0;
    hash_table->data[index].value     = {0};
    hash_table->count--;
}

template <typename Key, typename Value>
static inline
bool
hash_table_remove_if_contains(Hash_Table<Key, Value>* hash_table, Key key) {
    u32 hash      = get_hash(key);
    u32 iteration = 0;
    u32 index     = 0;

    while (true) {
        index = hash_table_double_hash(hash, hash_table->length, iteration++);

        if (hash_table->data[index].tombstone)    continue;
        if (hash_table->data[index].hash == hash) break;
        if (hash_table->data[index].hash == 0)    break;
    }

    if (hash_table->data[index].hash != hash) {
        return false;
    }

    hash_table->data[index].tombstone = true;
    hash_table->data[index].hash      = 0;
    hash_table->data[index].value     = {0};
    hash_table->count--;

    return true;
}

template <typename Key, typename Value>
static inline
bool
hash_table_contains(Hash_Table<Key, Value>* hash_table, Key key) {
    u32 hash      = get_hash(key);
    u32 iteration = 0;
    u32 index     = 0;

    while (true) {
        index = hash_table_double_hash(hash, hash_table->length, iteration++);

        if (hash_table->data[index].tombstone)    continue;
        if (hash_table->data[index].hash == hash) break;
        if (hash_table->data[index].hash == 0)    break;
    }

    return hash_table->data[index].hash == hash;
}

template <typename Key, typename Value>
static inline
Value
hash_table_get(Hash_Table<Key, Value>* hash_table, Key key) {
    u32 hash      = get_hash(key);
    u32 iteration = 0;
    u32 index     = 0;

    while (true) {
        index = hash_table_double_hash(hash, hash_table->length, iteration++);

        if (hash_table->data[index].tombstone)    continue;
        if (hash_table->data[index].hash == hash) break;
        if (hash_table->data[index].hash == 0)    break;
    }

    Assert(hash_table->data[index].hash == hash, "The key was not presented in the hash table.");

    return hash_table->data[index].value;
}

static inline
u32
hash_table_double_hash(u32 hash, u32 length, u32 iteration) {
    return (1 + (hash + iteration * (hash % (length / 2)))) % length;
}