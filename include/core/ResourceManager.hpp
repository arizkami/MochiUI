#pragma once
#include <string>
#include <map>
#include <vector>
#include <memory>

namespace AureliaUI {

class ResourceManager {
public:
    static ResourceManager& getInstance();

    void registerResource(const std::string& path, const uint8_t* data, size_t size);

    struct ResourceData {
        const uint8_t* data;
        size_t size;
    };

    ResourceData getResource(const std::string& path);
    std::string getResourceString(const std::string& path);

private:
    ResourceManager() = default;
    std::map<std::string, ResourceData> resources;
};

} // namespace AureliaUI
