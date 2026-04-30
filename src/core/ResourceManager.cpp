#include <core/ResourceManager.hpp>

namespace MochiUI {

ResourceManager& ResourceManager::getInstance() {
    static ResourceManager instance;
    return instance;
}

void ResourceManager::registerResource(const std::string& path, const uint8_t* data, size_t size) {
    resources[path] = {data, size};
}

ResourceManager::ResourceData ResourceManager::getResource(const std::string& path) {
    auto it = resources.find(path);
    if (it != resources.end()) return it->second;
    return {nullptr, 0};
}

std::string ResourceManager::getResourceString(const std::string& path) {
    auto res = getResource(path);
    if (res.data) return std::string(reinterpret_cast<const char*>(res.data), res.size);
    return "";
}

} // namespace MochiUI
