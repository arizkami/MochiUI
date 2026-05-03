#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <optional>

namespace AureliaUI {

// Re-export nlohmann::json under an AureliaUI alias so consumers don't need to
// know the underlying library name.
using Json = nlohmann::json;

// Safely read a key from a JSON object, returning defaultVal on any error.
template<typename T>
T jsonGet(const Json& j, const std::string& key, const T& defaultVal = T{}) noexcept {
    try {
        if (j.is_object() && j.contains(key) && !j.at(key).is_null())
            return j.at(key).get<T>();
    } catch (...) {}
    return defaultVal;
}

// Safely read a nested path ("outer.inner") from a JSON object.
template<typename T>
T jsonGetPath(const Json& j, const std::string& path, const T& defaultVal = T{}) noexcept {
    try {
        const Json* cur = &j;
        std::string key;
        for (size_t i = 0; i <= path.size(); ++i) {
            if (i == path.size() || path[i] == '.') {
                if (!cur->is_object() || !cur->contains(key)) return defaultVal;
                cur = &(*cur)[key];
                key.clear();
            } else {
                key += path[i];
            }
        }
        if (cur->is_null()) return defaultVal;
        return cur->get<T>();
    } catch (...) {}
    return defaultVal;
}

// Parse JSON from a string, returns empty Json on parse error.
inline Json jsonParse(const std::string& text) noexcept {
    try { return Json::parse(text); } catch (...) { return Json{}; }
}

} // namespace AureliaUI
