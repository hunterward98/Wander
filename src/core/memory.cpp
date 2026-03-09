#include "wander/core/memory.h"
#include "wander/core/log.h"
#include <cstring>

namespace wander {

// --- ArenaAllocator ---

ArenaAllocator::ArenaAllocator(size_t capacity)
    : capacity_(capacity), offset_(0)
{
    buffer_ = static_cast<u8*>(std::malloc(capacity));
    if (!buffer_) {
        LOG_ERROR("ArenaAllocator: failed to allocate %zu bytes", capacity);
    }
}

ArenaAllocator::~ArenaAllocator() {
    std::free(buffer_);
}

void* ArenaAllocator::alloc(size_t size, size_t alignment) {
    // Align offset
    size_t aligned = (offset_ + alignment - 1) & ~(alignment - 1);
    if (aligned + size > capacity_) {
        LOG_ERROR("ArenaAllocator: out of memory (requested %zu, used %zu/%zu)",
            size, offset_, capacity_);
        return nullptr;
    }
    void* ptr = buffer_ + aligned;
    offset_ = aligned + size;
    return ptr;
}

void ArenaAllocator::reset() {
    offset_ = 0;
}

// --- PoolAllocator ---

PoolAllocator::PoolAllocator(size_t block_size, size_t block_count)
    : block_size_(block_size < sizeof(void*) ? sizeof(void*) : block_size)
    , block_count_(block_count)
    , used_count_(0)
{
    buffer_ = static_cast<u8*>(std::malloc(block_size_ * block_count));
    if (!buffer_) {
        LOG_ERROR("PoolAllocator: failed to allocate %zu bytes", block_size_ * block_count);
        return;
    }

    // Build free list
    free_list_ = nullptr;
    for (size_t i = block_count; i > 0; i--) {
        void* block = buffer_ + (i - 1) * block_size_;
        *static_cast<void**>(block) = free_list_;
        free_list_ = block;
    }
}

PoolAllocator::~PoolAllocator() {
    std::free(buffer_);
}

void* PoolAllocator::alloc() {
    if (!free_list_) {
        LOG_ERROR("PoolAllocator: no free blocks (%zu/%zu used)", used_count_, block_count_);
        return nullptr;
    }
    void* block = free_list_;
    free_list_ = *static_cast<void**>(block);
    used_count_++;
    return block;
}

void PoolAllocator::free(void* ptr) {
    if (!ptr) return;
    *static_cast<void**>(ptr) = free_list_;
    free_list_ = ptr;
    used_count_--;
}

} // namespace wander
