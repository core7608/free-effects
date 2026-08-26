#include "asset_library.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <random>

namespace FreeEffect {

std::string AssetLibrary::generateId() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);
    char buf[33];
    snprintf(buf, sizeof(buf), "%08x%08x%08x%08x",
             dis(gen), dis(gen), dis(gen), dis(gen));
    return std::string(buf);
}

bool AssetLibrary::createLibrary(const std::string& name, const std::string& path) {
    m_name = name;
    m_path = path;
    m_assets.clear();
    m_open = true;
    return saveToFile(path);
}

bool AssetLibrary::openLibrary(const std::string& path) {
    m_path = path;
    bool ok = loadFromFile(path);
    m_open = ok;
    return ok;
}

void AssetLibrary::closeLibrary() {
    if (m_open && !m_path.empty()) {
        saveToFile(m_path);
    }
    m_open = false;
    m_assets.clear();
    m_name.clear();
    m_path.clear();
}

std::vector<LibraryAsset> AssetLibrary::getAssetsByType(LibraryAssetType type) const {
    std::vector<LibraryAsset> result;
    std::copy_if(m_assets.begin(), m_assets.end(), std::back_inserter(result),
        [type](const LibraryAsset& a) { return a.type == type; });
    return result;
}

LibraryAsset AssetLibrary::getAsset(const std::string& assetId) const {
    auto it = std::find_if(m_assets.begin(), m_assets.end(),
        [&assetId](const LibraryAsset& a) { return a.id == assetId; });
    if (it != m_assets.end()) return *it;
    return {};
}

bool AssetLibrary::addAsset(const LibraryAsset& asset) {
    LibraryAsset a = asset;
    if (a.id.empty()) a.id = generateId();
    if (a.createdAt == std::chrono::system_clock::time_point{}) {
        a.createdAt = std::chrono::system_clock::now();
    }
    m_assets.push_back(a);
    return true;
}

bool AssetLibrary::removeAsset(const std::string& assetId) {
    auto it = std::find_if(m_assets.begin(), m_assets.end(),
        [&assetId](const LibraryAsset& a) { return a.id == assetId; });
    if (it == m_assets.end()) return false;
    m_assets.erase(it);
    return true;
}

bool AssetLibrary::updateAsset(const std::string& assetId, const LibraryAsset& asset) {
    auto it = std::find_if(m_assets.begin(), m_assets.end(),
        [&assetId](const LibraryAsset& a) { return a.id == assetId; });
    if (it == m_assets.end()) return false;
    *it = asset;
    it->id = assetId;
    return true;
}

bool AssetLibrary::addColorSwatch(const std::string& name, double r, double g, double b, double a) {
    LibraryAsset asset;
    asset.id = generateId();
    asset.name = name;
    asset.type = LibraryAssetType::Color;
    asset.metadata["r"] = std::to_string(r);
    asset.metadata["g"] = std::to_string(g);
    asset.metadata["b"] = std::to_string(b);
    asset.metadata["a"] = std::to_string(a);
    asset.createdAt = std::chrono::system_clock::now();
    m_assets.push_back(asset);
    return true;
}

bool AssetLibrary::updateColorSwatch(const std::string& name, double r, double g, double b, double a) {
    auto it = std::find_if(m_assets.begin(), m_assets.end(),
        [&name](const LibraryAsset& a) { return a.name == name && a.type == LibraryAssetType::Color; });
    if (it == m_assets.end()) return false;
    it->metadata["r"] = std::to_string(r);
    it->metadata["g"] = std::to_string(g);
    it->metadata["b"] = std::to_string(b);
    it->metadata["a"] = std::to_string(a);
    return true;
}

bool AssetLibrary::addCharacterStyle(const std::string& name, const std::string& fontFamily,
                                     double size, double leading, double tracking) {
    LibraryAsset asset;
    asset.id = generateId();
    asset.name = name;
    asset.type = LibraryAssetType::CharacterStyle;
    asset.metadata["fontFamily"] = fontFamily;
    asset.metadata["fontSize"] = std::to_string(size);
    asset.metadata["leading"] = std::to_string(leading);
    asset.metadata["tracking"] = std::to_string(tracking);
    asset.createdAt = std::chrono::system_clock::now();
    m_assets.push_back(asset);
    return true;
}

bool AssetLibrary::exportLibrary(const std::string& path) {
    return saveToFile(path);
}

bool AssetLibrary::importLibrary(const std::string& path) {
    AssetLibrary other;
    if (!other.loadFromFile(path)) return false;
    for (const auto& asset : other.m_assets) {
        bool exists = std::any_of(m_assets.begin(), m_assets.end(),
            [&asset](const LibraryAsset& a) { return a.id == asset.id; });
        if (!exists) {
            m_assets.push_back(asset);
        }
    }
    return true;
}

bool AssetLibrary::saveToFile(const std::string& path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    file << "FELIBv1\n";
    file << m_name << "\n";
    file << m_assets.size() << "\n";

    for (const auto& asset : m_assets) {
        file << asset.id << "\n";
        file << asset.name << "\n";
        file << static_cast<int>(asset.type) << "\n";
        file << asset.filePath << "\n";
        file << asset.thumbnailWidth << " " << asset.thumbnailHeight << "\n";
        file << asset.thumbnailData.size() << "\n";
        if (!asset.thumbnailData.empty()) {
            file.write(reinterpret_cast<const char*>(asset.thumbnailData.data()),
                       asset.thumbnailData.size());
        }
        file << "\n";
        file << asset.metadata.size() << "\n";
        for (const auto& [k, v] : asset.metadata) {
            file << k << "=" << v << "\n";
        }
        auto ct = std::chrono::system_clock::to_time_t(asset.createdAt);
        file << ct << "\n";
    }
    return file.good();
}

bool AssetLibrary::loadFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    std::string header;
    std::getline(file, header);
    if (header != "FELIBv1") return false;

    std::getline(file, m_name);

    size_t count = 0;
    file >> count;
    file.ignore();

    m_assets.clear();
    for (size_t i = 0; i < count; ++i) {
        LibraryAsset asset;
        std::getline(file, asset.id);
        std::getline(file, asset.name);

        int typeInt = 0;
        file >> typeInt;
        file.ignore();
        asset.type = static_cast<LibraryAssetType>(typeInt);

        std::getline(file, asset.filePath);
        file >> asset.thumbnailWidth >> asset.thumbnailHeight;
        file.ignore();

        size_t thumbSize = 0;
        file >> thumbSize;
        file.ignore();
        if (thumbSize > 0) {
            asset.thumbnailData.resize(thumbSize);
            file.read(reinterpret_cast<char*>(asset.thumbnailData.data()), thumbSize);
        }
        file.ignore();

        size_t metaCount = 0;
        file >> metaCount;
        file.ignore();
        for (size_t j = 0; j < metaCount; ++j) {
            std::string line;
            std::getline(file, line);
            auto eq = line.find('=');
            if (eq != std::string::npos) {
                asset.metadata[line.substr(0, eq)] = line.substr(eq + 1);
            }
        }

        time_t ct = 0;
        file >> ct;
        file.ignore();
        asset.createdAt = std::chrono::system_clock::from_time_t(ct);

        m_assets.push_back(std::move(asset));
    }
    return true;
}

} // namespace FreeEffect
