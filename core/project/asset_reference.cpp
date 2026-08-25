#include "asset_reference.h"
#include <filesystem>

namespace FreeEffect {

AssetReference::AssetReference(const std::string& path, const std::string& name, AssetType type)
    : m_id(generateUUID())
    , m_path(path)
    , m_name(name)
    , m_type(type) {
    // Check if file exists on disk
    if (!std::filesystem::exists(path)) {
        m_status = AssetStatus::Missing;
    }
}

} // namespace FreeEffect
