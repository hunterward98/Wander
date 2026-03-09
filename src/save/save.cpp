#include "wander/save/save.h"
#include "wander/core/log.h"
#include <fstream>

namespace wander {

bool save_write(const std::string& path, const void* data, size_t size) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        LOG_ERROR("Failed to open save file for writing: %s", path.c_str());
        return false;
    }
    file.write(static_cast<const char*>(data), size);
    return file.good();
}

bool save_read(const std::string& path, std::vector<u8>& out) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        LOG_ERROR("Failed to open save file for reading: %s", path.c_str());
        return false;
    }
    auto size = file.tellg();
    file.seekg(0);
    out.resize(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(out.data()), size);
    return file.good();
}

} // namespace wander
