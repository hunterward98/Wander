#pragma once

#include "wander/core/types.h"
#include <string>
#include <vector>

namespace wander {

// Save system — stub for Phase 1
// Full implementation in Phase 3

bool save_write(const std::string& path, const void* data, size_t size);
bool save_read(const std::string& path, std::vector<u8>& out);

} // namespace wander
