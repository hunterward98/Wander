#pragma once

#include "wander/core/types.h"
#include <cstdlib>

namespace wander {

// Arena allocator — linear allocation, bulk reset
// Great for per-frame temporary allocations
class ArenaAllocator {
public:
    ArenaAllocator(size_t capacity);
    ~ArenaAllocator();

    // Non-copyable
    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    void* alloc(size_t size, size_t alignment = 8);
    void reset();

    size_t used() const { return offset_; }
    size_t capacity() const { return capacity_; }

private:
    u8* buffer_ = nullptr;
    size_t capacity_ = 0;
    size_t offset_ = 0;
};

// Pool allocator — fixed-size block allocation
// Great for entities, components, particles
class PoolAllocator {
public:
    PoolAllocator(size_t block_size, size_t block_count);
    ~PoolAllocator();

    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;

    void* alloc();
    void free(void* ptr);

    size_t used_blocks() const { return used_count_; }
    size_t total_blocks() const { return block_count_; }

private:
    u8* buffer_ = nullptr;
    void* free_list_ = nullptr;
    size_t block_size_ = 0;
    size_t block_count_ = 0;
    size_t used_count_ = 0;
};

} // namespace wander
